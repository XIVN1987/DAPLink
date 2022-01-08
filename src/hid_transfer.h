#ifndef __HID_TRANSFER_H__
#define __HID_TRANSFER_H__


#define USBD_VID	0x0416
#define USBD_PID    0x5021


/* Define the EP number */
#define HID_INT_IN_EP       1
#define HID_INT_OUT_EP      1
#define CDC_INT_IN_EP     	2
#define CDC_BULK_IN_EP    	3
#define CDC_BULK_OUT_EP   	3


/* Define EP maximum packet size */
#define EP0_MAX_PKT_SIZE    64
#define EP1_MAX_PKT_SIZE    EP0_MAX_PKT_SIZE
#define EP2_MAX_PKT_SIZE    64
#define EP3_MAX_PKT_SIZE    64
#define EP4_MAX_PKT_SIZE    8
#define EP5_MAX_PKT_SIZE    64
#define EP6_MAX_PKT_SIZE    64


#define SETUP_BUF_BASE  	0
#define SETUP_BUF_SIZE   	8
#define EP0_BUF_BASE    	(SETUP_BUF_BASE + SETUP_BUF_SIZE)
#define EP1_BUF_BASE    	(SETUP_BUF_BASE + SETUP_BUF_SIZE)
#define EP2_BUF_BASE    	(EP1_BUF_BASE + EP1_MAX_PKT_SIZE)
#define EP3_BUF_BASE    	(EP2_BUF_BASE + EP2_MAX_PKT_SIZE)
#define EP4_BUF_BASE        (EP3_BUF_BASE + EP3_MAX_PKT_SIZE)
#define EP5_BUF_BASE        (EP4_BUF_BASE + EP4_MAX_PKT_SIZE)
#define EP6_BUF_BASE        (EP5_BUF_BASE + EP5_MAX_PKT_SIZE)


/* HID Class Specific Request */
#define GET_REPORT          0x01
#define GET_IDLE            0x02
#define GET_PROTOCOL        0x03
#define SET_REPORT          0x09
#define SET_IDLE            0x0A
#define SET_PROTOCOL        0x0B



void HID_Init(void);
void HID_ClassRequest(void);
uint8_t usbd_hid_process(void);

void HID_SetInReport(void);
void HID_GetOutReport(uint8_t *pu8EpBuf, uint32_t u32Size);


#endif
