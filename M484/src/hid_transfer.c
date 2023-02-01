#include "NuMicro.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


void HID_InitEP(int usbhs)
{
	/* EPA ==> HID Interrupt IN endpoint */
	HSUSBD_ConfigEp(EPA, HID_INT_IN_EP & 0x0F, HSUSBD_EP_CFG_TYPE_INT, HSUSBD_EP_CFG_DIR_IN);
    HSUSBD_SetEpBufAddr(EPA, HID_INT_IN_BUF_BA, HID_INT_IN_BUF_SZ);
    HSUSBD_SET_MAX_PAYLOAD(EPA, usbhs ? HID_INT_IN_SZ_HS : HID_INT_IN_SZ_FS);
	HSUSBD_ENABLE_EP_INT(EPA, HSUSBD_EPINTEN_TXPKIEN_Msk);
	
    /* EPB ==> HID Interrupt OUT endpoint */
	HSUSBD_ConfigEp(EPB, HID_INT_OUT_EP & 0x0F, HSUSBD_EP_CFG_TYPE_INT, HSUSBD_EP_CFG_DIR_OUT);
    HSUSBD_SetEpBufAddr(EPB, HID_INT_OUT_BUF_BA, HID_INT_OUT_BUF_SZ);
    HSUSBD_SET_MAX_PAYLOAD(EPB, usbhs ? HID_INT_OUT_SZ_HS : HID_INT_OUT_SZ_FS);
    HSUSBD_ENABLE_EP_INT(EPB, HSUSBD_EPINTEN_RXPKIEN_Msk);
	
	/* EPC ==> CDC Interrupt IN endpoint */
	HSUSBD_ConfigEp(EPC, CDC_INT_IN_EP & 0x0F, HSUSBD_EP_CFG_TYPE_INT, HSUSBD_EP_CFG_DIR_IN);
    HSUSBD_SetEpBufAddr(EPC, CDC_INT_IN_BUF_BA, CDC_INT_IN_BUF_SZ);
    HSUSBD_SET_MAX_PAYLOAD(EPC, usbhs ? CDC_INT_IN_SZ_HS : CDC_INT_IN_SZ_FS);
	
	/* EPD ==> CDC Bulk IN endpoint */
	HSUSBD_ConfigEp(EPD, CDC_BULK_IN_EP & 0x0F, HSUSBD_EP_CFG_TYPE_BULK, HSUSBD_EP_CFG_DIR_IN);
	HSUSBD->EP[EPD].EPRSPCTL = HSUSBD->EP[EPD].EPRSPCTL & (~HSUSBD_EP_RSPCTL_MODE_MASK) | HSUSBD_EP_RSPCTL_MODE_MANUAL;
    HSUSBD_SetEpBufAddr(EPD, CDC_BULK_IN_BUF_BA, CDC_BULK_IN_BUF_SZ);
    HSUSBD_SET_MAX_PAYLOAD(EPD, usbhs ? CDC_BULK_IN_SZ_HS : CDC_BULK_IN_SZ_FS);
	HSUSBD_ENABLE_EP_INT(EPD, HSUSBD_EPINTEN_TXPKIEN_Msk);

    /* EPE ==> CDC Bulk OUT endpoint */
	HSUSBD_ConfigEp(EPE, CDC_BULK_OUT_EP & 0x0F, HSUSBD_EP_CFG_TYPE_BULK, HSUSBD_EP_CFG_DIR_OUT);
    HSUSBD_SetEpBufAddr(EPE, CDC_BULK_OUT_BUF_BA, CDC_BULK_OUT_BUF_SZ);
    HSUSBD_SET_MAX_PAYLOAD(EPE, usbhs ? CDC_BULK_OUT_SZ_HS : CDC_BULK_OUT_SZ_FS);
    HSUSBD_ENABLE_EP_INT(EPE, HSUSBD_EPINTEN_RXPKIEN_Msk);
}


