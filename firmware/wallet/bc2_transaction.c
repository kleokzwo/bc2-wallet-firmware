#include "bc2_transaction.h"
#include "bc2_encoding.h"
#include <limits.h>
#include <string.h>

static int add_u64(uint64_t a, uint64_t b, uint64_t *out) {
    if (UINT64_MAX - a < b) return 0;
    *out = a + b;
    return 1;
}

static int bech32_value(char c) {
    static const char *alphabet = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";
    const char *p = strchr(alphabet, c);
    return p == NULL ? -1 : (int)(p - alphabet);
}

static uint32_t polymod_step(uint32_t pre) {
    const uint8_t b = (uint8_t)(pre >> 25);
    uint32_t chk = (pre & 0x1ffffffU) << 5;
    if (b & 1U) chk ^= 0x3b6a57b2U;
    if (b & 2U) chk ^= 0x26508e6dU;
    if (b & 4U) chk ^= 0x1ea119faU;
    if (b & 8U) chk ^= 0x3d4233ddU;
    if (b & 16U) chk ^= 0x2a1462b3U;
    return chk;
}

static int decode_segwit(const char *address, const char *expected_hrp,
                         uint8_t *version, uint8_t *program, size_t *program_length) {
    size_t length = strlen(address);
    size_t separator = 0U;
    uint32_t polymod = 1U;
    int saw_lower = 0, saw_upper = 0;
    if (length < 14U || length > 90U) return 0;
    for (size_t i = 0; i < length; ++i) {
        if (address[i] >= 'a' && address[i] <= 'z') saw_lower = 1;
        if (address[i] >= 'A' && address[i] <= 'Z') saw_upper = 1;
        if (address[i] == '1') separator = i;
    }
    if (saw_lower && saw_upper) return 0;
    if (separator == 0U || separator + 7U > length || separator != strlen(expected_hrp)) return 0;
    for (size_t i = 0; i < separator; ++i) {
        char c = address[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c + ('a' - 'A'));
        if (c != expected_hrp[i]) return 0;
        polymod = polymod_step(polymod) ^ (uint32_t)((unsigned char)c >> 5);
    }
    polymod = polymod_step(polymod);
    for (size_t i = 0; i < separator; ++i) {
        char c = address[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c + ('a' - 'A'));
        polymod = polymod_step(polymod) ^ (uint32_t)((unsigned char)c & 31U);
    }
    uint8_t values[84]; size_t value_count = 0U;
    for (size_t i = separator + 1U; i < length; ++i) {
        char c = address[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c + ('a' - 'A'));
        int value = bech32_value(c);
        if (value < 0) return 0;
        polymod = polymod_step(polymod) ^ (uint32_t)value;
        values[value_count++] = (uint8_t)value;
    }
    if (polymod != 1U || value_count < 7U) return 0;
    *version = values[0];
    if (*version > 16U || *version != 0U) return 0;
    size_t out = 0U; unsigned accumulator = 0U; unsigned bits = 0U;
    for (size_t i = 1U; i + 6U < value_count; ++i) {
        accumulator = (accumulator << 5) | values[i]; bits += 5U;
        while (bits >= 8U) {
            bits -= 8U;
            if (out >= 40U) return 0;
            program[out++] = (uint8_t)((accumulator >> bits) & 0xffU);
        }
    }
    if (bits >= 5U || ((accumulator << (8U - bits)) & 0xffU) != 0U) return 0;
    if (out != 20U && out != 32U) return 0;
    *program_length = out;
    return 1;
}

bc2_tx_status bc2_address_to_script(const char *address, const bc2_network *network,
                                    uint8_t *script, size_t script_capacity,
                                    size_t *script_length) {
    uint8_t payload[64], program[40], version = 0U; size_t payload_length = 0U, program_length = 0U;
    if (address == NULL || network == NULL || script == NULL || script_length == NULL) return BC2_TX_INVALID_ARGUMENT;
    *script_length = 0U;
    if (decode_segwit(address, network->bech32_hrp, &version, program, &program_length)) {
        if (script_capacity < program_length + 2U) return BC2_TX_BUFFER_TOO_SMALL;
        script[0] = version == 0U ? 0x00U : (uint8_t)(0x50U + version);
        script[1] = (uint8_t)program_length;
        memcpy(script + 2U, program, program_length);
        *script_length = program_length + 2U;
        return BC2_TX_OK;
    }
    if (bc2_base58check_decode(address, strlen(address), payload, sizeof(payload), &payload_length) &&
        payload_length == 21U && payload[0] == network->p2pkh_prefix) {
        if (script_capacity < 25U) return BC2_TX_BUFFER_TOO_SMALL;
        script[0]=0x76U; script[1]=0xa9U; script[2]=0x14U;
        memcpy(script+3U,payload+1U,20U); script[23]=0x88U; script[24]=0xacU;
        *script_length=25U; return BC2_TX_OK;
    }
    return BC2_TX_INVALID_ADDRESS;
}

