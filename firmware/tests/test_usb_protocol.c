#include "bc2_usb_protocol.h"
#include <assert.h>
#include <string.h>
int main(void) {
    uint8_t frame[64]={0}; const uint8_t payload[]={0x70,0x69,0x6e,0x67};
    size_t n=bc2_usb_encode(BC2_USB_CMD_PING,0x1234U,payload,sizeof(payload),frame,sizeof(frame));
    assert(n==13U);
    bc2_usb_message_t msg={0};
    assert(bc2_usb_parse(frame,n,&msg)==BC2_USB_PARSE_OK);
    assert(msg.command==BC2_USB_CMD_PING && msg.sequence==0x1234U);
    assert(msg.payload_length==sizeof(payload) && memcmp(msg.payload,payload,sizeof(payload))==0);
    frame[0]=0; assert(bc2_usb_parse(frame,n,&msg)==BC2_USB_PARSE_MAGIC);
    return 0;
}