void HID_Init(void)
{
	HSUSBD_SET_ADDR(0);
	
    /* Enable USB BUS, Control Endpoint and Endpoint A-E global interrupt */
    HSUSBD_ENABLE_USB_INT(HSUSBD_GINTEN_USBIEN_Msk |
						  HSUSBD_GINTEN_CEPIEN_Msk |
						  HSUSBD_GINTEN_EPAIEN_Msk |
						  HSUSBD_GINTEN_EPBIEN_Msk |
						  HSUSBD_GINTEN_EPCIEN_Msk |
						  HSUSBD_GINTEN_EPDIEN_Msk |
						  HSUSBD_GINTEN_EPEIEN_Msk);
	
    /* Enable Plug-In/Un-Plug and Reset bus interrupt */
    HSUSBD_ENABLE_BUS_INT(HSUSBD_BUSINTEN_VBUSDETIEN_Msk |
						  HSUSBD_BUSINTEN_RSTIEN_Msk);
    
    /* Control endpoint */
    HSUSBD_SetEpBufAddr(CEP, EP0_BUF_BASE, EP0_BUF_SIZE);
    HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_SETUPPKIEN_Msk |
						  HSUSBD_CEPINTEN_STSDONEIEN_Msk);

    HID_InitEP(0);
}


void EPA_Handler(void)	// HID Interrupt IN
{
    HID_SetInReport();
}

void EPB_Handler(void)	// HID Interrupt OUT
{
	static uint8_t buff[1024];
	
    int len = HSUSBD->EP[EPB].EPDATCNT & 0xFFFF;
    for(int i = 0; i < len; i++)
        buff[i] = HSUSBD->EP[EPB].EPDAT_BYTE;
    HID_GetOutReport(buff, len);
}

void EPD_Handler(void)	// CDC Bulk IN
{
    Vcom.in_ready = 1;
}

void EPE_Handler(void)	// CDC Bulk OUT
{
    Vcom.out_bytes = HSUSBD->EP[EPE].EPDATCNT & 0xFFFF;
	for(int i = 0; i < Vcom.out_bytes; i++)
            Vcom.out_buff[i] = HSUSBD->EP[EPE].EPDAT_BYTE;

    Vcom.out_ready = 1;
}

