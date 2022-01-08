#include "NuMicro.h"
#include "hid_transfer.h"


uint8_t HID_DeviceReportDescriptor[] =
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


uint8_t gu8DeviceDescriptor[] =
{
    LEN_DEVICE,         // bLength
    DESC_DEVICE,        // bDescriptorType
    0x00, 0x02,         // bcdUSB
    0x00,               // bDeviceClass
    0x00,               // bDeviceSubClass
    0x00,               // bDeviceProtocol
    EP0_MAX_PKT_SIZE,   // bMaxPacketSize0
    USBD_VID & 0xFF, USBD_VID >> 8,  	// idVendor
    USBD_PID & 0xFF, USBD_PID >> 8,  	// idProduct
    0x00, 0x01,         // bcdDevice
    0x01,               // iManufacture
    0x02,               // iProduct
    0x00,               // iSerialNumber - no serial
    0x01                // bNumConfigurations
};


uint8_t gu8ConfigDescriptor[] =
{
    LEN_CONFIG,         // bLength
    DESC_CONFIG,        // bDescriptorType
#define TOTAL_LEN	(LEN_CONFIG + (LEN_INTERFACE + LEN_HID + LEN_ENDPOINT + LEN_ENDPOINT) + \
								  (8 + LEN_INTERFACE + 5 + 5 + 4 + 5 + LEN_ENDPOINT + LEN_INTERFACE + LEN_ENDPOINT + LEN_ENDPOINT))
	TOTAL_LEN & 0xFF, TOTAL_LEN >> 8,	// wTotalLength
    0x03, 				// bNumInterfaces
    0x01,               // bConfigurationValue
    0x00,               // iConfiguration
    0x00,               // bmAttributes, D6: self power  D5: remote wake-up
    0x64,               // MaxPower, 100 * 2mA = 200mA
	
	// I/F descriptor: HID
    LEN_INTERFACE,      // bLength
    DESC_INTERFACE,     // bDescriptorType
    0x00,               // bInterfaceNumber
    0x00,               // bAlternateSetting
    0x02,               // bNumEndpoints
    0x03,               // bInterfaceClass
    0x00,               // bInterfaceSubClass
    0x00,               // bInterfaceProtocol
    0x00,               // iInterface

    // HID Descriptor
    LEN_HID,            // bLength
    DESC_HID,           // bDescriptorType
    0x10, 0x01,         // HID Class Spec
    0x00,               // H/W target country.
    0x01,               // Number of HID class descriptors to follow.
    DESC_HID_RPT,       // Descriptor type.
    sizeof(HID_DeviceReportDescriptor) & 0xFF, sizeof(HID_DeviceReportDescriptor) >> 8,  // Total length of report descriptor.

    // EP Descriptor: interrupt in.
    LEN_ENDPOINT,               	// bLength
    DESC_ENDPOINT,              	// bDescriptorType
    (EP_INPUT | HID_INT_IN_EP), 	// bEndpointAddress
    EP_INT,                     	// bmAttributes
    EP2_MAX_PKT_SIZE, 0x00,  		// wMaxPacketSize
    1,        						// bInterval

    // EP Descriptor: interrupt out.
    LEN_ENDPOINT,                   // bLength
    DESC_ENDPOINT,            		// bDescriptorType
    (EP_OUTPUT | HID_INT_OUT_EP),	// bEndpointAddress
    EP_INT,                      	// bmAttributes
    EP3_MAX_PKT_SIZE, 0x00,  		// wMaxPacketSize
    1,        						// bInterval

    // Interface Association Descriptor (IAD)
    0x08,               // bLength
    0x0B,               // bDescriptorType: IAD
    0x01,               // bFirstInterface
    0x02,               // bInterfaceCount
    0x02,               // bFunctionClass: CDC
    0x02,               // bFunctionSubClass
    0x01,               // bFunctionProtocol
    0x00,               // iFunction, ÃèÊö×Ö·û´®Ë÷Òý

    // I/F descriptor: VCOM
    LEN_INTERFACE,      // bLength
    DESC_INTERFACE,     // bDescriptorType
    0x01,               // bInterfaceNumber
    0x00,               // bAlternateSetting
    0x01,               // bNumEndpoints
    0x02,               // bInterfaceClass
    0x02,               // bInterfaceSubClass
    0x01,               // bInterfaceProtocol
    0x00,               // iInterface

    // Communication Class Specified INTERFACE descriptor
    0x05,               // Size of the descriptor, in bytes
    0x24,               // CS_INTERFACE descriptor type
    0x00,               // Header functional descriptor subtype
    0x10, 0x01,         // Communication device compliant to the communication spec. ver. 1.10

    // Communication Class Specified INTERFACE descriptor
    0x05,               // Size of the descriptor, in bytes
    0x24,               // CS_INTERFACE descriptor type
    0x01,               // Call management functional descriptor
    0x00,               // BIT0: Whether device handle call management itself.
                        // BIT1: Whether device can send/receive call management information over a Data Class Interface 0
    0x02,               // Interface number of data class interface optionally used for call management

    // Communication Class Specified INTERFACE descriptor
    0x04,               // Size of the descriptor, in bytes
    0x24,               // CS_INTERFACE descriptor type
    0x02,               // Abstract control management functional descriptor subtype
    0x00,               // bmCapabilities

    // Communication Class Specified INTERFACE descriptor
    0x05,               // bLength
    0x24,               // bDescriptorType: CS_INTERFACE descriptor type
    0x06,               // bDescriptorSubType
    0x01,               // bMasterInterface
    0x02,               // bSlaveInterface0

    // ENDPOINT descriptor
    LEN_ENDPOINT,                   // bLength
    DESC_ENDPOINT,                  // bDescriptorType
    (EP_INPUT | CDC_INT_IN_EP),   	// bEndpointAddress
    EP_INT,                         // bmAttributes
    EP4_MAX_PKT_SIZE, 0x00,         // wMaxPacketSize
    10,                           	// bInterval

    // I/F descriptor
    LEN_INTERFACE,      // bLength
    DESC_INTERFACE,     // bDescriptorType
    0x02,               // bInterfaceNumber
    0x00,               // bAlternateSetting
    0x02,               // bNumEndpoints
    0x0A,               // bInterfaceClass
    0x00,               // bInterfaceSubClass
    0x00,               // bInterfaceProtocol
    0x00,               // iInterface

    // ENDPOINT descriptor
    LEN_ENDPOINT,                   // bLength
    DESC_ENDPOINT,                  // bDescriptorType
    (EP_INPUT | CDC_BULK_IN_EP),  	// bEndpointAddress
    EP_BULK,                        // bmAttributes
    EP5_MAX_PKT_SIZE, 0x00,         // wMaxPacketSize
    0x00,                           // bInterval

    // ENDPOINT descriptor
    LEN_ENDPOINT,                   // bLength
    DESC_ENDPOINT,                  // bDescriptorType
    (EP_OUTPUT | CDC_BULK_OUT_EP),	// bEndpointAddress
    EP_BULK,                        // bmAttributes
    EP6_MAX_PKT_SIZE, 0x00,         // wMaxPacketSize
    0x00,                           // bInterval
};


