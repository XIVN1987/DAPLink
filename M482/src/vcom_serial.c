#include "NuMicro.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


void VCOM_Init(void)
{
	GPIO_SetPullCtl(PB, BIT0, GPIO_PUSEL_PULL_UP);
	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk       | SYS_GPB_MFPL_PB1MFP_Msk);
    SYS->GPB_MFPL |=  (SYS_GPB_MFPL_PB0MFP_UART2_RXD | SYS_GPB_MFPL_PB1MFP_UART2_TXD);
	
	CLK_SetModuleClock(UART2_MODULE, CLK_CLKSEL3_UART2SEL_HXT, CLK_CLKDIV4_UART2(1));
	CLK_EnableModuleClock(UART2_MODULE);
	
	UART_Open(UART2, 115200);
	
	UART2->FIFO = (UART2->FIFO & (~UART_FIFO_RFITL_Msk)) | UART_FIFO_RFITL_8BYTES;
	UART_SetTimeoutCnt(UART2, 50);
	
	UART_ENABLE_INT(UART2, (UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk));

    NVIC_EnableIRQ(UART2_IRQn);

#if VCOM_COUNT > 1
	GPIO_SetPullCtl(PB, BIT2, GPIO_PUSEL_PULL_UP);
	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB2MFP_Msk       | SYS_GPB_MFPL_PB3MFP_Msk);
    SYS->GPB_MFPL |=  (SYS_GPB_MFPL_PB2MFP_UART1_RXD | SYS_GPB_MFPL_PB3MFP_UART1_TXD);
	
	CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UART1SEL_HXT, CLK_CLKDIV0_UART1(1));
	CLK_EnableModuleClock(UART1_MODULE);
	
	UART_Open(UART1, 115200);
	
	UART1->FIFO = (UART1->FIFO & (~UART_FIFO_RFITL_Msk)) | UART_FIFO_RFITL_8BYTES;
	UART_SetTimeoutCnt(UART1, 50);
	
	UART_ENABLE_INT(UART1, (UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk));

    NVIC_EnableIRQ(UART1_IRQn);
#endif
}


volatile VCOM Vcom, Vcom2;

void EP5_Handler(void)	// Bulk IN
{
    Vcom.in_bytes = 0;
}

void EP6_Handler(void)	// Bulk OUT
{
    Vcom.out_bytes = USBD_GET_PAYLOAD_LEN(EP6);
    Vcom.out_ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP6));

    Vcom.out_ready = 1;
}

void EP8_Handler(void)	// Bulk IN
{
    Vcom2.in_bytes = 0;
}

void EP9_Handler(void)	// Bulk OUT
{
    Vcom2.out_bytes = USBD_GET_PAYLOAD_LEN(EP9);
    Vcom2.out_ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP9));

    Vcom2.out_ready = 1;
}


VCOM_LINE_CODING LineCfg = {115200, 0, 0, 8};   // Baud rate, stop bits, parity bits, data bits
VCOM_LINE_CODING LineCfg2 = {115200, 0, 0, 8};

void VCOM_LineCoding(UART_T * UARTx, VCOM_LINE_CODING * LineCfgx, volatile VCOM * Vcomx)
{
   	uint32_t data_len, parity, stop_len;
	
	switch(LineCfgx->u8DataBits)
	{
	case 5:  data_len = UART_WORD_LEN_5; break;
	case 6:  data_len = UART_WORD_LEN_6; break;
	case 7:  data_len = UART_WORD_LEN_7; break;
	case 8:  data_len = UART_WORD_LEN_8; break;
	default: data_len = UART_WORD_LEN_8; break;
	}
	
	switch(LineCfgx->u8ParityType)
	{
	case 0:  parity = UART_PARITY_NONE;  break;
	case 1:  parity = UART_PARITY_ODD;   break;
	case 2:  parity = UART_PARITY_EVEN;  break;
	case 3:  parity = UART_PARITY_MARK;  break;
	case 4:  parity = UART_PARITY_SPACE; break;
	default: parity = UART_PARITY_NONE;  break;
	}
	
	switch(LineCfgx->u8CharFormat)
	{
	case 0:  stop_len = UART_STOP_BIT_1;   break;
	case 1:  stop_len = UART_STOP_BIT_1_5; break;
	case 2:  stop_len = UART_STOP_BIT_2;   break;
	default: stop_len = UART_STOP_BIT_1;   break;
	}
	
	__disable_irq();
	
	// Reset software FIFO
	Vcomx->rx_bytes = 0;
	Vcomx->rx_head = 0;
	Vcomx->rx_tail = 0;

	Vcomx->tx_bytes = 0;
	Vcomx->tx_head = 0;
	Vcomx->tx_tail = 0;

	// Reset hardware FIFO
	UARTx->FIFO |= (UART_FIFO_RXRST_Msk | UART_FIFO_TXRST_Msk);
	
	UART_SetLineConfig(UARTx, LineCfgx->u32DTERate, data_len, parity, stop_len);
	
	__enable_irq();
}