void USBD20_IRQHandler(void)
{
    uint32_t usb_if, bus_if, ept_if;

    usb_if = HSUSBD->GINTSTS & HSUSBD->GINTEN;

    /* Bus Event */
    if(usb_if & HSUSBD_GINTSTS_USBIF_Msk)
    {
        bus_if = HSUSBD->BUSINTSTS & HSUSBD->BUSINTEN;

        if(bus_if & HSUSBD_BUSINTSTS_VBUSDETIF_Msk)
        {
            if(HSUSBD_IS_ATTACHED())    // USB Plug In
            {
                HSUSBD_ENABLE_USB();
            }
            else                        // USB Un-plug
            {
                HSUSBD_DISABLE_USB();
            }

            HSUSBD_CLR_BUS_INT_FLAG(HSUSBD_BUSINTSTS_VBUSDETIF_Msk);
        }

        if(bus_if & HSUSBD_BUSINTSTS_RSTIF_Msk)
        {
            HSUSBD_SwReset();
            HSUSBD_SET_ADDR(0);

            HID_InitEP(HSUSBD->OPER & HSUSBD_OPER_CURSPD_Msk);
            
            HSUSBD_CLR_CEP_INT_FLAG(0x1FFC);
            HSUSBD_CLR_BUS_INT_FLAG(HSUSBD_BUSINTSTS_RSTIF_Msk);
        }

        if(bus_if & HSUSBD_BUSINTSTS_SOFIF_Msk)
        {
            HSUSBD_CLR_BUS_INT_FLAG(HSUSBD_BUSINTSTS_SOFIF_Msk);
        }
    }

    /* Control Endpoint */
    if(usb_if & HSUSBD_GINTSTS_CEPIF_Msk)
    {
        ept_if = HSUSBD->CEPINTSTS & HSUSBD->CEPINTEN;

        if(ept_if & HSUSBD_CEPINTSTS_SETUPPKIF_Msk)
        {
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_SETUPPKIF_Msk);
            HSUSBD_ProcessSetupPacket();
            return;
        }

        if(ept_if & HSUSBD_CEPINTSTS_OUTTKIF_Msk)
        {
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_OUTTKIF_Msk);
            HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_STSDONEIEN_Msk);
            return;
        }

        if(ept_if & HSUSBD_CEPINTSTS_INTKIF_Msk)
        {
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_INTKIF_Msk);
            if(!(ept_if & HSUSBD_CEPINTSTS_STSDONEIF_Msk))
            {
                HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_TXPKIF_Msk);
                HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_TXPKIEN_Msk);
                HSUSBD_CtrlIn();
            }
            else
            {
                HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_TXPKIF_Msk);
                HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_TXPKIEN_Msk | HSUSBD_CEPINTEN_STSDONEIEN_Msk);
            }
            return;
        }

        if(ept_if & HSUSBD_CEPINTSTS_TXPKIF_Msk)
        {
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_STSDONEIF_Msk);
            HSUSBD_SET_CEP_STATE(HSUSBD_CEPCTL_NAKCLR);
            if(g_hsusbd_CtrlInSize)
            {
                HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_INTKIF_Msk);
                HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_INTKIEN_Msk);
            }
            else
            {
                if(g_hsusbd_CtrlZero == 1)
                    HSUSBD_SET_CEP_STATE(HSUSBD_CEPCTL_ZEROLEN);
                HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_STSDONEIF_Msk);
                HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_SETUPPKIEN_Msk|HSUSBD_CEPINTEN_STSDONEIEN_Msk);
            }
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_TXPKIF_Msk);
            return;
        }

        if(ept_if & HSUSBD_CEPINTSTS_RXPKIF_Msk)
        {
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_RXPKIF_Msk);
            HSUSBD_SET_CEP_STATE(HSUSBD_CEPCTL_NAKCLR);
            HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_SETUPPKIEN_Msk|HSUSBD_CEPINTEN_STSDONEIEN_Msk);
            return;
        }

        if(ept_if & HSUSBD_CEPINTSTS_STSDONEIF_Msk)
        {
            HSUSBD_UpdateDeviceState();
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_STSDONEIF_Msk);
            HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_SETUPPKIEN_Msk);
            return;
        }
    }

    /* HID Interrupt IN */
    if(usb_if & HSUSBD_GINTSTS_EPAIF_Msk)
    {
        ept_if = HSUSBD->EP[EPA].EPINTSTS & HSUSBD->EP[EPA].EPINTEN;

        if(ept_if & HSUSBD_EPINTSTS_TXPKIF_Msk)
            EPA_Handler();

        HSUSBD_CLR_EP_INT_FLAG(EPA, ept_if);
    }

    /* HID Interrupt OUT */
    if(usb_if & HSUSBD_GINTSTS_EPBIF_Msk)
    {
        ept_if = HSUSBD->EP[EPB].EPINTSTS & HSUSBD->EP[EPB].EPINTEN;

        if(ept_if & HSUSBD_EPINTSTS_RXPKIF_Msk)
            EPB_Handler();

        HSUSBD_CLR_EP_INT_FLAG(EPB, ept_if);
    }

    /* CDC Interrupt IN */
    if(usb_if & HSUSBD_GINTSTS_EPCIF_Msk)
    {
        ept_if = HSUSBD->EP[EPC].EPINTSTS & HSUSBD->EP[EPC].EPINTEN;

        HSUSBD_CLR_EP_INT_FLAG(EPC, ept_if);
    }

    /* CDC Bulk IN */
    if(usb_if & HSUSBD_GINTSTS_EPDIF_Msk)
    {
        ept_if = HSUSBD->EP[EPD].EPINTSTS & HSUSBD->EP[EPD].EPINTEN;
		
		if(ept_if & HSUSBD_EPINTSTS_TXPKIF_Msk)
            EPD_Handler();

        HSUSBD_CLR_EP_INT_FLAG(EPD, ept_if);
    }

    /* CDC Bulk OUT */
    if(usb_if & HSUSBD_GINTSTS_EPEIF_Msk)
    {
        ept_if = HSUSBD->EP[EPE].EPINTSTS & HSUSBD->EP[EPE].EPINTEN;
		
		if(ept_if & HSUSBD_EPINTSTS_RXPKIF_Msk)
            EPE_Handler();

        HSUSBD_CLR_EP_INT_FLAG(EPE, ept_if);
    }

    if(usb_if & HSUSBD_GINTSTS_EPFIF_Msk)
    {
        ept_if = HSUSBD->EP[EPF].EPINTSTS & HSUSBD->EP[EPF].EPINTEN;
        
        HSUSBD_CLR_EP_INT_FLAG(EPF, ept_if);
    }
}


