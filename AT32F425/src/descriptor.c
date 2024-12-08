#include "usb_std.h"
#include "usbd_core.h"
#include "hid_transfer.h"


ALIGNED_HEAD uint8_t g_hid_report[] ALIGNED_TAIL =
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


ALIGNED_HEAD static uint8_t g_device_descriptor[USB_DEVICE_DESC_LEN] ALIGNED_TAIL =
{
	USB_DEVICE_DESC_LEN,				// bLength
	USB_DESCIPTOR_TYPE_DEVICE,			// bDescriptorType
#ifdef DAP_FW_V1
	0x00, 0x02,             			// bcdUSB
#else
	0x01, 0x02,             			// bcdUSB
#endif
	0x00,               				// bDeviceClass
	0x00,               				// bDeviceSubClass
	0x00,               				// bDeviceProtocol
	USB_MAX_EP0_SIZE,   				// bMaxPacketSize0
	USBD_VID & 0xFF, USBD_VID >> 8,		// idVendor
	USBD_PID & 0xFF, USBD_PID >> 8,		// idProduct
	0x10, 0x01,         				// bcdDevice
	0x01,               				// iManufacture
	0x02,               				// iProduct
	0x03,               				// iSerialNumber
	0x01                				// bNumConfigurations
};


ALIGNED_HEAD uint8_t g_config_descriptor[] ALIGNED_TAIL =
{
	USB_DEVICE_CFG_DESC_LEN,			// bLength
	USB_DESCIPTOR_TYPE_CONFIGURATION,	// bDescriptorType
#ifdef DAP_FW_V1
#define TOTAL_LEN_	(USB_DEVICE_CFG_DESC_LEN + (USB_DEVICE_IF_DESC_LEN + 0x09 + USB_DEVICE_EPT_LEN + USB_DEVICE_EPT_LEN))
#else
#define TOTAL_LEN_	(USB_DEVICE_CFG_DESC_LEN + (USB_DEVICE_IF_DESC_LEN        + USB_DEVICE_EPT_LEN + USB_DEVICE_EPT_LEN))
#endif
#define TOTAL_LEN   (TOTAL_LEN_ + (8 + USB_DEVICE_IF_DESC_LEN + 5 + 5 + 4 + 5 + USB_DEVICE_EPT_LEN + USB_DEVICE_IF_DESC_LEN + USB_DEVICE_EPT_LEN + USB_DEVICE_EPT_LEN))
	TOTAL_LEN & 0xFF, TOTAL_LEN >> 8,	// wTotalLength
	1 + 2, 								// bNumInterfaces
	0x01,               				// bConfigurationValue
	0x00,               				// iConfiguration
	0x00,               				// bmAttributes, D6: self power  D5: remote wake-up
	0x64,               				// MaxPower, 100 * 2mA = 200mA
	
	// I/F descriptor: HID
	USB_DEVICE_IF_DESC_LEN,      		// bLength
	USB_DESCIPTOR_TYPE_INTERFACE,		// bDescriptorType
	0x00,               				// bInterfaceNumber
	0x00,               				// bAlternateSetting
	0x02,               				// bNumEndpoints    
#ifdef DAP_FW_V1
	0x03,               				// bInterfaceClass
#else
	0xFF,               				// bInterfaceClass
#endif
	0x00,               				// bInterfaceSubClass
	0x00,               				// bInterfaceProtocol
	0x05,               				// iInterface，必须是 5，因为 USB_INTERFACE_STRING == 5

#ifdef DAP_FW_V1
	// HID Descriptor
	0x09,            					// bLength
	HID_CLASS_DESC_HID,					// bDescriptorType
	0x10, 0x01,         				// HID Class Spec
	0x00,               				// H/W target country.
	0x01,               				// Number of HID class descriptors to follow.
	HID_CLASS_DESC_REPORT,				// Descriptor type.
	sizeof(g_hid_report) & 0xFF, 0x00,  // Total length of report descriptor.
#endif

	// CMSIS-DAP v2 WinUSB 要求第一个端点是 Bulk OUT，第二个端点是 Bulk IN
	// EP Descriptor: interrupt out.
	USB_DEVICE_EPT_LEN,             	// bLength
	USB_DESCIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	HID_INT_OUT_EP,						// bEndpointAddress    
#ifdef DAP_FW_V1
	USB_EPT_DESC_INTERRUPT,				// bmAttributes
#else
	USB_EPT_DESC_BULK,					// bmAttributes
#endif
	HID_INT_OUT_SZ, 0x00,				// wMaxPacketSize
	1,									// bInterval
	
	// EP Descriptor: interrupt in.
	USB_DEVICE_EPT_LEN,					// bLength
	USB_DESCIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	HID_INT_IN_EP,						// bEndpointAddress
#ifdef DAP_FW_V1
	USB_EPT_DESC_INTERRUPT,				// bmAttributes
#else
	USB_EPT_DESC_BULK,					// bmAttributes
#endif
	HID_INT_IN_SZ, 0x00,				// wMaxPacketSize
	1,									// bInterval

	// Interface Association Descriptor (IAD)
	0x08,               				// bLength
	0x0B,               				// bDescriptorType: IAD
	0x01,               				// bFirstInterface
	0x02,               				// bInterfaceCount
	0x02,               				// bFunctionClass: CDC
	0x02,               				// bFunctionSubClass
	0x01,               				// bFunctionProtocol
	0x00,               				// iFunction

	// I/F descriptor: CDC Control
	USB_DEVICE_IF_DESC_LEN,				// bLength
	USB_DESCIPTOR_TYPE_INTERFACE,		// bDescriptorType
	0x01,								// bInterfaceNumber
	0x00,								// bAlternateSetting
	0x01,								// bNumEndpoints
	0x02,								// bInterfaceClass
	0x02,								// bInterfaceSubClass
	0x01,								// bInterfaceProtocol
	0x00,								// iInterface

	// Communication Class Specified INTERFACE descriptor
	0x05,								// Size of the descriptor, in bytes
	0x24,								// CS_INTERFACE descriptor type
	0x00,								// Header functional descriptor subtype
	0x10, 0x01,							// Communication device compliant to the communication spec. ver. 1.10

	// Communication Class Specified INTERFACE descriptor
	0x05,								// Size of the descriptor, in bytes
	0x24,								// CS_INTERFACE descriptor type
	0x01,								// Call management functional descriptor
	0x00,								// BIT0: Whether device handle call management itself.
										// BIT1: Whether device can send/receive call management information over a Data Class Interface 0
	0x02,								// Interface number of data class interface optionally used for call management

	// Communication Class Specified INTERFACE descriptor
	0x04,								// Size of the descriptor, in bytes
	0x24,								// CS_INTERFACE descriptor type
	0x02,								// Abstract control management functional descriptor subtype
	0x00,								// bmCapabilities

	// Communication Class Specified INTERFACE descriptor
	0x05,								// bLength
	0x24,								// bDescriptorType: CS_INTERFACE descriptor type
	0x06,								// bDescriptorSubType
	0x01,								// bMasterInterface
	0x02,								// bSlaveInterface0

	// ENDPOINT descriptor
	USB_DEVICE_EPT_LEN,					// bLength
	USB_DESCIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	CDC_INT_IN_EP,						// bEndpointAddress
	USB_EPT_DESC_INTERRUPT,				// bmAttributes
	CDC_INT_IN_SZ, 0x00,				// wMaxPacketSize
	10,									// bInterval

	// I/F descriptor: CDC Data
	USB_DEVICE_IF_DESC_LEN,				// bLength
	USB_DESCIPTOR_TYPE_INTERFACE,		// bDescriptorType
	0x02,								// bInterfaceNumber
	0x00,								// bAlternateSetting
	0x02,								// bNumEndpoints
	0x0A,								// bInterfaceClass
	0x00,								// bInterfaceSubClass
	0x00,								// bInterfaceProtocol
	0x00,								// iInterface

	// ENDPOINT descriptor
	USB_DEVICE_EPT_LEN,					// bLength
	USB_DESCIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	CDC_BULK_IN_EP,						// bEndpointAddress
	USB_EPT_DESC_BULK,					// bmAttributes
	CDC_BULK_IN_SZ, 0x00,				// wMaxPacketSize
	0x00,								// bInterval

	// ENDPOINT descriptor
	USB_DEVICE_EPT_LEN,					// bLength
	USB_DESCIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	CDC_BULK_OUT_EP,					// bEndpointAddress
	USB_EPT_DESC_BULK,					// bmAttributes
	CDC_BULK_OUT_SZ, 0x00,				// wMaxPacketSize
	0x00,								// bInterval
};


