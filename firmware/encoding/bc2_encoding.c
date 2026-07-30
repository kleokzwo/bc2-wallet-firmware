#include "bc2_encoding.h"
#include "bc2_crypto.h"
#include <string.h>
#include <limits.h>
static const char A[]="123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
bool bc2_hex_encode(const uint8_t*i,size_t n,char*o,size_t z,bool up){const char*h=up?"0123456789ABCDEF":"0123456789abcdef";if((n&&!i)||!o||n>(SIZE_MAX-1U)/2U||z<2U*n+1U)return false;for(size_t x=0;x<n;x++){o[2*x]=h[i[x]>>4];o[2*x+1]=h[i[x]&15U];}o[2*n]='\0';return true;}
static int hv(char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;}
bool bc2_hex_decode(const char*i,size_t n,uint8_t*o,size_t z,size_t*l){if(!l)return false;*l=0;if((n&&!i)||(n%2U)||z<n/2U||(n&& !o))return false;for(size_t x=0;x<n/2U;x++){int a=hv(i[2*x]),b=hv(i[2*x+1]);if(a<0||b<0)return false;o[x]=(uint8_t)((a<<4)|b);}*l=n/2U;return true;}
bool bc2_base58_encode(const uint8_t*i,size_t n,char*o,size_t z){if(!o||z==0U||(n&&!i))return false;o[0]='\0';if(n==0U)return true;uint8_t d[256]={0};size_t zeros=0,len=0;while(zeros<n&&i[zeros]==0)zeros++;for(size_t x=zeros;x<n;x++){unsigned c=i[x];size_t j=0;while(j<len||c){if(j>=sizeof d)return false;c+=(unsigned)d[j]*256U;d[j++]=(uint8_t)(c%58U);c/=58U;}if(j>len)len=j;}if(zeros+len+1U>z)return false;size_t k=0;while(k<zeros)o[k++]='1';while(len)o[k++]=A[d[--len]];o[k]='\0';return true;}
static int av(char c){const char*p=strchr(A,c);return p?(int)(p-A):-1;}
bool bc2_base58_decode(const char*i,size_t n,uint8_t*o,size_t z,size_t*l){if(!l)return false;*l=0;if((n&&!i)||(n&&!o))return false;uint8_t b[256]={0};size_t zeros=0,len=0;while(zeros<n&&i[zeros]=='1')zeros++;for(size_t x=zeros;x<n;x++){int v=av(i[x]);if(v<0)return false;unsigned c=(unsigned)v;size_t j=0;while(j<len||c){if(j>=sizeof b)return false;c+=(unsigned)b[j]*58U;b[j++]=(uint8_t)(c&255U);c>>=8U;}if(j>len)len=j;}if(zeros+len>z)return false;size_t k=0;while(k<zeros)o[k++]=0;while(len)o[k++]=b[--len];*l=k;return true;}
bool bc2_base58check_encode(const uint8_t*p,size_t n,char*o,size_t z){if((n&&!p)||n>128U)return false;uint8_t b[132],h[32];if(!bc2_sha256d(p,n,h))return false;memcpy(b,p,n);memcpy(b+n,h,4);return bc2_base58_encode(b,n+4,o,z);}
bool bc2_base58check_decode(const char*i,size_t n,uint8_t*p,size_t z,size_t*l){uint8_t b[132],h[32];size_t m=0;if(!l)return false;*l=0;if(!bc2_base58_decode(i,n,b,sizeof b,&m)||m<4U||m-4U>z||((m>4U)&&!p)||!bc2_sha256d(b,m-4U,h))return false;unsigned diff=0;for(size_t x=0;x<4;x++)diff|=(unsigned)(h[x]^b[m-4U+x]);if(diff)return false;memcpy(p,b,m-4U);*l=m-4U;return true;}
static uint32_t polymod_step(uint32_t pre){
 uint8_t b=(uint8_t)(pre>>25);uint32_t c=(pre&0x1ffffffU)<<5;
 c^=(b&1U)?0x3b6a57b2U:0U;c^=(b&2U)?0x26508e6dU:0U;c^=(b&4U)?0x1ea119faU:0U;c^=(b&8U)?0x3d4233ddU:0U;c^=(b&16U)?0x2a1462b3U:0U;return c;
}
static bool conv(const uint8_t*in,size_t n,int from,int to,bool pad,uint8_t*out,size_t cap,size_t*olen){uint32_t acc=0;int bits=0;size_t j=0;uint32_t maxv=((uint32_t)1<<to)-1U;for(size_t i=0;i<n;i++){if((in[i]>>from)!=0)return false;acc=(acc<<from)|in[i];bits+=from;while(bits>=to){bits-=to;if(j>=cap)return false;out[j++]=(uint8_t)((acc>>bits)&maxv);}}if(pad&&bits){if(j>=cap)return false;out[j++]=(uint8_t)((acc<<(to-bits))&maxv);}else if(bits>=from||((acc<<(to-bits))&maxv))return false;*olen=j;return true;}
bool bc2_bech32_segwit_encode(const char*hrp,uint8_t ver,const uint8_t*prog,size_t prog_len,char*out,size_t z){
 static const char C[]="qpzry9x8gf2tvdw0s3jn54khce6mua7l";
 if(!hrp||!prog||!out||ver>16U||prog_len<2U||prog_len>40U)return false;
 uint8_t data[65];size_t converted=0,dn=1U;data[0]=ver;
 if(!conv(prog,prog_len,8,5,true,data+1U,sizeof(data)-1U,&converted)) return false;
 dn+=converted;
 size_t hl=strlen(hrp);if(hl<1U||hl>83U||hl+1U+dn+6U+1U>z)return false;
 for(size_t i=0;i<hl;i++){unsigned char ch=(unsigned char)hrp[i];if(ch<33U||ch>126U||(ch>='A'&&ch<='Z'))return false;}
 uint32_t chk=1U;
 for(size_t i=0;i<hl;i++)chk=polymod_step(chk)^((uint8_t)hrp[i]>>5);
 chk=polymod_step(chk);
 for(size_t i=0;i<hl;i++)chk=polymod_step(chk)^((uint8_t)hrp[i]&31U);
 for(size_t i=0;i<dn;i++)chk=polymod_step(chk)^data[i];
 for(size_t i=0;i<6U;i++)chk=polymod_step(chk);
 chk^=(ver==0U)?1U:0x2bc830a3U;
 memcpy(out,hrp,hl);out[hl]='1';
 for(size_t i=0;i<dn;i++)out[hl+1U+i]=C[data[i]];
 for(size_t i=0;i<6U;i++)out[hl+1U+dn+i]=C[(chk>>(5U*(5U-i)))&31U];
 out[hl+1U+dn+6U]='\0';return true;
}
