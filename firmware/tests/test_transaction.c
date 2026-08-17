#include "bc2_transaction.h"
#include "bc2_psbt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"CHECK failed: %s line %d\n",#x,__LINE__); return EXIT_FAILURE; } } while(0)
int main(void){
    bc2_tx_utxo u[2]; memset(u,0,sizeof(u));
    memset(u[0].txid,0x11,32);u[0].output_index=1;u[0].amount=60000;u[0].script[0]=0;u[0].script[1]=20;memset(u[0].script+2,0x22,20);u[0].script_length=22;
    memset(u[1].txid,0x33,32);u[1].output_index=2;u[1].amount=50000;memcpy(u[1].script,u[0].script,22);u[1].script_length=22;
    bc2_tx_plan p; CHECK(bc2_transaction_plan(u,2,70000,2,546,&p)==BC2_TX_OK);CHECK(p.selected_input_count==2);CHECK(p.change_output_created==1);CHECK(p.fee_amount==2*(10+2*68+2*31));
    uint8_t recipient[22]={0,20},change[22]={0,20};memset(recipient+2,0x44,20);memset(change+2,0x55,20);
    uint8_t psbt[2048];size_t len=0;CHECK(bc2_psbt_create(u,2,&p,recipient,22,change,22,psbt,sizeof(psbt),&len)==BC2_TX_OK);
    bc2_owned_script own;memset(&own,0,sizeof(own));memcpy(own.bytes,change,22);own.length=22;own.is_change=1;bc2_psbt_summary summary;
    CHECK(bc2_psbt_review(psbt,len,&own,1,"bc",&summary)==BC2_PSBT_OK);CHECK(summary.input_count==2);CHECK(summary.external_output_amount==70000);CHECK(summary.change_amount==p.change_amount);CHECK(summary.fee_amount==p.fee_amount);
    uint8_t script[32];size_t sl=0;CHECK(bc2_address_to_script("bc1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq9e75rs",bc2_network_mainnet(),script,sizeof(script),&sl)==BC2_TX_OK);CHECK(sl==22);
    CHECK(bc2_address_to_script("not-an-address",bc2_network_mainnet(),script,sizeof(script),&sl)==BC2_TX_INVALID_ADDRESS);
    CHECK(bc2_transaction_plan(u,1,70000,2,546,&p)==BC2_TX_INSUFFICIENT_FUNDS);
    return EXIT_SUCCESS;
}
