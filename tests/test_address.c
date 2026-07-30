#include "bc2_bip32.h"
#include "bc2_address.h"
#include <assert.h>
#include <string.h>
int main(void){uint8_t k[32]={0},p[33];char a[96];k[31]=1;assert(bc2_secp256k1_public(k,p));assert(bc2_address_p2pkh(p,0,a,sizeof a));assert(strcmp(a,"1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH")==0);assert(bc2_address_p2wpkh(p,"bc",a,sizeof a));assert(strcmp(a,"bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4")==0);return 0;}
