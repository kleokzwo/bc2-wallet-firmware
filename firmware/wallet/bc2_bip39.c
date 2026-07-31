#include "bc2_bip39.h"
#include "bc2_crypto.h"
#include "bc2_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef BC2_BIP39_WORDLIST_PATH
#define BC2_BIP39_WORDLIST_PATH "third_party/bip39/english.txt"
#endif
static bool load(char words[2048][9]){FILE*f=fopen(BC2_BIP39_WORDLIST_PATH,"r");if(!f)return false;for(size_t i=0;i<2048;i++){if(!fgets(words[i],9,f)){fclose(f);return false;}words[i][strcspn(words[i],"\r\n")]=0;}fclose(f);return true;}
static int findw(char words[2048][9],const char*w){int lo=0,hi=2047;while(lo<=hi){int m=(lo+hi)/2,c=strcmp(w,words[m]);if(c==0)return m;if(c<0)hi=m-1;else lo=m+1;}return -1;}
bool bc2_bip39_validate(const char*m){if(!m)return false;char words[2048][9];if(!load(words))return false;char buf[256];if(strlen(m)>=sizeof buf)return false;if (!bc2_copy_string(buf, sizeof buf, m)) return false;uint16_t idx[24];size_t count=0;char*save=NULL;for(char*t=bc2_strtok(buf," ",&save);t;t=bc2_strtok(NULL," ",&save)){if(count>=24U)return false;int x=findw(words,t);if(x<0)return false;idx[count++]=(uint16_t)x;}if(!(count==12U||count==15U||count==18U||count==21U||count==24U))return false;size_t total=count*11U,cs=total/33U,ent=total-cs;uint8_t entropy[32]={0};for(size_t b=0;b<ent;b++){size_t wi=b/11U,off=b%11U;uint8_t bit=(uint8_t)((idx[wi]>>(10U-off))&1U);entropy[b/8U]|=(uint8_t)(bit<<(7U-(b%8U)));}uint8_t h[32];if(!bc2_sha256(entropy,ent/8U,h))return false;for(size_t b=0;b<cs;b++){size_t pos=ent+b,wi=pos/11U,off=pos%11U;uint8_t got=(uint8_t)((idx[wi]>>(10U-off))&1U),exp=(uint8_t)((h[0]>>(7U-b))&1U);if(got!=exp)return false;}return true;}
bool bc2_bip39_seed(const char*m,const char*p,uint8_t out[64]){if(!m||!p||!out||!bc2_bip39_validate(m))return false;char salt[256];int n=snprintf(salt,sizeof salt,"mnemonic%s",p);if(n<0||(size_t)n>=sizeof salt)return false;return bc2_pbkdf2_hmac_sha512((const uint8_t*)m,strlen(m),(const uint8_t*)salt,(size_t)n,2048U,out,64U);}
bool bc2_bip39_generate_12(char*out,size_t cap){if(!out)return false;char words[2048][9];if(!load(words))return false;uint8_t e[16],h[32];if(!bc2_random(e,16)||!bc2_sha256(e,16,h))return false;uint16_t idx[12]={0};for(size_t b=0;b<132;b++){uint8_t bit=b<128U?(uint8_t)((e[b/8U]>>(7U-b%8U))&1U):(uint8_t)((h[0]>>(7U-(b-128U)))&1U);idx[b/11U]=(uint16_t)((idx[b/11U]<<1)|bit);}size_t used=0;for(size_t i=0;i<12;i++){size_t n=strlen(words[idx[i]]);if(used+n+(i?1U:0U)+1U>cap)return false;if(i)out[used++]=' ';memcpy(out+used,words[idx[i]],n);used+=n;}out[used]=0;return true;}
