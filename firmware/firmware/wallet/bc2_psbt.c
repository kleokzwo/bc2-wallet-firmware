#include "bc2_psbt.h"
#include "bc2_encoding.h"
#include <limits.h>
#include <string.h>
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif

#define BC2_PSBT_MAX_SIZE (1024U * 1024U)
#define BC2_PSBT_MAX_MAP_PAIRS 512U

typedef struct { const uint8_t *data; size_t size; size_t off; } reader;

static int add_u64(uint64_t a, uint64_t b, uint64_t *out) {
    if (UINT64_MAX - a < b) return 0;
    *out = a + b; return 1;
}
static int read_u32(reader *r, uint32_t *v) {
    if (r->size-r->off<4U) return 0;
    *v=(uint32_t)r->data[r->off]|((uint32_t)r->data[r->off+1U]<<8U)|
       ((uint32_t)r->data[r->off+2U]<<16U)|((uint32_t)r->data[r->off+3U]<<24U);
    r->off+=4U; return 1;
}
static int read_u64(reader *r, uint64_t *v) {
    if (r->size-r->off<8U) return 0; *v=0U;
    for (unsigned int i=0U;i<8U;++i) *v|=((uint64_t)r->data[r->off+i])<<(8U*i);
    r->off+=8U; return 1;
}
static int read_compact(reader *r, uint64_t *v) {
    uint8_t f; if(r->off>=r->size)return 0; f=r->data[r->off++];
    if(f<0xfdU){*v=f;return 1;}
    if(f==0xfdU){if(r->size-r->off<2U)return 0;*v=(uint64_t)r->data[r->off]|((uint64_t)r->data[r->off+1U]<<8U);r->off+=2U;return *v>=0xfdU;}
    if(f==0xfeU){uint32_t x;if(!read_u32(r,&x)||x<0x10000U)return 0;*v=x;return 1;}
    if(!read_u64(r,v)||*v<0x100000000ULL)return 0;return 1;
}
static int skip(reader *r,uint64_t n){if(n>(uint64_t)(r->size-r->off))return 0;r->off+=(size_t)n;return 1;}
static int script_address(const uint8_t *s,size_t n,const char *hrp,char *out,size_t outn){
    if(hrp!=NULL&&n==22U&&s[0]==0x00U&&s[1]==0x14U)return bc2_bech32_segwit_encode(hrp,0,s+2U,20U,out,outn);
    if(outn>0U)out[0]='\0';return 0;
}
static void classify_output(bc2_psbt_output *o,const bc2_owned_script *owned,size_t count){
    for(size_t i=0U;i<count;++i){if(owned[i].length==o->script_length&&memcmp(owned[i].bytes,o->script,o->script_length)==0){o->owned=1;o->change=owned[i].is_change;return;}}
}
static bc2_psbt_status parse_unsigned_tx(const uint8_t *d,size_t n,const bc2_owned_script *owned,size_t owned_count,const char *hrp,bc2_psbt_summary *s){
    reader r={d,n,0U};uint64_t count=0U,sl=0U;
    if(!read_u32(&r,&s->transaction_version)||!read_compact(&r,&count))return BC2_PSBT_TRUNCATED;
    if(count==0U||count>BC2_PSBT_MAX_INPUTS)return BC2_PSBT_LIMIT_EXCEEDED;s->input_count=(unsigned int)count;
    for(unsigned int i=0U;i<s->input_count;++i){
        if(r.size-r.off<32U)return BC2_PSBT_TRUNCATED;memcpy(s->inputs[i].previous_txid,r.data+r.off,32U);r.off+=32U;
        if(!read_u32(&r,&s->inputs[i].previous_output_index)||!read_compact(&r,&sl)||!skip(&r,sl)||!read_u32(&r,&s->inputs[i].sequence))return BC2_PSBT_TRUNCATED;
        if(sl!=0U)return BC2_PSBT_MALFORMED;
    }
    if(!read_compact(&r,&count))return BC2_PSBT_TRUNCATED;if(count>BC2_PSBT_MAX_OUTPUTS)return BC2_PSBT_LIMIT_EXCEEDED;s->output_count=(unsigned int)count;
    for(unsigned int i=0U;i<s->output_count;++i){bc2_psbt_output *o=&s->outputs[i];
        if(!read_u64(&r,&o->amount)||!read_compact(&r,&sl))return BC2_PSBT_TRUNCATED;if(sl>BC2_PSBT_MAX_SCRIPT_SIZE)return BC2_PSBT_LIMIT_EXCEEDED;
        if(!skip(&r,sl))return BC2_PSBT_TRUNCATED;o->script_length=(size_t)sl;memcpy(o->script,r.data+r.off-(size_t)sl,(size_t)sl);
        classify_output(o,owned,owned_count);(void)script_address(o->script,o->script_length,hrp,o->address,sizeof(o->address));
        if(!add_u64(s->total_output_amount,o->amount,&s->total_output_amount))return BC2_PSBT_VALUE_OVERFLOW;
        if(o->owned&&o->change){if(!add_u64(s->change_amount,o->amount,&s->change_amount))return BC2_PSBT_VALUE_OVERFLOW;}
        else if(!o->owned){if(!add_u64(s->external_output_amount,o->amount,&s->external_output_amount))return BC2_PSBT_VALUE_OVERFLOW;}
    }
    if(!read_u32(&r,&s->lock_time))return BC2_PSBT_TRUNCATED;if(r.off!=r.size)return BC2_PSBT_MALFORMED;return BC2_PSBT_OK;
}
static bc2_psbt_status parse_witness_utxo(const uint8_t *d,size_t n,uint64_t *amount){reader r={d,n,0U};uint64_t sl;if(!read_u64(&r,amount)||!read_compact(&r,&sl)||!skip(&r,sl)||r.off!=r.size)return BC2_PSBT_MALFORMED;return BC2_PSBT_OK;}
static bc2_psbt_status parse_map(reader *r,unsigned int *pairs,const uint8_t **unsigned_tx,size_t *unsigned_tx_len,int input_index,bc2_psbt_summary *s){
    unsigned int p=0U;for(;;){uint64_t kl=0U,vl=0U;if(!read_compact(r,&kl))return BC2_PSBT_TRUNCATED;if(kl==0U){*pairs=p;return BC2_PSBT_OK;}if(kl>(uint64_t)(r->size-r->off))return BC2_PSBT_TRUNCATED;
        const uint8_t *key=r->data+r->off;r->off+=(size_t)kl;if(!read_compact(r,&vl)||vl>(uint64_t)(r->size-r->off))return BC2_PSBT_TRUNCATED;const uint8_t *val=r->data+r->off;r->off+=(size_t)vl;
        if(input_index<0&&kl==1U&&key[0]==0x00U){if(*unsigned_tx!=NULL)return BC2_PSBT_MALFORMED;*unsigned_tx=val;*unsigned_tx_len=(size_t)vl;s->contains_unsigned_transaction=1;}
        if(input_index>=0&&kl==1U&&key[0]==0x01U){uint64_t amount=0U;bc2_psbt_status st=parse_witness_utxo(val,(size_t)vl,&amount);if(st!=BC2_PSBT_OK)return st;s->inputs[input_index].amount=amount;s->inputs[input_index].amount_known=1;}
        if(++p>BC2_PSBT_MAX_MAP_PAIRS)return BC2_PSBT_LIMIT_EXCEEDED;
    }}

