#ifndef BC2_BIP39_H
#define BC2_BIP39_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
bool bc2_bip39_validate(const char*);
bool bc2_bip39_seed(const char*,const char*,uint8_t[64]);
bool bc2_bip39_generate_12(char*,size_t);
#endif
