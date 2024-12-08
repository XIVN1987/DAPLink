#ifndef __USB_CONF_H
#define __USB_CONF_H

#include <stdio.h>
#include "at32f425.h"


#define USE_OTG_DEVICE_MODE


#define USBD_RX_SIZE    	128
#define USBD_EP0_TX_SIZE	24
#define USBD_EP1_TX_SIZE	20
#define USBD_EP2_TX_SIZE	20
#define USBD_EP3_TX_SIZE	20
#define USBD_EP4_TX_SIZE	20
#define USBD_EP5_TX_SIZE	20
#define USBD_EP6_TX_SIZE	20
#define USBD_EP7_TX_SIZE	20



#define USB_VBUS_IGNORE		// ignore vbus detection , not use vbus pin



#define DESC_BOS            0x0F
#define DESC_CAPABILITY     0x10

#define WINUSB_VENDOR_CODE	0x34



void usb_delay_ms(uint32_t ms);


#endif