uint8_t gu8StringLang[] =
{
    4,
    DESC_STRING,
    0x09, 0x04
};

uint8_t gu8VendorStringDesc[] =
{
    18,
    DESC_STRING,
    'X', 0, 'I', 0, 'V', 0, 'N', 0, '1', 0, '9', 0, '8', 0, '7', 0
};

uint8_t gu8ProductStringDesc[] =
{
    30,
    DESC_STRING,
    'M', 0, '4', 0, '8', 0, '0', 0, ' ', 0, 'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0, 'P', 0
};

uint8_t *gpu8UsbString[] =
{
    gu8StringLang,
    gu8VendorStringDesc,
    gu8ProductStringDesc,
    NULL,
};


uint8_t *gu8UsbHidReport[] =
{
    HID_DeviceReportDescriptor,
    NULL,
};

uint32_t gu32UsbHidReportLen[] =
{
    sizeof(HID_DeviceReportDescriptor),
    0,
};

uint32_t gu32ConfigHidDescIdx[] =
{
    (LEN_CONFIG + LEN_INTERFACE),
    0,
};


uint8_t gu8BOSDescriptor[] =
{
    LEN_BOS,
    DESC_BOS,
    0x0C,  						// wTotalLength
    0x01,                       // bNumDeviceCaps

    /* Device Capability */
    LEN_BOSCAP,                 // bLength
    DESC_CAPABILITY,            // bDescriptorType
    CAP_USB20_EXT,              // bDevCapabilityType
    0x02, 0x00, 0x00, 0x00      // bmAttributes
};


const S_USBD_INFO_T gsInfo =
{
    (uint8_t *)gu8DeviceDescriptor,
    (uint8_t *)gu8ConfigDescriptor,
    (uint8_t **)gpu8UsbString,
    (uint8_t **)gu8UsbHidReport,
    (uint8_t *)gu8BOSDescriptor,
    (uint32_t *)gu32UsbHidReportLen,
    (uint32_t *)gu32ConfigHidDescIdx
};
