/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_desc.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2019/10/15
 * Description        : USB Descriptors.
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
*******************************************************************************/ 
#include "usb_lib.h"
#include "usb_desc.h"


const uint8_t  USBD_DeviceDescriptor[] = {
    USBD_SIZE_DEVICE_DESC,              // bLength
    DEVICE_DESCRIPTOR,                  // bDescriptorType
#ifdef DAP_FW_V1
    0x00, 0x02,                         // bcdUSB
#else
    0x01, 0x02,                         // bcdUSB
#endif
    0x00,                               // bDeviceClass
    0x00,                               // bDeviceSubClass
    0x00,                               // bDeviceProtocol
    USB_MAX_EP0_SZ,                     // bMaxPacketSize0
    USBD_VID & 0xFF, USBD_VID >> 8,     // idVendor
    USBD_PID & 0xFF, USBD_PID >> 8,     // idProduct
    0x10, 0x01,                         // bcdDevice
    0x01,                               // iManufacture
    0x02,                               // iProduct
    0x03,                               // iSerialNumber
    0x01                                // bNumConfigurations
};


const uint8_t  USBD_ConfigDescriptor[] = {
    USBD_SIZE_CONFIG_DESC,              // bLength
    CONFIG_DESCRIPTOR,                  // bDescriptorType
    USBD_SIZE_CONFIG_TOTAL & 0xFF,
    USBD_SIZE_CONFIG_TOTAL >> 8,        // wTotalLength
    1 + 2,                              // bNumInterfaces
    0x01,                               // bConfigurationValue
    0x00,                               // iConfiguration
    0x00,                               // bmAttributes, D6: self power  D5: remote wake-up
    0x64,                               // MaxPower, 100 * 2mA = 200mA

    // I/F descriptor: HID
    USBD_SIZE_INTERFACE_DESC,           // bLength
    INTERFACE_DESCRIPTOR,               // bDescriptorType
    0x00,                               // bInterfaceNumber
    0x00,                               // bAlternateSetting
    0x02,                               // bNumEndpoints
#ifdef DAP_FW_V1
    0x03,                               // bInterfaceClass
#else
    0xFF,                               // bInterfaceClass
#endif
    0x00,                               // bInterfaceSubClass
    0x00,                               // bInterfaceProtocol
    0x00,                               // iInterface

#ifdef DAP_FW_V1
    // HID Descriptor
    0x09,                               // bLength
    HID_CLASS_DESC_HID,                 // bDescriptorType
    0x10, 0x01,                         // HID Class Spec
    0x00,                               // H/W target country.
    0x01,                               // Number of HID class descriptors to follow.
    HID_CLASS_DESC_REPORT,              // Descriptor type.
    USBD_SIZE_REPORT_DESC, 0x00,        // Total length of report descriptor.
#endif

    // CMSIS-DAP v2 WinUSB 要求第一个端点是 Bulk OUT，第二个端点是 Bulk IN
    // EP Descriptor: interrupt out.
    USBD_SIZE_ENDPOINT_DESC,            // bLength
    ENDPOINT_DESCRIPTOR,                // bDescriptorType
    HID_INT_OUT_EP,                     // bEndpointAddress
#ifdef DAP_FW_V1
    USB_EPT_DESC_INTERRUPT,             // bmAttributes
#else
    USB_EPT_DESC_BULK,                  // bmAttributes
#endif
    HID_INT_OUT_SZ, 0x00,               // wMaxPacketSize
    1,                                  // bInterval

    // EP Descriptor: interrupt in.
    USBD_SIZE_ENDPOINT_DESC,            // bLength
    ENDPOINT_DESCRIPTOR,                // bDescriptorType
    HID_INT_IN_EP,                      // bEndpointAddress
#ifdef DAP_FW_V1
    USB_EPT_DESC_INTERRUPT,             // bmAttributes
#else
    USB_EPT_DESC_BULK,                  // bmAttributes
#endif
    HID_INT_IN_SZ, 0x00,                // wMaxPacketSize
    1,                                  // bInterval

    // Interface Association Descriptor (IAD)
    0x08,                               // bLength
    0x0B,                               // bDescriptorType: IAD
    0x01,                               // bFirstInterface
    0x02,                               // bInterfaceCount
    0x02,                               // bFunctionClass: CDC
    0x02,                               // bFunctionSubClass
    0x01,                               // bFunctionProtocol
    0x00,                               // iFunction

    // I/F descriptor: CDC Control
    USBD_SIZE_INTERFACE_DESC,           // bLength
    INTERFACE_DESCRIPTOR,               // bDescriptorType
    0x01,                               // bInterfaceNumber
    0x00,                               // bAlternateSetting
    0x01,                               // bNumEndpoints
    0x02,                               // bInterfaceClass
    0x02,                               // bInterfaceSubClass
    0x01,                               // bInterfaceProtocol
    0x00,                               // iInterface

    // Communication Class Specified INTERFACE descriptor
    0x05,                               // Size of the descriptor, in bytes
    0x24,                               // CS_INTERFACE descriptor type
    0x00,                               // Header functional descriptor subtype
    0x10, 0x01,                         // Communication device compliant to the communication spec. ver. 1.10

    // Communication Class Specified INTERFACE descriptor
    0x05,                               // Size of the descriptor, in bytes
    0x24,                               // CS_INTERFACE descriptor type
    0x01,                               // Call management functional descriptor
    0x00,                               // BIT0: Whether device handle call management itself.
                                        // BIT1: Whether device can send/receive call management information over a Data Class Interface 0
    0x02,                               // Interface number of data class interface optionally used for call management

    // Communication Class Specified INTERFACE descriptor
    0x04,                               // Size of the descriptor, in bytes
    0x24,                               // CS_INTERFACE descriptor type
    0x02,                               // Abstract control management functional descriptor subtype
    0x00,                               // bmCapabilities

    // Communication Class Specified INTERFACE descriptor
    0x05,                               // bLength
    0x24,                               // bDescriptorType: CS_INTERFACE descriptor type
    0x06,                               // bDescriptorSubType
    0x01,                               // bMasterInterface
    0x02,                               // bSlaveInterface0

    // ENDPOINT descriptor
    USBD_SIZE_ENDPOINT_DESC,            // bLength
    ENDPOINT_DESCRIPTOR,                // bDescriptorType
    CDC_INT_IN_EP,                      // bEndpointAddress
    USB_EPT_DESC_INTERRUPT,             // bmAttributes
    CDC_INT_IN_SZ, 0x00,                // wMaxPacketSize
    10,                                 // bInterval

    // I/F descriptor: CDC Data
    USBD_SIZE_INTERFACE_DESC,           // bLength
    INTERFACE_DESCRIPTOR,               // bDescriptorType
    0x02,                               // bInterfaceNumber
    0x00,                               // bAlternateSetting
    0x02,                               // bNumEndpoints
    0x0A,                               // bInterfaceClass
    0x00,                               // bInterfaceSubClass
    0x00,                               // bInterfaceProtocol
    0x00,                               // iInterface

    // ENDPOINT descriptor
    USBD_SIZE_ENDPOINT_DESC,            // bLength
    ENDPOINT_DESCRIPTOR,                // bDescriptorType
    CDC_BULK_IN_EP,                     // bEndpointAddress
    USB_EPT_DESC_BULK,                  // bmAttributes
    CDC_BULK_IN_SZ, 0x00,               // wMaxPacketSize
    0x00,                               // bInterval

    // ENDPOINT descriptor
    USBD_SIZE_ENDPOINT_DESC,            // bLength
    ENDPOINT_DESCRIPTOR,                // bDescriptorType
    CDC_BULK_OUT_EP,                    // bEndpointAddress
    USB_EPT_DESC_BULK,                  // bmAttributes
    CDC_BULK_OUT_SZ, 0x00,              // wMaxPacketSize
    0x00,                               // bInterval
};