ALIGNED_HEAD static uint8_t g_langid_string[] ALIGNED_TAIL =
{
	4,
	USB_DESCIPTOR_TYPE_STRING,
	0x09,
	0x04,
};


ALIGNED_HEAD static uint8_t g_vendor_string[] ALIGNED_TAIL =
{
	18,
    USB_DESCIPTOR_TYPE_STRING,
    'X', 0, 'I', 0, 'V', 0, 'N', 0, '1', 0, '9', 0, '8', 0, '7', 0
};


ALIGNED_HEAD static uint8_t g_product_string[] ALIGNED_TAIL =
{
	36,
    USB_DESCIPTOR_TYPE_STRING,
    'X', 0, 'V', 0, '-', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0, ' ', 0, 'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0, 'P', 0
};


ALIGNED_HEAD static uint8_t g_serial_string[] ALIGNED_TAIL =
{
	26,
	USB_DESCIPTOR_TYPE_STRING,
	'0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0, '9', 0, 'A', 0, 'B', 0
};


ALIGNED_HEAD static uint8_t g_interface_string[] ALIGNED_TAIL =
{
	36,
    USB_DESCIPTOR_TYPE_STRING,
    'X', 0, 'V', 0, '-', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0, ' ', 0, 'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0, 'P', 0
};


static usbd_desc_t *get_device_descriptor(void)
{
	static usbd_desc_t device_descriptor =
	{
		sizeof(g_device_descriptor),
		g_device_descriptor
	};
	
	return &device_descriptor;
}


