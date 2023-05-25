#include <string.h>
#include "ch32v30x.h"
#include "ch32v30x_usb.h"
#include "ch32v30x_usbhs_device.h"

#include "usb_desc.h"
#include "vcom_serial.h"


volatile VCOM Vcom;

VCOM_LINE_CODING LineCfg = {115200, 0, 0, 8};   // Baud rate, stop bits, parity bits, data bits


void VCOM_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);              // PA2 => USART2_TX

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);              // PA3 => USART2_RX

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &USART_InitStructure);

    USART_Cmd(USART2, ENABLE);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	NVIC_EnableIRQ(USART2_IRQn);
}


void VCOM_LineCoding(VCOM_LINE_CODING * LineCfgx)
{
    USART_InitTypeDef USART_InitStructure;
    
    switch(LineCfgx->u8DataBits)
    {
    case 8:  USART_InitStructure.USART_WordLength = USART_WordLength_8b; break;
    default: USART_InitStructure.USART_WordLength = USART_WordLength_8b; break;
    }
    
    switch(LineCfgx->u8ParityType)
    {
    case 0:  USART_InitStructure.USART_Parity     = USART_Parity_No;     break;
    case 1:  USART_InitStructure.USART_Parity     = USART_Parity_Odd;
             USART_InitStructure.USART_WordLength = USART_WordLength_9b; break;
    case 2:  USART_InitStructure.USART_Parity     = USART_Parity_Even;
             USART_InitStructure.USART_WordLength = USART_WordLength_9b; break;
    default: USART_InitStructure.USART_Parity     = USART_Parity_No;     break;
    }

    switch(LineCfgx->u8CharFormat)
    {
    case 0:  USART_InitStructure.USART_StopBits = USART_StopBits_1;   break;
    case 1:  USART_InitStructure.USART_StopBits = USART_StopBits_1_5; break;
    case 2:  USART_InitStructure.USART_StopBits = USART_StopBits_2;   break;
    default: USART_InitStructure.USART_StopBits = USART_StopBits_1;   break;
    }

    USART_InitStructure.USART_BaudRate = LineCfgx->u32DTERate;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;


    __disable_irq();
    
    // Reset software FIFO
    Vcom.rx_bytes = 0;
	Vcom.rx_wrptr = 0;
    Vcom.rx_rdptr = 0;

    Vcom.tx_bytes = 0;
	Vcom.tx_wrptr = 0;
    Vcom.tx_rdptr = 0;
    
    USART_Init(USART2, &USART_InitStructure);
    
    __enable_irq();
}


void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		uint16_t chr = USART_ReceiveData(USART2);
		
		if(Vcom.rx_bytes < RX_BUFF_SIZE)
		{
			Vcom.rx_buff[Vcom.rx_wrptr++] = chr;
			if(Vcom.rx_wrptr == RX_BUFF_SIZE)
				Vcom.rx_wrptr = 0;
			
			Vcom.rx_bytes++;
		}
	}
	
	if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
	{
		if(Vcom.tx_bytes)
        {
		    USART_SendData(USART2, Vcom.tx_buff[Vcom.tx_rdptr++]);
			if(Vcom.tx_rdptr == TX_BUFF_SIZE)
				Vcom.tx_rdptr = 0;
			
			Vcom.tx_bytes--;
        }
        else
        {
            /* No more data, just stop Tx (Stop work) */
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
        }
	}
}


void VCOM_TransferData(void)
{
    if(Vcom.in_ready)		// 可以向主机发送数据
    {
        if(Vcom.rx_bytes)	// 有新的数据可以发送
        {
            Vcom.in_bytes = Vcom.rx_bytes;
            if(Vcom.in_bytes > CDC_BULK_IN_SZ_HS)
                Vcom.in_bytes = CDC_BULK_IN_SZ_HS;

            for(int i = 0; i < Vcom.in_bytes; i++)
            {
                Vcom.in_buff[i] = Vcom.rx_buff[Vcom.rx_rdptr++];
                if(Vcom.rx_rdptr >= RX_BUFF_SIZE)
                    Vcom.rx_rdptr = 0;
            }

            __disable_irq();
            Vcom.rx_bytes -= Vcom.in_bytes;
            __enable_irq();

            Vcom.in_ready = 0;

            memcpy(USBHS_EP3_Tx_Buf, (uint8_t *)Vcom.in_buff, Vcom.in_bytes);
            USBHSD->UEP3_TX_LEN = Vcom.in_bytes;
            USBHSD->UEP3_TX_CTRL = (USBHSD->UEP3_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_ACK;
        }
        else
        {
            /* Prepare a zero packet if previous packet size is CDC_BULK_IN_SZ and
               no more data to send at this moment to note Host the transfer has been done */
            if(Vcom.in_bytes == CDC_BULK_IN_SZ_HS)
			{
				Vcom.in_bytes = 0;
				
				USBHSD->UEP3_TX_LEN = 0;
				USBHSD->UEP3_TX_CTRL = (USBHSD->UEP3_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_ACK;
			}
        }
    }

	/* 从主机接收到数据，且 tx_buff 能够装下它们 */
    if(Vcom.out_ready && (Vcom.out_bytes <= TX_BUFF_SIZE - Vcom.tx_bytes))
    {
        for(int i = 0; i < Vcom.out_bytes; i++)
        {
            Vcom.tx_buff[Vcom.tx_wrptr++] = Vcom.out_buff[i];
            if(Vcom.tx_wrptr >= TX_BUFF_SIZE)
                Vcom.tx_wrptr = 0;
        }

        __disable_irq();
        Vcom.tx_bytes += Vcom.out_bytes;
        __enable_irq();

        Vcom.out_ready = 0;

        /* Ready for next BULK OUT */
        USBHSD->UEP3_RX_CTRL = (USBHSD->UEP3_RX_CTRL & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;
    }

    if(Vcom.tx_bytes && ((USART2->CTLR1 & USART_CTLR1_TXEIE) == 0))
    {
        USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
    }
}
