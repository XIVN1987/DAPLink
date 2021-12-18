#include <stdio.h>
#include <string.h>
#include "NuMicro.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

//------------------------------------------------------------------
    if(u32IntSts & USBD_INTSTS_FLDET)
    {
        // Floating detect
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if(USBD_IS_ATTACHED())
        {
            /* USB Plug In */
            USBD_ENABLE_USB();
        }
        else
        {
            /* USB Un-plug */
            USBD_DISABLE_USB();
        }
    }

//------------------------------------------------------------------
    if(u32IntSts & USBD_INTSTS_BUS)
    {
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if(u32State & USBD_STATE_USBRST)
        {
            /* Bus reset */
            USBD_ENABLE_USB();
            USBD_SwReset();
        }
        if(u32State & USBD_STATE_SUSPEND)
        {
            /* Enable USB but disable PHY */
            USBD_DISABLE_PHY();
        }
        if(u32State & USBD_STATE_RESUME)
        {
            /* Enable USB and enable PHY */
            USBD_ENABLE_USB();
        }
    }

//------------------------------------------------------------------
    if(u32IntSts & USBD_INTSTS_WAKEUP)
    {
        USBD_CLR_INT_FLAG(USBD_INTSTS_WAKEUP);
    }

//------------------------------------------------------------------
    if(u32IntSts & USBD_INTSTS_USB)
    {
        if(u32IntSts & USBD_INTSTS_SETUP)
        {
            // Setup packet
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

            /* Clear the data IN/OUT ready flag of control end-points */
            USBD_STOP_TRANSACTION(EP0);
            USBD_STOP_TRANSACTION(EP1);

            USBD_ProcessSetupPacket();
        }

        if(u32IntSts & USBD_INTSTS_EP0)			// control IN
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
            
            USBD_CtrlIn();
        }

        if(u32IntSts & USBD_INTSTS_EP1)			// control OUT
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);

            USBD_CtrlOut();
			
			// In ACK of SET_LINE_CODE
			extern uint8_t g_usbd_SetupPacket[];
            if(g_usbd_SetupPacket[1] == SET_LINE_CODE)
            {
                if(g_usbd_SetupPacket[4] == 1)	// Interface number
                    VCOM_LineCoding();
            }
        }

        if(u32IntSts & USBD_INTSTS_EP2)			// Interrupt IN
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            
            EP2_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP3)			// Interrupt OUT
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            
            EP3_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP4)
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
        }

        if(u32IntSts & USBD_INTSTS_EP5)			// Bulk IN
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
			
            EP5_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP6)			// Bulk OUT
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
			
            EP6_Handler();
        }
    }
}


void EP2_Handler(void)  	// Interrupt IN handler
{
    HID_SetInReport();
}


void EP3_Handler(void)  	// Interrupt OUT handler
{
    uint8_t *ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3));
	
    HID_GetOutReport(ptr, USBD_GET_PAYLOAD_LEN(EP3));
	
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);
}


void HID_Init(void)
{
    /* Init setup packet buffer */
    /* Buffer range for setup packet -> [0 ~ 0x7] */
    USBD->STBUFSEG = SETUP_BUF_BASE;

    /*****************************************************/
    /* EP0 ==> control IN endpoint, address 0 */
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    /* Buffer range for EP0 */
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);

    /* EP1 ==> control OUT endpoint, address 0 */
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    /* Buffer range for EP1 */
    USBD_SET_EP_BUF_ADDR(EP1, EP1_BUF_BASE);

    /*****************************************************/
    /* EP2 ==> Interrupt IN endpoint, address 1 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | HID_INT_IN_EP);
    /* Buffer range for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Interrupt OUT endpoint, address 1 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | HID_INT_OUT_EP);
    /* Buffer range for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
    /* trigger to receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

}


void HID_ClassRequest(void)
{
    uint8_t buf[8];

    USBD_GetSetupPacket(buf);

    if(buf[0] & 0x80) 	// Device to host
    {
        switch(buf[1])
        {
        case GET_REPORT:
        case GET_IDLE:
        case GET_PROTOCOL:
			/* Setup error, stall the device */
			USBD_SetStall(0);
			break;
		
		case GET_LINE_CODE:
            if(buf[4] == 1)
            {
                USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)), (uint8_t *)&LineCfg, 7);
			}

            /* Data stage */
            USBD_SET_DATA1(EP0);
            USBD_SET_PAYLOAD_LEN(EP0, 7);

            /* Status stage */
            USBD_PrepareCtrlOut(0,0);
            break;
		
        default:
            /* Setup error, stall the device */
            USBD_SetStall(0);
            break;
        }
    }
    else				// Host to device
    {
        switch(buf[1])
        {
        case SET_REPORT:
            if(buf[3] == 3)
            {
                /* Request Type = Feature */
                USBD_SET_DATA1(EP1);
                USBD_SET_PAYLOAD_LEN(EP1, 0);
            }
            break;
        
        case SET_IDLE:
            /* Status stage */
            USBD_SET_DATA1(EP0);
            USBD_SET_PAYLOAD_LEN(EP0, 0);
            break;
		
		case SET_PROTOCOL:
			 /* Setup error, stall the device */
            USBD_SetStall(0);
            break;
		
		case SET_CONTROL_LINE_STATE:
            if(buf[4] == 1)
            {
				vcom.hw_flow = (buf[3] << 8) | buf[2];
            }

            /* Status stage */
            USBD_SET_DATA1(EP0);
            USBD_SET_PAYLOAD_LEN(EP0, 0);
            break;

        case SET_LINE_CODE:
            if(buf[4] == 1)
                USBD_PrepareCtrlOut((uint8_t *)&LineCfg, 7);
            
            /* Status stage */
            USBD_SET_DATA1(EP0);
            USBD_SET_PAYLOAD_LEN(EP0, 0);
            break;
		
        default:
            /* Setup error, stall the device */
            USBD_SetStall(0);
            break;
        }
    }
}