static usbd_desc_t * get_device_qualifier(void)
{
	return NULL;
}


static usbd_desc_t *get_config_descriptor(void)
{
	static usbd_desc_t config_descriptor =
	{
		sizeof(g_config_descriptor),
		g_config_descriptor
	};

	return &config_descriptor;
}


static usbd_desc_t *get_langid_string(void)
{
	static usbd_desc_t langid_string =
	{
		sizeof(g_langid_string),
		g_langid_string
	};

	return &langid_string;
}


static usbd_desc_t *get_vendor_string(void)
{
	static usbd_desc_t vendor_string =
	{
		sizeof(g_vendor_string),
		g_vendor_string
	};

	return &vendor_string;
}


static usbd_desc_t *get_product_string(void)
{
	static usbd_desc_t product_string =
	{
		sizeof(g_product_string),
		g_product_string
	};

	return &product_string;
}


static void int_to_unicode(uint32_t value , uint8_t *pbuf , uint8_t len);
static usbd_desc_t *get_serial_string(void)
{
	static usbd_desc_t serial_string =
	{
		sizeof(g_serial_string),
		g_serial_string
	};

	uint32_t serial0, serial1, serial2;
	serial0 = *(uint32_t*)0x1FFFF7E8;
	serial1 = *(uint32_t*)0x1FFFF7EC;
	serial2 = *(uint32_t*)0x1FFFF7F0;

	serial0 += serial2;

	if(serial0 != 0)
	{
		int_to_unicode(serial0, &g_serial_string[2],  8);
		int_to_unicode(serial1, &g_serial_string[18], 4);
	}
	
	return &serial_string;
}


static usbd_desc_t *get_interface_string(void)
{
	static usbd_desc_t interface_string =
	{
		sizeof(g_interface_string),
		g_interface_string
	};

	return &interface_string;
}


usbd_desc_handler hid_desc_handler =
{
	get_device_descriptor,
	get_device_qualifier,
	get_config_descriptor,
	NULL,					// get_device_other_speed
	get_langid_string,
	get_vendor_string,
	get_product_string,
	get_serial_string,
	get_interface_string,
	NULL,					// get_device_config_string
};


static void int_to_unicode(uint32_t value , uint8_t *pbuf , uint8_t len)
{
	for(int i = 0 ; i < len ; i++)
	{
		if((value >> 28) < 0xA)
		{
			pbuf[2 * i] = (value >> 28) + '0';
		}
		else
		{
			pbuf[2 * i] = (value >> 28) + 'A' - 10;
		}

		pbuf[2 * i + 1] = 0;
		
		value = value << 4;
	}
}



