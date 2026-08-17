#ifndef BC2_ADDRESS_H
#define BC2_ADDRESS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
bool bc2_address_p2pkh(const uint8_t[33],uint8_t,char*,size_t);
bool bc2_address_p2wpkh(const uint8_t[33],const char*,char*,size_t);
#endif
