#include "bc2_bip32.h"
#include "bc2_crypto.h"
#include "bc2_encoding.h"
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <string.h>
#include <stdlib.h>
static void be32(uint8_t*p,uint32_t x){p[0]=(uint8_t)(x>>24);p[1]=(uint8_t)(x>>16);p[2]=(uint8_t)(x>>8);p[3]=(uint8_t)x;}
bool bc2_secp256k1_public(const uint8_t k[32],uint8_t o[33]){if(!k||!o)return false;EC_GROUP*g=EC_GROUP_new_by_curve_name(NID_secp256k1);BN_CTX*c=BN_CTX_new();BIGNUM*b=BN_bin2bn(k,32,NULL);EC_POINT*q=g?EC_POINT_new(g):NULL;bool ok=g&&c&&b&&q&&!BN_is_zero(b)&&EC_POINT_mul(g,q,b,NULL,NULL,c)==1&&EC_POINT_point2oct(g,q,POINT_CONVERSION_COMPRESSED,o,33,c)==33U;EC_POINT_free(q);BN_clear_free(b);BN_CTX_free(c);EC_GROUP_free(g);return ok;}
static uint32_t fp(const bc2_xprv*x){uint8_t p[33],h[20];if(!bc2_secp256k1_public(x->key,p)||!bc2_hash160(p,33,h))return 0;return ((uint32_t)h[0]<<24)|((uint32_t)h[1]<<16)|((uint32_t)h[2]<<8)|h[3];}
bool bc2_bip32_master(const uint8_t*s,size_t n,bc2_xprv*o){static const uint8_t k[]="Bitcoin seed";uint8_t h[64];if(!s||!o||!bc2_hmac_sha512(k,sizeof k-1U,s,n,h))return false;memset(o,0,sizeof *o);memcpy(o->key,h,32);memcpy(o->chain,h+32,32);return true;}
bool bc2_bip32_derive(const bc2_xprv*p,uint32_t idx,bc2_xprv*o){if(!p||!o||p->depth==255U)return false;uint8_t d[37],h[64],pub[33];if(idx&0x80000000U){d[0]=0;memcpy(d+1,p->key,32);}else{if(!bc2_secp256k1_public(p->key,pub))return false;memcpy(d,pub,33);}be32(d+33,idx);if(!bc2_hmac_sha512(p->chain,32,d,37,h))return false;EC_GROUP*g=EC_GROUP_new_by_curve_name(NID_secp256k1);BN_CTX*c=BN_CTX_new();BIGNUM*a=BN_bin2bn(h,32,NULL),*b=BN_bin2bn(p->key,32,NULL),*ord=BN_new();bool ok=g&&c&&a&&b&&ord&&EC_GROUP_get_order(g,ord,c)==1&&BN_cmp(a,ord)<0&&BN_mod_add(b,b,a,ord,c)==1&&!BN_is_zero(b)&&BN_bn2binpad(b,o->key,32)==32;if(ok){memcpy(o->chain,h+32,32);o->depth=(uint8_t)(p->depth+1U);o->child=idx;o->parent_fingerprint=fp(p);}BN_clear_free(a);BN_clear_free(b);BN_free(ord);BN_CTX_free(c);EC_GROUP_free(g);return ok;}
bool bc2_bip32_derive_path(const bc2_xprv*m,const char*path,bc2_xprv*o){if(!m||!path||!o||path[0]!='m')return false;bc2_xprv cur=*m;if(path[1]==0){*o=cur;return true;}if(path[1]!='/')return false;char b[256];if(strlen(path)>=sizeof b)return false;strcpy(b,path+2);char*save=NULL;for(char*t=strtok_r(b,"/",&save);t;t=strtok_r(NULL,"/",&save)){size_t n=strlen(t);bool hard=n&&t[n-1]=='\'';if(hard)t[--n]=0;if(n==0)return false;char*e=NULL;unsigned long v=strtoul(t,&e,10);if(!e||*e||v>0x7fffffffUL)return false;uint32_t idx=(uint32_t)v|(hard?0x80000000U:0U);bc2_xprv next;if(!bc2_bip32_derive(&cur,idx,&next))return false;cur=next;}*o=cur;return true;}
static bool ser(const bc2_xprv*x,uint32_t ver,bool pub,char*out,size_t z){uint8_t b[78];be32(b,ver);b[4]=x->depth;be32(b+5,x->parent_fingerprint);be32(b+9,x->child);memcpy(b+13,x->chain,32);if(pub){if(!bc2_secp256k1_public(x->key,b+45))return false;}else{b[45]=0;memcpy(b+46,x->key,32);}return bc2_base58check_encode(b,78,out,z);}
bool bc2_xprv_serialize(const bc2_xprv*x,uint32_t v,char*o,size_t z){return x&&ser(x,v,false,o,z);}bool bc2_xpub_serialize(const bc2_xprv*x,uint32_t v,char*o,size_t z){return x&&ser(x,v,true,o,z);}
