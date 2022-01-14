#include "NuMicro.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


void HID_Init(void)
{
    /* Init setup packet buffer */
    USBD->STBUFSEG = SETUP_BUF_BASE;

    /*****************************************************/
    /* EP0 ==> control IN endpoint, address 0 */
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);

    /* EP1 ==> control OUT endpoint, address 0 */
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    USBD_SET_EP_BUF_ADDR(EP1, EP1_BUF_BASE);

    /*****************************************************/
    /* EP2 ==> Interrupt IN endpoint, address 1 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | HID_INT_IN_EP);
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Interrupt OUT endpoint, address 1 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | HID_INT_OUT_EP);
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
    /* trigger to receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

	/*****************************************************/
	/* EP4 ==> Interrupt IN endpoint, address 2 */
    USBD_CONFIG_EP(EP4, USBD_CFG_EPMODE_IN | CDC_INT_IN_EP);
    USBD_SET_EP_BUF_ADDR(EP4, EP4_BUF_BASE);
	
    /* EP5 ==> Bulk IN endpoint, address 3 */
    USBD_CONFIG_EP(EP5, USBD_CFG_EPMODE_IN | CDC_BULK_IN_EP);
    USBD_SET_EP_BUF_ADDR(EP5, EP5_BUF_BASE);

    /* EP6 ==> Bulk Out endpoint, address 3 */
    USBD_CONFIG_EP(EP6, USBD_CFG_EPMODE_OUT | CDC_BULK_OUT_EP);
    USBD_SET_EP_BUF_ADDR(EP6, EP6_BUF_BASE);
    /* trigger receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP6, EP6_MAX_PKT_SIZE);
	
#if VCOM_COUNT > 1
	/*****************************************************/
	/* EP7 ==> Interrupt IN endpoint, address 4 */
    USBD_CONFIG_EP(EP7, USBD_CFG_EPMODE_IN | CDC2_INT_IN_EP);
    USBD_SET_EP_BUF_ADDR(EP7, EP7_BUF_BASE);
	
    /* EP8 ==> Bulk IN endpoint, address 5 */
    USBD_CONFIG_EP(EP8, USBD_CFG_EPMODE_IN | CDC2_BULK_IN_EP);
    USBD_SET_EP_BUF_ADDR(EP8, EP8_BUF_BASE);

    /* EP9 ==> Bulk Out endpoint, address 5 */
    USBD_CONFIG_EP(EP9, USBD_CFG_EPMODE_OUT | CDC2_BULK_OUT_EP);
    USBD_SET_EP_BUF_ADDR(EP9, EP9_BUF_BASE);
    /* trigger receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP9, EP9_MAX_PKT_SIZE);
#endif
}


void EP2_Handler(void)  	// Interrupt IN
{
    HID_SetInReport();
}

void EP3_Handler(void)  	// Interrupt OUT
{
    uint8_t *ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3));
	
    HID_GetOutReport(ptr, USBD_GET_PAYLOAD_LEN(EP3));
	
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);
}

void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

    if(u32IntSts & USBD_INTSTS_FLDET)			// Floating detect
    {
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if(USBD_IS_ATTACHED())					// USB Plug In
        {
            USBD_ENABLE_USB();
        }
        else									// USB Un-plug
        {
            USBD_DISABLE_USB();
        }
    }
	
    if(u32IntSts & USBD_INTSTS_BUS)
    {
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if(u32State & USBD_STATE_USBRST)		// Bus reset
        {
            USBD_ENABLE_USB();
            USBD_SwReset();
        }
        if(u32State & USBD_STATE_SUSPEND)		// Suspend
        {
            USBD_DISABLE_PHY();
        }
        if(u32State & USBD_STATE_RESUME)		// Resume
        {
            USBD_ENABLE_USB();
        }
    }
	
    if(u32IntSts & USBD_INTSTS_WAKEUP)			// Wakeup
    {
        USBD_CLR_INT_FLAG(USBD_INTSTS_WAKEUP);
    }
	
    if(u32IntSts & USBD_INTSTS_USB)
    {
        if(u32IntSts & USBD_INTSTS_SETUP)
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

            /* Clear the data IN/OUT ready flag of control end-points */
            USBD_STOP_TRANSACTION(EP0);
            USBD_STOP_TRANSACTION(EP1);

            USBD_ProcessSetupPacket();
        }

        if(u32IntSts & USBD_INTSTS_EP0)			// Control IN
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
            
            USBD_CtrlIn();
        }

        if(u32IntSts & USBD_INTSTS_EP1)			// Control OUT
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);

            USBD_CtrlOut();
			
			// In ACK of SET_LINE_CODE
			extern uint8_t g_usbd_SetupPacket[];
            if(g_usbd_SetupPacket[1] == SET_LINE_CODE)
            {
				switch(g_usbd_SetupPacket[4])	// Interface number
                {
				case 1:
                    VCOM_LineCoding(UART2, &LineCfg, &Vcom);
					break;
				
				case 3:
					VCOM_LineCoding(UART1, &LineCfg2, &Vcom2);
					break;
				}
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
		
		if(u32IntSts & USBD_INTSTS_EP7)
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
        }

        if(u32IntSts & USBD_INTSTS_EP8)			// Bulk IN
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP8);
			
            EP8_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP9)			// Bulk OUT
        {
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP9);
			
            EP9_Handler();
        }
    }
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
			USBD_SetStall(0);
			break;
		
		case GET_LINE_CODE:
			switch(buf[4])	// Interface number
            {
			case 1:
                USBD_PrepareCtrlIn((uint8_t *)&LineCfg, 7);
				break;
			
			case 3:
				USBD_PrepareCtrlIn((uint8_t *)&LineCfg2, 7);
				break;
			}

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
            USBD_SetStall(0);
            break;
		
		case SET_LINE_CODE:
            switch(buf[4])	// Interface number
			{
			case 1:
                USBD_PrepareCtrlOut((uint8_t *)&LineCfg, 7);
				break;
			
			case 3:
				USBD_PrepareCtrlOut((uint8_t *)&LineCfg2, 7);
				break;
			}
            
            /* Status stage */
            USBD_SET_DATA1(EP0);
            USBD_SET_PAYLOAD_LEN(EP0, 0);
            break;
		
		case SET_CONTROL_LINE:
			switch(buf[4])  // Interface number
			{
			case 1:
				Vcom.hw_flow = (buf[3] << 8) | buf[2];
				break;

			case 3:
				Vcom2.hw_flow = (buf[3] << 8) | buf[2];
				break;
			}
			
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


