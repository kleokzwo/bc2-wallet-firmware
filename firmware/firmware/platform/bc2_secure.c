#include "bc2_secure.h"
#include <openssl/crypto.h>
void bc2_secure_zero(void*p,size_t n){if(p&&n)OPENSSL_cleanse(p,n);}
