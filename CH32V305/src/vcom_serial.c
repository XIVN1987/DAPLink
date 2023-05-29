#include <string.h>
#include "ch32v30x.h"
#include "ch32v30x_usb.h"
#include "ch32v30x_usbhs_device.h"

#include "usb_desc.h"
#include "vcom_serial.h"


volatile VCOM Vcom;

VCOM_LINE_CODING LineCfg = {115200, 0, 0, 8};   // Baud rate, stop bits, parity bits, data bits

volatile uint32_t RX_Timeout = (1000.0 / 115200) * (512 * 10) * 1.5;


#define BUF_SZ  1024
uint8_t TXBuffer[BUF_SZ] __attribute__((aligned(4)));
uint8_t RXBuffer[BUF_SZ] __attribute__((aligned(4)));


void VCOM_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef   DMA_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);


    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);              // PA2 => USART2_TX

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);              // PA3 => USART2_RX


    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DATAR;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)TXBuffer;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel7, ENABLE);

    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RXBuffer;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel6, ENABLE);


    USART_InitStructure.USART_BaudRate = LineCfg.u32DTERate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &USART_InitStructure);

    USART_DMACmd(USART2, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);

    USART_Cmd(USART2, ENABLE);
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
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
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
    
    RX_Timeout = (1000.0 / USART_InitStructure.USART_BaudRate) * (512 * 10) * 1.5;

    __enable_irq();
}


extern volatile uint32_t SysTick_ms;
void VCOM_TransferData(void)
{
    uint8_t *rxdata = RXBuffer;

    if(Vcom.in_ready)		// 可以向主机发送数据
    {
        if((DMA1->INTFR & (DMA1_FLAG_HT6 | DMA1_FLAG_TC6)) || (SysTick_ms > RX_Timeout))
        {
            if(DMA1->INTFR & DMA1_FLAG_HT6)
            {
                rxdata = &RXBuffer[0];

                Vcom.in_bytes = BUF_SZ/2;

                DMA1->INTFCR = DMA1_FLAG_HT6;
            }
            else if(DMA1->INTFR & DMA1_FLAG_TC6)
            {
                rxdata = &RXBuffer[BUF_SZ/2];

                Vcom.in_bytes = BUF_SZ/2;

                DMA1->INTFCR = DMA1_FLAG_TC6;

                SysTick_ms = 0;
            }
            else
            {
                SysTick_ms = 0;

                uint32_t n_xfer = BUF_SZ - DMA_GetCurrDataCounter(DMA1_Channel6);
                if(n_xfer == 0)
                {
                    goto xfer_out;
                }
                if(n_xfer < BUF_SZ/2)
                {
                    rxdata = &RXBuffer[0];

                    Vcom.in_bytes = n_xfer;
                }
                else
                {
                    rxdata = &RXBuffer[BUF_SZ/2];

                    Vcom.in_bytes = n_xfer - BUF_SZ/2;
                }

                DMA_Cmd(DMA1_Channel6, DISABLE);
                DMA_SetCurrDataCounter(DMA1_Channel6, BUF_SZ);
                DMA_Cmd(DMA1_Channel6, ENABLE);
            }

            Vcom.in_ready = 0;

            USBHSD->UEP3_TX_DMA = (uint32_t)rxdata;
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

xfer_out:
	/* 从主机接收到数据，且 tx_buff 能够装下它们 */
    if(Vcom.out_ready && (DMA_GetCurrDataCounter(DMA1_Channel7) == 0))
    {
        Vcom.out_ready = 0;

        DMA_Cmd(DMA1_Channel7, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel7, Vcom.out_bytes);
        DMA_Cmd(DMA1_Channel7, ENABLE);

        /* Ready for next BULK OUT */
        USBHSD->UEP3_RX_CTRL = (USBHSD->UEP3_RX_CTRL & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;
    }
}