bc2_psbt_status bc2_psbt_review(const uint8_t *data,size_t size,const bc2_owned_script *owned,size_t owned_count,const char *hrp,bc2_psbt_summary *s){
    static const uint8_t magic[5]={'p','s','b','t',0xff};reader r;const uint8_t *tx=NULL;size_t txlen=0U;unsigned int pairs=0U;bc2_psbt_status st;
    if(data==NULL||s==NULL||(owned_count>0U&&owned==NULL)||owned_count>BC2_PSBT_MAX_OWNED_SCRIPTS)return BC2_PSBT_INVALID_ARGUMENT;memset(s,0,sizeof(*s));s->total_size=size;
    if(size>BC2_PSBT_MAX_SIZE)return BC2_PSBT_LIMIT_EXCEEDED;if(size<5U)return BC2_PSBT_TRUNCATED;if(memcmp(data,magic,5U)!=0)return BC2_PSBT_INVALID_MAGIC;r=(reader){data,size,5U};
    st=parse_map(&r,&pairs,&tx,&txlen,-1,s);if(st!=BC2_PSBT_OK)return st;s->global_key_value_pairs=pairs;if(tx==NULL)return BC2_PSBT_UNSUPPORTED;
    st=parse_unsigned_tx(tx,txlen,owned,owned_count,hrp,s);if(st!=BC2_PSBT_OK)return st;
    for(unsigned int i=0U;i<s->input_count;++i){st=parse_map(&r,&pairs,&tx,&txlen,(int)i,s);if(st!=BC2_PSBT_OK)return st;}
    for(unsigned int i=0U;i<s->output_count;++i){st=parse_map(&r,&pairs,&tx,&txlen,-2,s);if(st!=BC2_PSBT_OK)return st;}
    if(r.off!=r.size)return BC2_PSBT_MALFORMED;s->all_input_amounts_known=1;
    for(unsigned int i=0U;i<s->input_count;++i){if(!s->inputs[i].amount_known){s->all_input_amounts_known=0;continue;}if(!add_u64(s->total_input_amount,s->inputs[i].amount,&s->total_input_amount))return BC2_PSBT_VALUE_OVERFLOW;}
    if(!s->all_input_amounts_known){s->structurally_valid=1;return BC2_PSBT_MISSING_UTXO;}if(s->total_input_amount<s->total_output_amount)return BC2_PSBT_MALFORMED;
    s->fee_amount=s->total_input_amount-s->total_output_amount;s->fee_known=1;s->change_verified=owned_count>0U?1:0;s->structurally_valid=1;return BC2_PSBT_OK;
}
bc2_psbt_status bc2_psbt_inspect(const uint8_t *d,size_t n,bc2_psbt_summary *s){return bc2_psbt_review(d,n,NULL,0U,NULL,s);}
const char *bc2_psbt_status_message(bc2_psbt_status st){switch(st){case BC2_PSBT_OK:return "PSBT transaction accepted for review";case BC2_PSBT_INVALID_ARGUMENT:return "Invalid argument";case BC2_PSBT_INVALID_MAGIC:return "Invalid PSBT magic";case BC2_PSBT_TRUNCATED:return "Truncated PSBT";case BC2_PSBT_MALFORMED:return "Malformed PSBT or transaction";case BC2_PSBT_UNSUPPORTED:return "Unsupported PSBT structure";case BC2_PSBT_LIMIT_EXCEEDED:return "PSBT safety limit exceeded";case BC2_PSBT_MISSING_UTXO:return "Input amount missing: witness_utxo required";case BC2_PSBT_VALUE_OVERFLOW:return "Transaction value overflow";default:return "Unknown PSBT status";}}