ALIGNED_HEAD uint8_t BOS_Descriptor[] ALIGNED_TAIL =
{
	5,
	DESC_BOS,
	5+20+8, 0,					// wTotalLength
	1,							// bNumDeviceCaps
	
	/*** MS OS 2.0 descriptor platform capability descriptor ***/
	28,
	DESC_CAPABILITY,
	5,							// bDevCapabilityType: PLATFORM (05H)
	0x00,
	0xDF, 0x60, 0xDD, 0xD8,		// PlatformCapabilityUUID: MS_OS_20_Platform_Capability_ID (D8DD60DF-4589-4CC7-9CD2-659D9E648A9F)
	0x89, 0x45, 0xC7, 0x4C,
	0x9C, 0xD2, 0x65, 0x9D,
	0x9E, 0x64, 0x8A, 0x9F,
	
	0x00, 0x00, 0x03, 0x06,		// dwWindowsVersion: 0x06030000 for Windows 8.1
	10+8+20+128, 0x00,      	// wTotalLength: size of MS OS 2.0 descriptor set
    WINUSB_VENDOR_CODE,   		// bMS_VendorCode
    0x00,                 		// bAltEnumCmd
};

usbd_desc_t *get_bos_descriptor(void)
{
	static usbd_desc_t bos_descriptor =
	{
		sizeof(BOS_Descriptor),
		BOS_Descriptor
	};
	
	return &bos_descriptor;
}


#define MS_OS_20_SET_HEADER_DESCRIPTOR        0x00
#define MS_OS_20_SUBSET_HEADER_CONFIGURATION  0x01
#define MS_OS_20_SUBSET_HEADER_FUNCTION       0x02
#define MS_OS_20_FEATURE_COMPATIBLE_ID        0x03
#define MS_OS_20_FEATURE_REG_PROPERTY         0x04
#define MS_OS_20_FEATURE_MIN_RESUME_TIME      0x05
#define MS_OS_20_FEATURE_MODEL_ID             0x06
#define MS_OS_20_FEATURE_CCGP_DEVICE          0x07
#define MS_OS_20_FEATURE_VENDOR_REVISION      0x08

ALIGNED_HEAD uint8_t MS_OS_20_DescriptorSet[] ALIGNED_TAIL = 
{
	/*** Microsoft OS 2.0 Descriptor Set Header ***/
	10, 0,
	MS_OS_20_SET_HEADER_DESCRIPTOR, 0,
	0x00, 0x00, 0x03, 0x06,		// dwWindowsVersion: 0x06030000 for Windows 8.1
	10+8+20+128, 0,				// wTotalLength
	
	/*** Microsoft OS 2.0 function subset header ***/
	8, 0,
	MS_OS_20_SUBSET_HEADER_FUNCTION, 0,
	0,							// bFirstInterface, first interface to which this subset applies
	0,
	8+20+128, 0,				// wSubsetLength
	
	/*** Microsoft OS 2.0 compatible ID descriptor ***/
	20, 0,
	MS_OS_20_FEATURE_COMPATIBLE_ID, 0,
	'W',  'I',  'N',  'U',  'S',  'B',  0x00, 0x00,		// CompatibleID
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// SubCompatibleID
	
	/*** MS OS 2.0 registry property descriptor ***/
	128, 0,
	MS_OS_20_FEATURE_REG_PROPERTY, 0,
	1, 0,						// wPropertyDataType: 1 = Unicode REG_SZ
	40, 0x00,                  	// wPropertyNameLength
	'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0,
	'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0, 'G', 0, 'U', 0, 'I', 0, 'D', 0,   0, 0,		// PropertyName: "DeviceInterfaceGUID"
	78, 0x00, 					// wPropertyDataLength
	'{', 0, 'C', 0, 'D', 0, 'B', 0, '3', 0, 'B', 0, '5', 0, 'A', 0, 'D', 0, '-', 0,
	'2', 0, '9', 0, '3', 0, 'B', 0, '-', 0, '4', 0, '6', 0, '6', 0, '3', 0, '-', 0,
	'A', 0, 'A', 0, '3', 0, '6', 0, '-', 0, '1', 0, 'A', 0, 'A', 0, 'E', 0, '4', 0,
	'6', 0, '4', 0, '6', 0, '3', 0, '7', 0, '7', 0, '6', 0, '}', 0,   0, 0, 			// PropertyData: "{CDB3B5AD-293B-4663-AA36-1AAE46463776}"
};
