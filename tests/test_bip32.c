#include "bc2_bip32.h"
#include "bc2_encoding.h"
#include <assert.h>
#include <string.h>
int main(void){uint8_t seed[16];size_t n;bc2_xprv m;char h[65];assert(bc2_hex_decode("000102030405060708090a0b0c0d0e0f",32,seed,sizeof seed,&n));assert(bc2_bip32_master(seed,n,&m));assert(bc2_hex_encode(m.key,32,h,sizeof h,false));assert(strcmp(h,"e8f32e723decf4051aefac8e2c93c9c5b214313817cdb01a1494b917c8436b35")==0);assert(bc2_hex_encode(m.chain,32,h,sizeof h,false));assert(strcmp(h,"873dff81c02f525623fd1fe5167eac3a55a049de3d314bb42ee227ffed37d508")==0);return 0;}
