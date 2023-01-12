#ifndef __CH32V30X_USBHS_DEVICE_H__
#define __CH32V30X_USBHS_DEVICE_H__


#define USB_EPT_IN          0x80
#define USB_EPT_OUT         0x00


#define USB_EPT_CONTROL     0x00
#define USB_EPT_ISO         0x01
#define USB_EPT_BULK        0x02
#define USB_EPT_INTERRUPT   0x03


extern uint8_t USBHS_EP1_Tx_Buf[];
extern uint8_t USBHS_EP1_Rx_Buf[];
extern uint8_t USBHS_EP2_Tx_Buf[];
extern uint8_t USBHS_EP3_Tx_Buf[];
extern uint8_t USBHS_EP3_Rx_Buf[];


extern void USBHS_RCC_Init(void);
extern void USBHS_Device_Init(FunctionalState sta);


#endif // __CH32V30X_USBHS_DEVICE_H__