const uint8_t USBD_StringLangID[] = {
	USBD_SIZE_STRING_LANGID,
	STRING_DESCRIPTOR,
	0x09, 0x04
};


const uint8_t USBD_StringVendor[] = {
	USBD_SIZE_STRING_VENDOR,    
	STRING_DESCRIPTOR,
	'X', 0, 'I', 0, 'V', 0, 'N', 0, '1', 0, '9', 0, '8', 0, '7', 0
};


const uint8_t USBD_StringProduct[] = {
    USBD_SIZE_STRING_PRODUCT,
    STRING_DESCRIPTOR,
    'X', 0, 'V', 0, '-', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0, ' ', 0, 'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0, 'P', 0
};


uint8_t USBD_StringSerial[] = {
	USBD_SIZE_STRING_SERIAL,          
	STRING_DESCRIPTOR,
	'0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0, '9', 0, 'A', 0, 'B', 0
};


const uint8_t USBD_HidRepDesc[] =
{
    0x06, 0x00, 0xFF,   // Usage Page = 0xFF00 (Vendor Defined Page 1)
    0x09, 0x01,         // Usage (Vendor Usage 1)
    0xA1, 0x01,         // Collection (Application)
    0x19, 0x01,         // Usage Minimum
    0x29, 0x40,         // Usage Maximum //64 input usages total (0x01 to 0x40)
    0x15, 0x00,         // Logical Minimum (data bytes in the report may have minimum value = 0x00)
    0x26, 0xFF, 0x00,   // Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255)
    0x75, 0x08,         // Report Size: 8-bit field size
    0x95, 0x40,         // Report Count: Make sixty-four 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
    0x81, 0x00,         // Input (Data, Array, Abs): Instantiates input packet fields based on the above report size, count, logical min/max, and usage.
    0x19, 0x01,         // Usage Minimum
    0x29, 0x40,         // Usage Maximum //64 output usages total (0x01 to 0x40)
    0x91, 0x00,         // Output (Data, Array, Abs): Instantiates output packet fields. Uses same report size and count as "Input" fields, since nothing new/different was specified to the parser since the "Input" item.
    0xC0                // End Collection
};

