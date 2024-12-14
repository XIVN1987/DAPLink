#ifndef __USB_DESC_H__
#define __USB_DESC_H__


#define USBD_VID    0x1A86
#ifdef DAP_FW_V1
#define USBD_PID    0x5321
#else
#define USBD_PID    0x7321
#endif


/* EP number */
#define HID_INT_IN_EP       0x81
#define HID_INT_OUT_EP      0x01
#define CDC_INT_IN_EP       0x82
#define CDC_BULK_IN_EP      0x83
#define CDC_BULK_OUT_EP     0x03


/* EP maximum packet size */
#define USB_MAX_EP0_SZ      64

#define HID_INT_IN_SZ_FS    64
#define HID_INT_OUT_SZ_FS   64
#define CDC_INT_IN_SZ_FS    8
#define CDC_BULK_IN_SZ_FS   64
#define CDC_BULK_OUT_SZ_FS  64

#define HID_INT_IN_SZ_HS    512
#define HID_INT_OUT_SZ_HS   512
#define CDC_INT_IN_SZ_HS    8
#define CDC_BULK_IN_SZ_HS   512
#define CDC_BULK_OUT_SZ_HS  512


/* Descriptor size */
#define USB_SIZE_DEVICE_DESC        18
#define USB_SIZE_CONFIG_DESC        9
#define USB_SIZE_INTERFACE_DESC     9
#define USB_SIZE_ENDPOINT_DESC      7
#define USB_SIZE_QUALIFIER_DESC     10
#define USB_SIZE_HID_DESC           9
#define USB_SIZE_IAD_DESC           8
#define USB_SIZE_STRING_LANGID      4
#define USB_SIZE_STRING_VENDOR      18
#define USB_SIZE_STRING_PRODUCT     36
#define USB_SIZE_STRING_SERIALNBR   26


#ifdef DAP_FW_V1
#define USB_SIZE_CONFIG_TOTAL_      (USB_SIZE_CONFIG_DESC + (USB_SIZE_INTERFACE_DESC + USB_SIZE_HID_DESC + USB_SIZE_ENDPOINT_DESC + USB_SIZE_ENDPOINT_DESC))
#else
#define USB_SIZE_CONFIG_TOTAL_      (USB_SIZE_CONFIG_DESC + (USB_SIZE_INTERFACE_DESC                     + USB_SIZE_ENDPOINT_DESC + USB_SIZE_ENDPOINT_DESC))
#endif
#define USB_SIZE_CONFIG_TOTAL       (USB_SIZE_CONFIG_TOTAL_ + (USB_SIZE_IAD_DESC + USB_SIZE_INTERFACE_DESC + 5 + 5 + 4 + 5 + USB_SIZE_ENDPOINT_DESC + USB_SIZE_INTERFACE_DESC + USB_SIZE_ENDPOINT_DESC + USB_SIZE_ENDPOINT_DESC))


extern const uint8_t USB_DevDesc[];
extern const uint8_t USB_CfgDesc_FS[];
extern const uint8_t USB_CfgDesc_HS[];
extern const uint8_t USB_StringLangID[];
extern const uint8_t USB_StringVendor[];
extern const uint8_t USB_StringProduct[];
extern       uint8_t USB_StringSerialNbr[];
extern const uint8_t USB_QualifierDesc[];
extern const uint8_t HID_ReportDesc_FS[];
extern const uint8_t HID_ReportDesc_HS[];


#endif // __USB_DESC_H__
