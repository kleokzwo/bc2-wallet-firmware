#include "bc2_sign.h"
#include "bc2_bip32.h"
#include "bc2_crypto.h"
#include <assert.h>
#include <string.h>
int main(void){uint8_t k[32]={0},h[32],p[33],s[80];size_t n;k[31]=1;assert(bc2_sha256((const uint8_t*)"BC2",3,h));assert(bc2_secp256k1_public(k,p));assert(bc2_ecdsa_sign_der(k,h,s,sizeof s,&n));assert(bc2_ecdsa_verify_der(p,h,s,n));
uint8_t prev[32],ph[20],rs[22]={0,20},cs[22]={0,20},got[32];const uint8_t exp[32]={0x0b,0xfb,0x33,0x36,0xe2,0x6f,0x2a,0xba,0xb9,0x5c,0x2a,0x96,0x56,0x68,0x1a,0x09,0x0a,0x5a,0x9a,0xd0,0x34,0x21,0x3a,0xab,0x72,0x5d,0xeb,0x80,0x4e,0x80,0x79,0x9c};for(unsigned i=0;i<32;i++)prev[i]=(uint8_t)i;for(unsigned i=0;i<20;i++)ph[i]=(uint8_t)i;memset(rs+2,0x44,20);memset(cs+2,0x55,20);assert(bc2_p2wpkh_sighash_all_single(prev,3,100000,ph,0xfffffffdU,rs,22,1000,cs,22,98720,0,got));assert(memcmp(got,exp,32)==0);return 0;}