static size_t estimate_vbytes(size_t inputs, int change) {
    return 10U + inputs * 68U + (change ? 2U : 1U) * 31U;
}

bc2_tx_status bc2_transaction_plan(const bc2_tx_utxo *utxos, size_t utxo_count,
                                   uint64_t recipient_amount, uint64_t fee_rate_sat_vbyte,
                                   uint64_t dust_limit, bc2_tx_plan *plan) {
    uint64_t selected = 0U;
    if (utxos == NULL || plan == NULL || utxo_count == 0U || recipient_amount == 0U || fee_rate_sat_vbyte == 0U)
        return BC2_TX_INVALID_ARGUMENT;
    if (utxo_count > BC2_TX_MAX_INPUTS) return BC2_TX_LIMIT_EXCEEDED;
    memset(plan, 0, sizeof(*plan)); plan->recipient_amount = recipient_amount; plan->fee_rate_sat_vbyte = fee_rate_sat_vbyte;
    for (size_t i = 0U; i < utxo_count; ++i) {
        uint64_t candidate; if (!add_u64(selected, utxos[i].amount, &candidate)) return BC2_TX_VALUE_OVERFLOW;
        selected = candidate; plan->selected_indices[plan->selected_input_count++] = (unsigned int)i;
        size_t with_change = estimate_vbytes(plan->selected_input_count, 1);
        if (fee_rate_sat_vbyte > UINT64_MAX / with_change) return BC2_TX_VALUE_OVERFLOW;
        uint64_t fee = fee_rate_sat_vbyte * with_change, needed;
        if (!add_u64(recipient_amount, fee, &needed)) return BC2_TX_VALUE_OVERFLOW;
        if (selected >= needed) {
            uint64_t change = selected - needed;
            if (change >= dust_limit) {
                plan->estimated_vbytes = with_change; plan->fee_amount = fee; plan->change_amount = change; plan->change_output_created = 1;
            } else {
                size_t no_change = estimate_vbytes(plan->selected_input_count, 0);
                uint64_t minimum_fee = fee_rate_sat_vbyte * no_change;
                if (selected < recipient_amount + minimum_fee) continue;
                plan->estimated_vbytes = no_change; plan->fee_amount = selected - recipient_amount; plan->change_amount = 0U; plan->change_output_created = 0;
            }
            plan->selected_amount = selected; return BC2_TX_OK;
        }
    }
    return BC2_TX_INSUFFICIENT_FUNDS;
}

static int put_byte(uint8_t *out,size_t cap,size_t *off,uint8_t v){if(*off>=cap)return 0;out[(*off)++]=v;return 1;}
static int put_data(uint8_t*out,size_t cap,size_t*off,const void*d,size_t n){if(n>cap-*off)return 0;memcpy(out+*off,d,n);*off+=n;return 1;}
static int put_u32(uint8_t*out,size_t cap,size_t*off,uint32_t v){uint8_t b[4];for(int i=0;i<4;i++)b[i]=(uint8_t)(v>>(8*i));return put_data(out,cap,off,b,4);}
static int put_u64(uint8_t*out,size_t cap,size_t*off,uint64_t v){uint8_t b[8];for(int i=0;i<8;i++)b[i]=(uint8_t)(v>>(8*i));return put_data(out,cap,off,b,8);}
static int put_compact(uint8_t*out,size_t cap,size_t*off,uint64_t v){if(v<0xfdU)return put_byte(out,cap,off,(uint8_t)v);return 0;}

