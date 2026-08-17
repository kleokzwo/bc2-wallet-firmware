#ifndef BC2_ENCODING_H
#define BC2_ENCODING_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
bool bc2_hex_encode(const uint8_t*,size_t,char*,size_t,bool);
bool bc2_hex_decode(const char*,size_t,uint8_t*,size_t,size_t*);
bool bc2_base58_encode(const uint8_t*,size_t,char*,size_t);
bool bc2_base58_decode(const char*,size_t,uint8_t*,size_t,size_t*);
bool bc2_base58check_encode(const uint8_t*,size_t,char*,size_t);
bool bc2_base58check_decode(const char*,size_t,uint8_t*,size_t,size_t*);
bool bc2_bech32_segwit_encode(const char*,uint8_t,const uint8_t*,size_t,char*,size_t);
#endif
