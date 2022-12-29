#include <string.h>
#include "usb_std.h"
#include "usbd_sdr.h"
#include "usbd_core.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


static usb_setup_type Setup;

static uint8_t hid_rx_buff[64];
uint8_t cdc_rx_buff[64];


/**
  * @brief  initialize usb endpoint
  * @param  udev: to the structure of usbd_core_type
  * @retval status of usb_sts_type
  */
static usb_sts_type class_init_handler(void *udev)
{
	usbd_core_type *usbd = (usbd_core_type *)udev;
	
	VCOM_Init();
	
#ifdef DAP_FW_V1
	usbd_ept_open(usbd, HID_INT_IN_EP,  EPT_INT_TYPE, HID_INT_IN_SZ);
	usbd_ept_open(usbd, HID_INT_OUT_EP, EPT_INT_TYPE, HID_INT_OUT_SZ);
#else
	usbd_ept_open(usbd, HID_INT_IN_EP,  EPT_BULK_TYPE, HID_INT_IN_SZ);
	usbd_ept_open(usbd, HID_INT_OUT_EP, EPT_BULK_TYPE, HID_INT_OUT_SZ);
#endif

	usbd_ept_recv(usbd, HID_INT_OUT_EP, hid_rx_buff, HID_INT_OUT_SZ);
	
	usbd_ept_open(usbd, CDC_INT_IN_EP,   EPT_INT_TYPE,  CDC_INT_IN_SZ);
	usbd_ept_open(usbd, CDC_BULK_IN_EP,  EPT_BULK_TYPE, CDC_BULK_IN_SZ);
	usbd_ept_open(usbd, CDC_BULK_OUT_EP, EPT_BULK_TYPE, CDC_BULK_OUT_SZ);

	usbd_ept_recv(usbd, CDC_BULK_OUT_EP, cdc_rx_buff, CDC_BULK_OUT_SZ);

	return USB_OK;
}


/**
  * @brief  clear endpoint or other state
  * @param  udev: to the structure of usbd_core_type
  * @retval status of usb_sts_type
  */
static usb_sts_type class_clear_handler(void *udev)
{
	usbd_core_type *usbd = (usbd_core_type *)udev;

	usbd_ept_close(usbd, HID_INT_IN_EP);
	usbd_ept_close(usbd, HID_INT_OUT_EP);
	
	usbd_ept_close(usbd, CDC_INT_IN_EP);
	usbd_ept_close(usbd, CDC_BULK_IN_EP);
	usbd_ept_close(usbd, CDC_BULK_OUT_EP);

	return USB_OK;
}


/**
  * @brief  usb device class setup request handler
  * @param  udev: to the structure of usbd_core_type
  * @param  setup: setup packet
  * @retval status of usb_sts_type
  */
static usb_sts_type class_setup_handler(void *udev, usb_setup_type *setup)
{
	usbd_core_type *usbd = (usbd_core_type *)udev;
	
	memcpy(&Setup, setup, sizeof(usb_setup_type));
	
	switch(setup->bmRequestType & USB_REQ_TYPE_RESERVED)
	{
	case USB_REQ_TYPE_STANDARD:
		switch(setup->bRequest)
		{
		case USB_STD_REQ_GET_DESCRIPTOR:
			if((setup->wValue >> 8) == HID_REPORT_DESC)
			{
				extern uint8_t g_hid_report[];
				extern uint8_t g_config_descriptor[];
				
				usbd_ctrl_send(usbd, g_hid_report, MIN(g_config_descriptor[9+9+7], setup->wLength));
			}
			else if((setup->wValue >> 8) == HID_DESCRIPTOR_TYPE)
			{
				extern uint8_t g_config_descriptor[];
				
				usbd_ctrl_send(usbd, &g_config_descriptor[9+9], MIN(9, setup->wLength));
			}
			break;
		
		default:
			break;
		}
		break;
	
	/* class request */
	case USB_REQ_TYPE_CLASS:
		switch(setup->bRequest)
		{
		case GET_LINE_CODING:
			usbd_ctrl_send(usbd, (uint8_t *)&LineCfg, MIN(setup->wLength, 7));
			break;
		
		case SET_LINE_CODING:
			usbd_ctrl_recv(usbd, (uint8_t *)&LineCfg, MIN(setup->wLength, 7));
			break;
		
		case 0x22:	// SET_CONTROL_LINE
			Vcom.hw_flow = setup->wValue;
			break;
		
		case HID_REQ_SET_IDLE:			// set Idle Rate for a particular Input report
			break;
		
		case HID_REQ_GET_IDLE:
		case HID_REQ_SET_REPORT:		// 
		case HID_REQ_GET_REPORT:
		case HID_REQ_SET_PROTOCOL:		// switch between Boot Protocol and Report Protocol
		case HID_REQ_GET_PROTOCOL:
		default:
			usbd_ctrl_unsupport(usbd);
			break;
		}
		break;
	
	default:
		usbd_ctrl_unsupport(usbd);
		break;
	}
	
	return USB_OK;
}


