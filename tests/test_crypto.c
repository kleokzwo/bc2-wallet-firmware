#include "bc2_crypto.h"
#include "bc2_encoding.h"
#include <assert.h>
#include <string.h>
int main(void){uint8_t h[32];char x[65];assert(bc2_sha256((const uint8_t*)"abc",3,h));assert(bc2_hex_encode(h,32,x,sizeof x,false));assert(strcmp(x,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")==0);return 0;}
