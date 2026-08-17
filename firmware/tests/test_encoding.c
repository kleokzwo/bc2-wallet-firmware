#include "bc2_encoding.h"
#include <assert.h>
#include <string.h>
int main(void){const uint8_t d[]={0,0,1};char b[32];uint8_t o[8];size_t n=0;assert(bc2_base58_encode(d,3,b,sizeof b));assert(strcmp(b,"112")==0);assert(bc2_base58_decode(b,strlen(b),o,sizeof o,&n));assert(n==3&&memcmp(d,o,3)==0);return 0;}
