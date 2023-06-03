#include <string.h>
#include "ch32v20x.h"
#include "usb_lib.h"
#include "usb_regs.h"
#include "usb_desc.h"
#include "vcom_serial.h"


volatile VCOM Vcom = {.in_ready = 1};

VCOM_LINE_CODING LineCfg = {115200, 0, 0, 8};   // Baud rate, stop bits, parity bits, data bits


#define RXDMA_SZ  (CDC_BULK_IN_SZ * 2)
uint8_t RXBuffer[RXDMA_SZ] __attribute__((aligned(4)));
uint8_t TXBuffer[CDC_BULK_OUT_SZ] __attribute__((aligned(4)));


void VCOM_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef   DMA_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
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
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel7, ENABLE);

    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DATAR;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RXBuffer;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_BufferSize = RXDMA_SZ;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel6, ENABLE);


    USART_InitStructure.USART_BaudRate = 115200;
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
    
    USART_Init(USART2, &USART_InitStructure);
    
    __enable_irq();
}


extern volatile uint32_t SysTick_ms;

void VCOM_TransferData(void)
{
    static uint32_t last_ms = 0;
    static uint32_t last_pos = 0;

    if(Vcom.in_ready)		// 可以向主机发送数据
    {
        uint32_t pos = RXDMA_SZ - DMA_GetCurrDataCounter(DMA1_Channel6);
        if((pos - last_pos >= CDC_BULK_IN_SZ) || ((pos != last_pos) && (SysTick_ms != last_ms)))
        {
            if(pos < last_pos)
                pos = RXDMA_SZ;

            if(pos - last_pos > CDC_BULK_IN_SZ)
                pos = last_pos + CDC_BULK_IN_SZ;

            Vcom.in_bytes = pos - last_pos;

            Vcom.in_ready = 0;

            USB_SIL_Write(EP3_IN, &RXBuffer[last_pos], Vcom.in_bytes);
            SetEPTxValid(ENDP3);

            last_pos = pos % RXDMA_SZ;

            last_ms = SysTick_ms;
        }
        else
        {
            /* Prepare a zero packet if previous packet size is CDC_BULK_IN_SZ and
               no more data to send at this moment to note Host the transfer has been done */
            if(Vcom.in_bytes == CDC_BULK_IN_SZ)
			{
				Vcom.in_bytes = 0;

				USB_SIL_Write(EP3_IN, (uint8_t *)0, 0);
				SetEPTxValid(ENDP3);
			}
        }
    }

	/* 从主机接收到数据，且前面的数据 DMA 已发送完 */
    if(Vcom.out_ready && (DMA_GetCurrDataCounter(DMA1_Channel7) == 0))
    {
        Vcom.out_ready = 0;

        memcpy(TXBuffer, (uint8_t *)Vcom.out_buff, Vcom.out_bytes);

        DMA_Cmd(DMA1_Channel7, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel7, Vcom.out_bytes);
        DMA_Cmd(DMA1_Channel7, ENABLE);

        /* Ready for next BULK OUT */
		SetEPRxValid(ENDP3);
    }
}
