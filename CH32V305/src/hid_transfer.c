#include "ch32v30x.h"
#include "ch32v30x_usb.h"
#include "ch32v30x_usbhs_device.h"

#include "usb_desc.h"
#include "hid_transfer.h"

#include "DAP_config.h"
#include "DAP.h"


static volatile uint8_t  USB_RequestFull;       // Request  Buffer Full Flag
static volatile uint32_t USB_RequestIn;         // Request  Buffer In  Index
static volatile uint32_t USB_RequestOut;        // Request  Buffer Out Index

static volatile uint8_t  USB_ResponseIdle = 1;  // Response Buffer Idle Flag
static volatile uint8_t  USB_ResponseFull;      // Response Buffer Full Flag
static volatile uint32_t USB_ResponseIn;        // Response Buffer In  Index
static volatile uint32_t USB_ResponseOut;       // Response Buffer Out Index

static uint8_t  USB_Request [DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Request  Buffer
static uint8_t  USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Response Buffer
static uint16_t USB_ResponseSize[DAP_PACKET_COUNT];				  // number of bytes in response


uint8_t usbd_hid_process(void)
{
	uint32_t n;

	if((USB_RequestOut != USB_RequestIn) || USB_RequestFull)
	{
		USB_ResponseSize[USB_ResponseIn] = DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);

		// Update request index and flag
		n = USB_RequestOut + 1;
		if(n == DAP_PACKET_COUNT)
			n = 0;
		USB_RequestOut = n;

		if(USB_RequestOut == USB_RequestIn)
			USB_RequestFull = 0;

		if(USB_ResponseIdle)
		{
			USB_ResponseIdle = 0;

#ifdef DAP_FW_V1
			n = DAP_PACKET_SIZE;
#else
			n = USB_ResponseSize[USB_ResponseIn];
#endif
			memcpy(USBHS_EP1_Tx_Buf, USB_Response[USB_ResponseIn], n);
			USBHSD->UEP1_TX_LEN = n;
			USBHSD->UEP1_TX_CTRL = (USBHSD->UEP1_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_ACK;
		}
		else
		{
			n = USB_ResponseIn + 1;
			if(n == DAP_PACKET_COUNT)
				n = 0;
			USB_ResponseIn = n;

			if(USB_ResponseIn == USB_ResponseOut)
				USB_ResponseFull = 1;
		}
		
		return 1;
	}
	
	return 0;
}


void HID_GetOutReport(uint8_t *buf, uint32_t len)
{
    if(buf[0] == ID_DAP_TransferAbort)
	{
		DAP_TransferAbort = 1;
		return;
	}
	
	if(USB_RequestFull && (USB_RequestIn == USB_RequestOut))
		return;  // Discard packet when buffer is full

	// Store data into request packet buffer
	memcpy(USB_Request[USB_RequestIn], buf, len);

	USB_RequestIn++;
	if(USB_RequestIn == DAP_PACKET_COUNT)
		USB_RequestIn = 0;
	if(USB_RequestIn == USB_RequestOut)
		USB_RequestFull = 1;
}


void HID_SetInReport(void)
{
    uint32_t n;

	if((USB_ResponseOut != USB_ResponseIn) || USB_ResponseFull)
	{
#ifdef DAP_FW_V1
	    n = DAP_PACKET_SIZE;
#else
	    n = USB_ResponseSize[USB_ResponseOut];
#endif
        memcpy(USBHS_EP1_Tx_Buf, USB_Response[USB_ResponseOut], n);
        USBHSD->UEP1_TX_LEN = n;
        USBHSD->UEP1_TX_CTRL = (USBHSD->UEP1_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_ACK;
		
		USB_ResponseOut++;
		if(USB_ResponseOut == DAP_PACKET_COUNT)
			USB_ResponseOut = 0;
		if(USB_ResponseOut == USB_ResponseIn)
			USB_ResponseFull = 0;
	}
	else
	{
		USB_ResponseIdle = 1;
	}
}
