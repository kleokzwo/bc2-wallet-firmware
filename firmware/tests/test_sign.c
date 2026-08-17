#include "bc2_sign.h"
#include "bc2_bip32.h"
#include "bc2_crypto.h"
#include <assert.h>
int main(void){uint8_t k[32]={0},h[32],p[33],s[80];size_t n;k[31]=1;assert(bc2_sha256((const uint8_t*)"BC2",3,h));assert(bc2_secp256k1_public(k,p));assert(bc2_ecdsa_sign_der(k,h,s,sizeof s,&n));assert(bc2_ecdsa_verify_der(p,h,s,n));return 0;}
