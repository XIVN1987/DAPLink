/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_desc.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/08/08
 * Description        : This file contains all the functions prototypes for the  
 *                      USB description firmware library.
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/ 
#ifndef __USB_DESC_H
#define __USB_DESC_H


#include "ch32v20x.h"


#define USBD_VID    0x1A86
#ifdef DAP_FW_V1
#define USBD_PID    0x5021
#else
#define USBD_PID    0x7021
#endif


/* Define EP number */
#define HID_INT_IN_EP       0x81
#define HID_INT_OUT_EP      0x01
#define CDC_INT_IN_EP       0x82
#define CDC_BULK_IN_EP      0x83
#define CDC_BULK_OUT_EP     0x03


/* Define EP maximum packet size */
#define USB_MAX_EP0_SZ      64
#define HID_INT_IN_SZ       64
#define HID_INT_OUT_SZ      64
#define CDC_INT_IN_SZ       8
#define CDC_BULK_IN_SZ      64
#define CDC_BULK_OUT_SZ     64


#define USBD_SIZE_DEVICE_DESC       18
#define USBD_SIZE_CONFIG_DESC       9
#define USBD_SIZE_INTERFACE_DESC    9
#define USBD_SIZE_ENDPOINT_DESC     7
#define USBD_SIZE_STRING_LANGID     4
#define USBD_SIZE_STRING_VENDOR     18
#define USBD_SIZE_STRING_PRODUCT    36
#define USBD_SIZE_STRING_SERIAL     26
#define USBD_SIZE_REPORT_DESC       29

#ifdef DAP_FW_V1
#define USBD_SIZE_CONFIG_TOTAL_     (USBD_SIZE_CONFIG_DESC + (USBD_SIZE_INTERFACE_DESC + 0x09 + USBD_SIZE_ENDPOINT_DESC + USBD_SIZE_ENDPOINT_DESC))
#else
#define USBD_SIZE_CONFIG_TOTAL_     (USBD_SIZE_CONFIG_DESC + (USBD_SIZE_INTERFACE_DESC        + USBD_SIZE_ENDPOINT_DESC + USBD_SIZE_ENDPOINT_DESC))
#endif
#define USBD_SIZE_CONFIG_TOTAL      (USBD_SIZE_CONFIG_TOTAL_ + (8 + USBD_SIZE_INTERFACE_DESC + 5 + 5 + 4 + 5 + USBD_SIZE_ENDPOINT_DESC + USBD_SIZE_INTERFACE_DESC + USBD_SIZE_ENDPOINT_DESC + USBD_SIZE_ENDPOINT_DESC))


extern const uint8_t USBD_DeviceDescriptor[];
extern const uint8_t USBD_ConfigDescriptor[];
extern const uint8_t USBD_StringLangID [];
extern const uint8_t USBD_StringVendor [];
extern const uint8_t USBD_StringProduct[];
extern const uint8_t USBD_StringInterface[];
extern const uint8_t USBD_HidRepDesc[];
extern uint8_t USBD_StringSerial [];


#define USB_EPT_DESC_CONTROL        0x00
#define USB_EPT_DESC_ISO            0x01
#define USB_EPT_DESC_BULK           0x02
#define USB_EPT_DESC_INTERRUPT      0x03


#define HID_CLASS_DESC_HID          0x21
#define HID_CLASS_DESC_REPORT       0x22


#endif /* __USB_DESC_H */