void HID_ClassRequest(void)
{
    if(gUsbCmd.bmRequestType & 0x80)
    {
		/* Device to host */
        switch(gUsbCmd.bRequest)
        {
        case GET_IDLE:
		case GET_REPORT:
        case GET_PROTOCOL:
			HSUSBD_SET_CEP_STATE(HSUSBD_CEPCTL_STALLEN_Msk);
			break;
		
		case GET_LINE_CODE:
			HSUSBD_PrepareCtrlIn((uint8_t *)&LineCfg, 7);
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_INTKIF_Msk);
            HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_INTKIEN_Msk);
            break;
		
        default:
            /* Setup error, stall the device */
            HSUSBD_SET_CEP_STATE(HSUSBD_CEPCTL_STALLEN_Msk);
            break;
        }
    }
    else
    {
		/* Host to device */
        switch(gUsbCmd.bRequest)
        {
		case SET_IDLE:
		case SET_REPORT:
            /* Status stage */
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_STSDONEIF_Msk);
            HSUSBD_SET_CEP_STATE(HSUSBD_CEPCTL_NAKCLR);
            HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_STSDONEIEN_Msk);
            break;
		
		case SET_PROTOCOL:
            HSUSBD_SET_CEP_STATE(HSUSBD_CEPCTL_STALLEN_Msk);
            break;
		
		case SET_LINE_CODE:
            HSUSBD_CtrlOut((uint8_t *)&LineCfg, 7);
            
            /* Status stage */
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_STSDONEIF_Msk);
            HSUSBD_SET_CEP_STATE(HSUSBD_CEPCTL_NAKCLR);
            HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_STSDONEIEN_Msk);
            break;
		
		case SET_CONTROL_LINE:
			Vcom.hw_flow = gUsbCmd.wValue;
			
            /* Status stage */
            HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_STSDONEIF_Msk);
            HSUSBD_SET_CEP_STATE(HSUSBD_CEPCTL_NAKCLR);
            HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_STSDONEIEN_Msk);
            break;
		
        default:
            /* Setup error, stall the device */
            HSUSBD_SET_CEP_STATE(HSUSBD_CEPCTL_STALLEN_Msk);
            break;
        }
    }
}


extern uint8_t MS_OS_20_DescriptorSet[];
void WINUSB_VendorRequest(void)
{
	uint16_t len;
	
   if(gUsbCmd.bmRequestType & 0x80)
    {
		/* Device to host */
        switch(gUsbCmd.bRequest)
        {
		case WINUSB_VENDOR_CODE:
			switch(gUsbCmd.wIndex & 0xFF)
			{
			case 7:
				len = Minimum(gUsbCmd.wLength, MS_OS_20_DescriptorSet[8] | (MS_OS_20_DescriptorSet[9] << 8));
				HSUSBD_PrepareCtrlIn(MS_OS_20_DescriptorSet, len);
				
				/* Status stage */
				HSUSBD_CLR_CEP_INT_FLAG(HSUSBD_CEPINTSTS_INTKIF_Msk);
				HSUSBD_ENABLE_CEP_INT(HSUSBD_CEPINTEN_INTKIEN_Msk);
				return;
			}
        }
    }
    else
    {
		/* Host to device */
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
	uint32_t n, len;

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
			len = DAP_PACKET_SIZE;
#else
			len = USB_ResponseSize[USB_ResponseIn];
#endif
			for(int i = 0; i < len; i++)
				HSUSBD->EP[EPA].EPDAT_BYTE = USB_Response[USB_ResponseIn][i];
			HSUSBD->EP[EPA].EPTXCNT = len;
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


void HID_GetOutReport(uint8_t *buf, uint32_t len)
{
    if(buf[0] == ID_DAP_TransferAbort)
	{
		DAP_TransferAbort = 1;
		return;
	}
	
	if(USB_RequestFlag && (USB_RequestIn == USB_RequestOut))
		return;  // Discard packet when buffer is full

	// Store data into request packet buffer
	USBD_MemCopy(USB_Request[USB_RequestIn], buf, len);

	USB_RequestIn++;
	if(USB_RequestIn == DAP_PACKET_COUNT)
		USB_RequestIn = 0;
	if(USB_RequestIn == USB_RequestOut)
		USB_RequestFlag = 1;
}


void HID_SetInReport(void)
{
	uint32_t len;
	
	if((USB_ResponseOut != USB_ResponseIn) || USB_ResponseFlag)
	{
#ifdef DAP_FW_V1
		len = DAP_PACKET_SIZE;
#else
		len = USB_ResponseSize[USB_ResponseOut];
#endif
		for(int i = 0; i < len; i++)
			HSUSBD->EP[EPA].EPDAT_BYTE = USB_Response[USB_ResponseOut][i];
		HSUSBD->EP[EPA].EPTXCNT = len;
		
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
