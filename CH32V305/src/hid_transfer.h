#ifndef __HID_TRANSFER_H__
#define __HID_TRANSFER_H__


uint8_t usbd_hid_process(void);


void HID_SetInReport(void);
void HID_GetOutReport(uint8_t *buf, uint32_t len);


#endif