bc2_tx_status bc2_psbt_create(const bc2_tx_utxo *utxos, size_t utxo_count, const bc2_tx_plan *plan,
                              const uint8_t *recipient_script,size_t recipient_script_length,
                              const uint8_t *change_script,size_t change_script_length,
                              uint8_t *output,size_t output_capacity,size_t *output_length) {
    uint8_t tx[8192]; size_t t=0U,o=0U;
    if(!utxos||!plan||!recipient_script||!output||!output_length||plan->selected_input_count==0U||recipient_script_length==0U)return BC2_TX_INVALID_ARGUMENT;
    if(plan->change_output_created && (!change_script||change_script_length==0U)) return BC2_TX_INVALID_ARGUMENT;
    if(utxo_count>BC2_TX_MAX_INPUTS||recipient_script_length>BC2_TX_MAX_SCRIPT_SIZE||change_script_length>BC2_TX_MAX_SCRIPT_SIZE)return BC2_TX_LIMIT_EXCEEDED;
    if(!put_u32(tx,sizeof tx,&t,2U)||!put_compact(tx,sizeof tx,&t,plan->selected_input_count))return BC2_TX_BUFFER_TOO_SMALL;
    for(unsigned int i=0;i<plan->selected_input_count;i++){unsigned int idx=plan->selected_indices[i];if(idx>=utxo_count)return BC2_TX_INVALID_ARGUMENT;
        if(!put_data(tx,sizeof tx,&t,utxos[idx].txid,32U)||!put_u32(tx,sizeof tx,&t,utxos[idx].output_index)||!put_byte(tx,sizeof tx,&t,0U)||!put_u32(tx,sizeof tx,&t,0xfffffffdU))return BC2_TX_BUFFER_TOO_SMALL;}
    if(!put_compact(tx,sizeof tx,&t,plan->change_output_created?2U:1U)||!put_u64(tx,sizeof tx,&t,plan->recipient_amount)||!put_compact(tx,sizeof tx,&t,recipient_script_length)||!put_data(tx,sizeof tx,&t,recipient_script,recipient_script_length))return BC2_TX_BUFFER_TOO_SMALL;
    if(plan->change_output_created && (!put_u64(tx,sizeof tx,&t,plan->change_amount)||!put_compact(tx,sizeof tx,&t,change_script_length)||!put_data(tx,sizeof tx,&t,change_script,change_script_length)))return BC2_TX_BUFFER_TOO_SMALL;
    if(!put_u32(tx,sizeof tx,&t,0U))return BC2_TX_BUFFER_TOO_SMALL;
    if(!put_data(output,output_capacity,&o,"psbt\xff",5U)||!put_byte(output,output_capacity,&o,1U)||!put_byte(output,output_capacity,&o,0U)||!put_compact(output,output_capacity,&o,t)||!put_data(output,output_capacity,&o,tx,t)||!put_byte(output,output_capacity,&o,0U))return BC2_TX_BUFFER_TOO_SMALL;
    for(unsigned int i=0;i<plan->selected_input_count;i++){unsigned int idx=plan->selected_indices[i];size_t value_length=8U+1U+utxos[idx].script_length;
        if(!put_byte(output,output_capacity,&o,1U)||!put_byte(output,output_capacity,&o,1U)||!put_compact(output,output_capacity,&o,value_length)||!put_u64(output,output_capacity,&o,utxos[idx].amount)||!put_compact(output,output_capacity,&o,utxos[idx].script_length)||!put_data(output,output_capacity,&o,utxos[idx].script,utxos[idx].script_length)||!put_byte(output,output_capacity,&o,0U))return BC2_TX_BUFFER_TOO_SMALL;}
    if(!put_byte(output,output_capacity,&o,0U))return BC2_TX_BUFFER_TOO_SMALL;
    if(plan->change_output_created && !put_byte(output,output_capacity,&o,0U))return BC2_TX_BUFFER_TOO_SMALL;
    *output_length=o;return BC2_TX_OK;
}

const char *bc2_tx_status_message(bc2_tx_status status){switch(status){case BC2_TX_OK:return "Transaction draft created";case BC2_TX_INVALID_ARGUMENT:return "Invalid transaction argument";case BC2_TX_INVALID_ADDRESS:return "Invalid BC2 address";case BC2_TX_INSUFFICIENT_FUNDS:return "Insufficient funds";case BC2_TX_LIMIT_EXCEEDED:return "Transaction safety limit exceeded";case BC2_TX_BUFFER_TOO_SMALL:return "Output buffer too small";case BC2_TX_VALUE_OVERFLOW:return "Transaction value overflow";default:return "Unknown transaction status";}}
