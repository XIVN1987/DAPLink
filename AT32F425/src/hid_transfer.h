#ifndef __HID_TRANSFER_H__
#define __HID_TRANSFER_H__


#define USBD_VID	0x0416
#ifdef DAP_FW_V1
#define USBD_PID    0x5021
#else
#define USBD_PID    0x7687
#endif


/* Define EP number */
#define HID_INT_IN_EP       0x81
#define HID_INT_OUT_EP      0x01
#define CDC_INT_IN_EP     	0x82
#define CDC_BULK_IN_EP    	0x83
#define CDC_BULK_OUT_EP   	0x03


/* Define EP maximum packet size */
#define HID_INT_IN_SZ       64
#define HID_INT_OUT_SZ      64
#define CDC_INT_IN_SZ     	8
#define CDC_BULK_IN_SZ    	64
#define CDC_BULK_OUT_SZ   	64


#define WINUSB_VENDOR_CODE	0x34


uint8_t usbd_hid_process(void);


void HID_SetInReport(void);
void HID_GetOutReport(uint8_t * buf, uint32_t len);


#endif // __HID_TRANSFER_H__
