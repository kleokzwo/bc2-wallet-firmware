#include "bc2_bip39.h"
#include "bc2_encoding.h"
#include <assert.h>
#include <string.h>
int main(void){const char*m="abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";uint8_t s[64];char h[129];assert(bc2_bip39_validate(m));assert(bc2_bip39_seed(m,"TREZOR",s));assert(bc2_hex_encode(s,64,h,sizeof h,false));assert(strcmp(h,"c55257c360c07c72029aebc1b53c05ed0362ada38ead3e3e9efa3708e53495531f09a6987599d18264c1e1c92f2cf141630c7a3c4ab7c81b2f001698e7463b04")==0);return 0;}
