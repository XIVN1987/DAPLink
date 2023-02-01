#ifndef __HID_TRANSFER_H__
#define __HID_TRANSFER_H__


#define USBD_VID	0x0416
#ifdef DAP_FW_V1
#define USBD_PID    0x5921
#else
#define USBD_PID    0x7987
#endif


/* EP number */
#define HID_INT_IN_EP       0x81
#define HID_INT_OUT_EP      0x01
#define CDC_INT_IN_EP     	0x82
#define CDC_BULK_IN_EP    	0x83
#define CDC_BULK_OUT_EP   	0x03


/* EP maximum packet size */
#define EP0_MAX_PKT_SIZE	64

#define HID_INT_IN_SZ_FS  	64
#define HID_INT_OUT_SZ_FS  	64
#define CDC_INT_IN_SZ_FS    8
#define CDC_BULK_IN_SZ_FS   64
#define CDC_BULK_OUT_SZ_FS  64

#define HID_INT_IN_SZ_HS	128
#define HID_INT_OUT_SZ_HS	128
#define CDC_INT_IN_SZ_HS    8
#define CDC_BULK_IN_SZ_HS   128
#define CDC_BULK_OUT_SZ_HS  128


#define EP0_BUF_BASE    	0
#define EP0_BUF_SIZE		EP0_MAX_PKT_SIZE
#define HID_INT_IN_BUF_BA	(EP0_BUF_BASE + EP0_BUF_SIZE)
#define HID_INT_IN_BUF_SZ	HID_INT_IN_SZ_HS
#define HID_INT_OUT_BUF_BA	(HID_INT_IN_BUF_BA + HID_INT_IN_BUF_SZ)
#define HID_INT_OUT_BUF_SZ	HID_INT_OUT_SZ_HS
#define CDC_INT_IN_BUF_BA	(HID_INT_OUT_BUF_BA + HID_INT_OUT_BUF_SZ)
#define CDC_INT_IN_BUF_SZ	CDC_INT_IN_SZ_HS
#define CDC_BULK_IN_BUF_BA	(CDC_INT_IN_BUF_BA + CDC_INT_IN_BUF_SZ)
#define CDC_BULK_IN_BUF_SZ	CDC_BULK_IN_SZ_HS
#define CDC_BULK_OUT_BUF_BA	(CDC_BULK_IN_BUF_BA + CDC_BULK_IN_BUF_SZ)
#define CDC_BULK_OUT_BUF_SZ	CDC_BULK_OUT_SZ_HS



/* HID Class Specific Request */
#define GET_REPORT          0x01
#define GET_IDLE            0x02
#define GET_PROTOCOL        0x03
#define SET_REPORT          0x09
#define SET_IDLE            0x0A
#define SET_PROTOCOL        0x0B


#define WINUSB_VENDOR_CODE	0x34



void HID_Init(void);
void HID_ClassRequest(void);
uint8_t usbd_hid_process(void);

void WINUSB_VendorRequest(void);

void HID_SetInReport(void);
void HID_GetOutReport(uint8_t *buf, uint32_t len);


#endif
