#include "bc2_bip39.h"
#include "bc2_bip32.h"
#include "bc2_address.h"
#include "bc2_secure.h"
#include <stdio.h>
#include <string.h>
static void usage(const char*x){fprintf(stderr,"Usage:\n  %s generate\n  %s derive \"mnemonic\" [passphrase] [path]\n",x,x);}int main(int ac,char**av){if(ac==2&&strcmp(av[1],"generate")==0){char m[256];if(!bc2_bip39_generate_12(m,sizeof m))return 1;puts(m);return 0;}if(ac>=3&&strcmp(av[1],"derive")==0){const char*pass=ac>3?av[3]:"";const char*path=ac>4?av[4]:"m/84'/0'/0'/0/0";uint8_t seed[64],pub[33];bc2_xprv master,node;char p2pkh[64],p2w[96],xprv[128],xpub[128];if(!bc2_bip39_seed(av[2],pass,seed)||!bc2_bip32_master(seed,64,&master)||!bc2_bip32_derive_path(&master,path,&node)||!bc2_secp256k1_public(node.key,pub)||!bc2_address_p2pkh(pub,0,p2pkh,sizeof p2pkh)||!bc2_address_p2wpkh(pub,"bc",p2w,sizeof p2w)||!bc2_xprv_serialize(&node,0x0488ADE4U,xprv,sizeof xprv)||!bc2_xpub_serialize(&node,0x0488B21EU,xpub,sizeof xpub)){bc2_secure_zero(seed,sizeof seed);return 1;}printf("path:  %s\np2pkh: %s\np2wpkh:%s\nxprv:  %s\nxpub:  %s\n",path,p2pkh,p2w,xprv,xpub);bc2_secure_zero(seed,sizeof seed);bc2_secure_zero(&master,sizeof master);bc2_secure_zero(&node,sizeof node);return 0;}usage(av[0]);return 2;}