/***************************************************************/
#include "DAP_Config.h"
#include "DAP.h"

static volatile uint8_t  USB_RequestFlag;       // Request  Buffer Usage Flag
static volatile uint32_t USB_RequestIn;         // Request  Buffer In  Index
static volatile uint32_t USB_RequestOut;        // Request  Buffer Out Index

static volatile uint8_t  USB_ResponseIdle = 1;  // Response Buffer Idle  Flag
static volatile uint8_t  USB_ResponseFlag;      // Response Buffer Usage Flag
static volatile uint32_t USB_ResponseIn;        // Response Buffer In  Index
static volatile uint32_t USB_ResponseOut;       // Response Buffer Out Index

static uint8_t  USB_Request [DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Request  Buffer
static uint8_t  USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Response Buffer


uint8_t usbd_hid_process(void)
{
	uint32_t n;

	// Process pending requests
	if((USB_RequestOut != USB_RequestIn) || USB_RequestFlag)
	{
		DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);

		// Update request index and flag
		n = USB_RequestOut + 1;
		if(n == DAP_PACKET_COUNT)
			n = 0;
		USB_RequestOut = n;

		if(USB_RequestOut == USB_RequestIn)
			USB_RequestFlag = 0;

		if(USB_ResponseIdle)
		{	// Request that data is send back to host
			USB_ResponseIdle = 0;
			
			USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2)), USB_Response[USB_ResponseIn], DAP_PACKET_SIZE);
			USBD_SET_PAYLOAD_LEN(EP2, DAP_PACKET_SIZE);
		}
		else
		{	// Update response index and flag
			n = USB_ResponseIn + 1;
			if (n == DAP_PACKET_COUNT)
				n = 0;
			USB_ResponseIn = n;

			if (USB_ResponseIn == USB_ResponseOut)
				USB_ResponseFlag = 1;
		}
		return 1;
	}
	return 0;
}


void HID_GetOutReport(uint8_t *EpBuf, uint32_t len)
{
    if(EpBuf[0] == ID_DAP_TransferAbort)
	{
		DAP_TransferAbort = 1;
		return;
	}
	
	if(USB_RequestFlag && (USB_RequestIn == USB_RequestOut))
		return;  // Discard packet when buffer is full

	// Store data into request packet buffer
	memcpy(USB_Request[USB_RequestIn], EpBuf, len);

	USB_RequestIn++;
	if(USB_RequestIn == DAP_PACKET_COUNT)
		USB_RequestIn = 0;
	if(USB_RequestIn == USB_RequestOut)
		USB_RequestFlag = 1;
}


void HID_SetInReport(void)
{
	if((USB_ResponseOut != USB_ResponseIn) || USB_ResponseFlag)
	{
		USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2)), USB_Response[USB_ResponseOut], DAP_PACKET_SIZE);
        USBD_SET_PAYLOAD_LEN(EP2, DAP_PACKET_SIZE);
		
		USB_ResponseOut++;
		if (USB_ResponseOut == DAP_PACKET_COUNT)
			USB_ResponseOut = 0;
		if (USB_ResponseOut == USB_ResponseIn)
			USB_ResponseFlag = 0;
	}
	else
	{
		USB_ResponseIdle = 1;
	}
}