extern uint8_t MS_OS_20_DescriptorSet[];
void WINUSB_VendorRequest(void)
{
	uint16_t len;
	uint8_t buf[8];

    USBD_GetSetupPacket(buf);
	
    if(buf[0] & 0x80) 	// Device to host
    {
        switch(buf[1])
        {
		case WINUSB_VENDOR_CODE:
			switch(buf[4])
			{
			case 7:
				len = USBD_Minimum(buf[6] | (buf[7] << 8), MS_OS_20_DescriptorSet[8] | (MS_OS_20_DescriptorSet[9] << 8));
				USBD_PrepareCtrlIn(MS_OS_20_DescriptorSet, len);
				
				/* Status stage */
				USBD_PrepareCtrlOut(0,0);
				return;
			}
        }
    }
    else				// Host to device
    {
    }
	
	USBD_SetStall(0);
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
static uint16_t USB_ResponseSize[DAP_PACKET_COUNT];				  // number of bytes in response


uint8_t usbd_hid_process(void)
{
	uint32_t n;

	if((USB_RequestOut != USB_RequestIn) || USB_RequestFlag)
	{
		USB_ResponseSize[USB_ResponseIn] = DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);

		// Update request index and flag
		n = USB_RequestOut + 1;
		if(n == DAP_PACKET_COUNT)
			n = 0;
		USB_RequestOut = n;

		if(USB_RequestOut == USB_RequestIn)
			USB_RequestFlag = 0;

		if(USB_ResponseIdle)
		{
			USB_ResponseIdle = 0;
			
#ifdef DAP_FW_V1
			USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2)), USB_Response[USB_ResponseIn], DAP_PACKET_SIZE);
			USBD_SET_PAYLOAD_LEN(EP2, DAP_PACKET_SIZE);
#else
			USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2)), USB_Response[USB_ResponseIn], USB_ResponseSize[USB_ResponseIn]);
			USBD_SET_PAYLOAD_LEN(EP2, USB_ResponseSize[USB_ResponseIn]);
#endif
		}
		else
		{
			n = USB_ResponseIn + 1;
			if(n == DAP_PACKET_COUNT)
				n = 0;
			USB_ResponseIn = n;

			if(USB_ResponseIn == USB_ResponseOut)
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
	USBD_MemCopy(USB_Request[USB_RequestIn], EpBuf, len);

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
#ifdef DAP_FW_V1
		USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2)), USB_Response[USB_ResponseOut], DAP_PACKET_SIZE);
        USBD_SET_PAYLOAD_LEN(EP2, DAP_PACKET_SIZE);
#else
		USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2)), USB_Response[USB_ResponseOut], USB_ResponseSize[USB_ResponseOut]);
        USBD_SET_PAYLOAD_LEN(EP2, USB_ResponseSize[USB_ResponseOut]);
#endif
		
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
