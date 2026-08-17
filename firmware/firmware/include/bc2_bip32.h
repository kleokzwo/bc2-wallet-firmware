#ifndef BC2_BIP32_H
#define BC2_BIP32_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct{uint8_t key[32],chain[32];uint8_t depth;uint32_t child;uint32_t parent_fingerprint;}bc2_xprv;
bool bc2_bip32_master(const uint8_t*,size_t,bc2_xprv*);
bool bc2_bip32_derive(const bc2_xprv*,uint32_t,bc2_xprv*);
bool bc2_bip32_derive_path(const bc2_xprv*,const char*,bc2_xprv*);
bool bc2_secp256k1_public(const uint8_t[32],uint8_t[33]);
bool bc2_xprv_serialize(const bc2_xprv*,uint32_t,char*,size_t);
bool bc2_xpub_serialize(const bc2_xprv*,uint32_t,char*,size_t);
#endif