/**
  * @brief  usb device endpoint 0 in status stage complete
  * @param  udev: to the structure of usbd_core_type
  * @retval status of usb_sts_type
  */
static usb_sts_type class_ept0_tx_handler(void *udev)
{
	return USB_OK;
}


/**
  * @brief  usb device endpoint 0 out status stage complete
  * @param  udev: to the structure of usbd_core_type
  * @retval status of usb_sts_type
  */
static usb_sts_type class_ept0_rx_handler(void *udev)
{
	if(Setup.bRequest == SET_LINE_CODING)
	{
		VCOM_LineCoding(&LineCfg);
	}
	
	return USB_OK;
}


/**
  * @brief  usb device transmision complete handler
  * @param  udev: to the structure of usbd_core_type
  * @param  ept_num: endpoint number
  * @retval status of usb_sts_type
  */
static usb_sts_type class_in_handler(void *udev, uint8_t ept_num)
{	
	switch(0x80 | ept_num)
	{
	case HID_INT_IN_EP:
		HID_SetInReport();
		break;
	
	case CDC_BULK_IN_EP:
		VCOM_InComplete();
		break;
	}
	
	return USB_OK;
}


/**
  * @brief  usb device endpoint receive data
  * @param  udev: to the structure of usbd_core_type
  * @param  ept_num: endpoint number
  * @retval status of usb_sts_type
  */
static usb_sts_type class_out_handler(void *udev, uint8_t ept_num)
{
	usbd_core_type *usbd = (usbd_core_type *)udev;

	uint32_t recv_len = usbd_get_recv_len(usbd, ept_num);

	switch(ept_num)
	{
		case HID_INT_OUT_EP:
			HID_GetOutReport(hid_rx_buff, recv_len);
			
			/* start receive next packet */
			usbd_ept_recv(usbd, HID_INT_OUT_EP, hid_rx_buff, HID_INT_OUT_SZ);
			break;
		
		case CDC_BULK_OUT_EP:
			VCOM_GetOutData(cdc_rx_buff, recv_len);
			break;
	}

	return USB_OK;
}


/**
  * @brief  usb device sof handler
  * @param  udev: to the structure of usbd_core_type
  * @retval status of usb_sts_type
  */
static usb_sts_type class_sof_handler(void *udev)
{
	return USB_OK;
}


/**
  * @brief  usb device event handler
  * @param  udev: to the structure of usbd_core_type
  * @param  event: usb device event
  * @retval status of usb_sts_type
  */
static usb_sts_type class_event_handler(void *udev, usbd_event_type event)
{
	switch(event)
	{
	case USBD_RESET_EVENT:
		break;
	
	case USBD_SUSPEND_EVENT:
		break;
	
	case USBD_WAKEUP_EVENT:
		break;
	
	default:
		break;
	}
	
	return USB_OK;
}


usbd_class_handler hid_class_handler =
{
	class_init_handler,
	class_clear_handler,
	class_setup_handler,
	class_ept0_tx_handler,
	class_ept0_rx_handler,
	class_in_handler,
	class_out_handler,
	class_sof_handler,
	class_event_handler,
	NULL
};



/***************************************************************/
#include "DAP_Config.h"
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

extern otg_core_type Otg;

uint8_t usbd_hid_process(void)
{
	uint32_t n;

	if((USB_RequestOut != USB_RequestIn) || USB_RequestFull)
	{
		USB_ResponseSize[USB_ResponseIn] = DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);

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
			usbd_ept_send(&Otg.dev, HID_INT_IN_EP, USB_Response[USB_ResponseIn], DAP_PACKET_SIZE);
#else
			usbd_ept_send(&Otg.dev, HID_INT_IN_EP, USB_Response[USB_ResponseIn], USB_ResponseSize[USB_ResponseIn]);
#endif
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


void HID_GetOutReport(uint8_t * buf, uint32_t len)
{
    if(buf[0] == ID_DAP_TransferAbort)
	{
		DAP_TransferAbort = 1;
		return;
	}
	
	if((USB_RequestIn == USB_RequestOut) && USB_RequestFull)	// Request  Buffer Full
		return;

	memcpy(USB_Request[USB_RequestIn], buf, len);

	USB_RequestIn++;
	if(USB_RequestIn == DAP_PACKET_COUNT)
		USB_RequestIn = 0;
	
	if(USB_RequestIn == USB_RequestOut)
		USB_RequestFull = 1;
}


void HID_SetInReport(void)
{
	if((USB_ResponseOut != USB_ResponseIn) || USB_ResponseFull)
	{
#ifdef DAP_FW_V1
		usbd_ept_send(&Otg.dev, HID_INT_IN_EP, USB_Response[USB_ResponseOut], DAP_PACKET_SIZE);
#else
		usbd_ept_send(&Otg.dev, HID_INT_IN_EP, USB_Response[USB_ResponseOut], USB_ResponseSize[USB_ResponseOut]);
#endif
		
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
