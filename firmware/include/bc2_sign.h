#ifndef BC2_SIGN_H
#define BC2_SIGN_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
bool bc2_ecdsa_sign_der(const uint8_t[32],const uint8_t[32],uint8_t*,size_t,size_t*);
bool bc2_ecdsa_verify_der(const uint8_t[33],const uint8_t[32],const uint8_t*,size_t);
#endif