void UARTX_IRQHandler(UART_T * UARTx, volatile VCOM * Vcomx)
{
    uint8_t chr;
    int32_t size;

    if((UARTx->INTSTS & UART_INTSTS_RDAIF_Msk) || (UARTx->INTSTS & UART_INTSTS_RXTOIF_Msk))
    {
        while((UARTx->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) == 0)
        {
            chr = UARTx->DAT;

            if(Vcomx->rx_bytes < RX_BUFF_SIZE)  // Check if buffer full
            {
                Vcomx->rx_buff[Vcomx->rx_tail++] = chr;
                if(Vcomx->rx_tail >= RX_BUFF_SIZE)
                    Vcomx->rx_tail = 0;
                Vcomx->rx_bytes++;
            }
        }
    }

    if(UARTx->INTSTS & UART_INTSTS_THREIF_Msk)
    {
        if(Vcomx->tx_bytes)
        {
            /* Fill the Tx FIFO */
            size = Vcomx->tx_bytes;
            if(size >= TX_FIFO_SIZE)
            {
                size = TX_FIFO_SIZE;
            }

            while(size)
            {
                UARTx->DAT = Vcomx->tx_buff[Vcomx->tx_head++];
                if(Vcomx->tx_head >= TX_BUFF_SIZE)
                    Vcomx->tx_head = 0;
                Vcomx->tx_bytes--;
                size--;
            }
        }
        else
        {
            /* No more data, just stop Tx (Stop work) */
            UARTx->INTEN &= ~UART_INTEN_THREIEN_Msk;
        }
    }
}


void UART2_IRQHandler(void)
{
	UARTX_IRQHandler(UART2, &Vcom);
}


void UART1_IRQHandler(void)
{
	UARTX_IRQHandler(UART1, &Vcom2);
}



void VCOMX_TransferData(volatile VCOM * Vcomx, UART_T * UARTx, uint32_t InEP, uint32_t InEP_MAX_PKT_SIZE, uint32_t OutEP, uint32_t OutEP_MAX_PKT_SIZE)
{
    int32_t i, len;

    /* Check whether USB is ready for next packet or not */
    if(Vcomx->in_bytes == 0)
    {
        /* Check whether we have new COM Rx data to send to USB or not */
        if(Vcomx->rx_bytes)
        {
            len = Vcomx->rx_bytes;
            if(len > InEP_MAX_PKT_SIZE)
                len = InEP_MAX_PKT_SIZE;

            for(i = 0; i < len; i++)
            {
                Vcomx->in_buff[i] = Vcomx->rx_buff[Vcomx->rx_head++];
                if(Vcomx->rx_head >= RX_BUFF_SIZE)
                    Vcomx->rx_head = 0;
            }

            __disable_irq();
            Vcomx->rx_bytes -= len;
            __enable_irq();

            Vcomx->in_bytes = len;
            USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(InEP)), (uint8_t *)Vcomx->in_buff, len);
            USBD_SET_PAYLOAD_LEN(InEP, len);
        }
        else
        {
            /* Prepare a zero packet if previous packet size is InEP_MAX_PKT_SIZE and
               no more data to send at this moment to note Host the transfer has been done */
            len = USBD_GET_PAYLOAD_LEN(InEP);
            if(len == InEP_MAX_PKT_SIZE)
                USBD_SET_PAYLOAD_LEN(InEP, 0);
        }
    }

    /* Process the Bulk out data when bulk out data is ready. */
    if(Vcomx->out_ready && (Vcomx->out_bytes <= TX_BUFF_SIZE - Vcomx->tx_bytes))
    {
        for(i = 0; i < Vcomx->out_bytes; i++)
        {
            Vcomx->tx_buff[Vcomx->tx_tail++] = Vcomx->out_ptr[i];
            if(Vcomx->tx_tail >= TX_BUFF_SIZE)
                Vcomx->tx_tail = 0;
        }

        __disable_irq();
        Vcomx->tx_bytes += Vcomx->out_bytes;
        __enable_irq();

        Vcomx->out_bytes = 0;
        Vcomx->out_ready = 0; /* Clear bulk out ready flag */

        /* Ready to get next BULK out */
        USBD_SET_PAYLOAD_LEN(OutEP, OutEP_MAX_PKT_SIZE);
    }

    /* Process the software Tx FIFO */
    if(Vcomx->tx_bytes)
    {
        /* Check if Tx is working */
        if((UARTx->INTEN & UART_INTEN_THREIEN_Msk) == 0)
        {
            /* Send one bytes out */
            UARTx->DAT = Vcomx->tx_buff[Vcomx->tx_head++];
            if(Vcomx->tx_head >= TX_BUFF_SIZE)
                Vcomx->tx_head = 0;

            __disable_irq();
            Vcomx->tx_bytes--;
            __enable_irq();

            /* Enable Tx Empty Interrupt. (Trigger first one) */
            UARTx->INTEN |= UART_INTEN_THREIEN_Msk;
        }
    }
}


void VCOM_TransferData(void)
{
    VCOMX_TransferData(&Vcom,  UART2, EP5, EP5_MAX_PKT_SIZE, EP6, EP6_MAX_PKT_SIZE);
    VCOMX_TransferData(&Vcom2, UART1, EP8, EP8_MAX_PKT_SIZE, EP9, EP9_MAX_PKT_SIZE);
}
