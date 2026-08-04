#include "bc2_psbt.h"
#include "bc2_network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"CHECK failed: %s line %d\n", #x,__LINE__); return EXIT_FAILURE; } } while (0)
static size_t put_u32(unsigned char*b,size_t o,unsigned int v){for(int i=0;i<4;i++)b[o++]=(unsigned char)(v>>(8*i));return o;}
static size_t put_u64(unsigned char*b,size_t o,unsigned long long v){for(int i=0;i<8;i++)b[o++]=(unsigned char)(v>>(8*i));return o;}
int main(void){
 unsigned char tx[256]={0},psbt[512]={0};size_t t=0,p=0;unsigned char ext[22]={0x00,0x14},chg[22]={0x00,0x14};memset(ext+2,0x11,20);memset(chg+2,0x22,20);
 t=put_u32(tx,t,2);tx[t++]=1;memset(tx+t,0xaa,32);t+=32;t=put_u32(tx,t,1);tx[t++]=0;t=put_u32(tx,t,0xffffffffU);tx[t++]=2;
 t=put_u64(tx,t,70000);tx[t++]=22;memcpy(tx+t,ext,22);t+=22;t=put_u64(tx,t,29000);tx[t++]=22;memcpy(tx+t,chg,22);t+=22;t=put_u32(tx,t,0);
 memcpy(psbt+p,"psbt\xff",5);p+=5;psbt[p++]=1;psbt[p++]=0;psbt[p++]=(unsigned char)t;memcpy(psbt+p,tx,t);p+=t;psbt[p++]=0;
 psbt[p++]=1;psbt[p++]=1;psbt[p++]=31;p=put_u64(psbt,p,100000);psbt[p++]=22;memcpy(psbt+p,ext,22);p+=22;psbt[p++]=0;psbt[p++]=0;psbt[p++]=0;
 bc2_owned_script own={.length=22,.is_change=1};memcpy(own.bytes,chg,22);bc2_psbt_summary s;
 CHECK(bc2_psbt_review(psbt,p,&own,1,bc2_network_mainnet()->bech32_hrp,&s)==BC2_PSBT_OK);CHECK(s.input_count==1);CHECK(s.output_count==2);CHECK(s.total_input_amount==100000);CHECK(s.total_output_amount==99000);CHECK(s.fee_amount==1000);CHECK(s.external_output_amount==70000);CHECK(s.change_amount==29000);CHECK(s.outputs[1].change==1);CHECK(s.outputs[0].address[0]!='\0');CHECK(s.change_verified==1);
 psbt[p-3]=0xff;CHECK(bc2_psbt_review(psbt,p,&own,1,bc2_network_mainnet()->bech32_hrp,&s)!=BC2_PSBT_OK);
 CHECK(bc2_psbt_inspect((const unsigned char*)"bad",3,&s)==BC2_PSBT_TRUNCATED);return EXIT_SUCCESS;}
