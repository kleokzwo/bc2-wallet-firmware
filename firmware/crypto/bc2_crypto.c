#include "bc2_crypto.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <limits.h>
static bool digest(const EVP_MD *md,const uint8_t *d,size_t n,uint8_t *out,unsigned int want){
 if((n>0U&&d==NULL)||out==NULL||n>(size_t)INT_MAX)return false;
 EVP_MD_CTX *c=EVP_MD_CTX_new(); if(!c)return false; unsigned int got=0; bool ok=EVP_DigestInit_ex(c,md,NULL)==1&&EVP_DigestUpdate(c,d,n)==1&&EVP_DigestFinal_ex(c,out,&got)==1&&got==want; EVP_MD_CTX_free(c); return ok;
}
bool bc2_sha256(const uint8_t*d,size_t n,uint8_t o[32]){return digest(EVP_sha256(),d,n,o,32U);} 
bool bc2_sha256d(const uint8_t*d,size_t n,uint8_t o[32]){uint8_t t[32]; if(!bc2_sha256(d,n,t))return false; return bc2_sha256(t,32,o);} 
bool bc2_ripemd160(const uint8_t*d,size_t n,uint8_t o[20]){return digest(EVP_ripemd160(),d,n,o,20U);} 
bool bc2_hash160(const uint8_t*d,size_t n,uint8_t o[20]){uint8_t t[32]; return bc2_sha256(d,n,t)&&bc2_ripemd160(t,32,o);} 
bool bc2_hmac_sha512(const uint8_t*k,size_t kn,const uint8_t*d,size_t n,uint8_t o[64]){if((kn&& !k)||(n&&!d)||!o||kn>(size_t)INT_MAX)return false; unsigned int got=0; return HMAC(EVP_sha512(),k,(int)kn,d,n,o,&got)!=NULL&&got==64U;}
bool bc2_pbkdf2_hmac_sha512(const uint8_t*p,size_t pn,const uint8_t*s,size_t sn,uint32_t it,uint8_t*o,size_t on){if((pn&&!p)||(sn&&!s)||!o||pn>(size_t)INT_MAX||sn>(size_t)INT_MAX||on>(size_t)INT_MAX||it==0U||it>(uint32_t)INT_MAX)return false;return PKCS5_PBKDF2_HMAC((const char*)p,(int)pn,s,(int)sn,(int)it,EVP_sha512(),(int)on,o)==1;}
bool bc2_random(uint8_t*o,size_t n){if(!o||n>(size_t)INT_MAX)return false;return RAND_bytes(o,(int)n)==1;}
