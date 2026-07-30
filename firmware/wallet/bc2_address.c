#include "bc2_address.h"
#include "bc2_crypto.h"
#include "bc2_encoding.h"
#include <string.h>
bool bc2_address_p2pkh(const uint8_t p[33],uint8_t v,char*o,size_t z){uint8_t b[21];b[0]=v;return p&&bc2_hash160(p,33,b+1)&&bc2_base58check_encode(b,21,o,z);}bool bc2_address_p2wpkh(const uint8_t p[33],const char*h,char*o,size_t z){uint8_t x[20];return p&&h&&bc2_hash160(p,33,x)&&bc2_bech32_segwit_encode(h,0,x,20,o,z);}
