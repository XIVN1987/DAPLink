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
	UART_ENABLE_INT(UART2, (UART_INTEN_RDAIEN_Msk | UART_INTEN_THREIEN_Msk | UART_INTEN_RXTOIEN_Msk));

    NVIC_EnableIRQ(UART2_IRQn);
}


volatile VCOM Vcom;

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


VCOM_LINE_CODING LineCfg = {115200, 0, 0, 8};   // Baud rate, stop bits, parity bits, data bits

void VCOM_LineCoding(void)
{
    uint32_t data_len, parity, stop_len;
	
	switch(LineCfg.u8DataBits)
	{
	case 5:  data_len = UART_WORD_LEN_5; break;
	case 6:  data_len = UART_WORD_LEN_6; break;
	case 7:  data_len = UART_WORD_LEN_7; break;
	case 8:  data_len = UART_WORD_LEN_8; break;
	default: data_len = UART_WORD_LEN_8; break;
	}
	
	switch(LineCfg.u8ParityType)
	{
	case 0:  parity = UART_PARITY_NONE;  break;
	case 1:  parity = UART_PARITY_ODD;   break;
	case 2:  parity = UART_PARITY_EVEN;  break;
	case 3:  parity = UART_PARITY_MARK;  break;
	case 4:  parity = UART_PARITY_SPACE; break;
	default: parity = UART_PARITY_NONE;  break;
	}
	
	switch(LineCfg.u8CharFormat)
	{
	case 0:  stop_len = UART_STOP_BIT_1;   break;
	case 1:  stop_len = UART_STOP_BIT_1_5; break;
	case 2:  stop_len = UART_STOP_BIT_2;   break;
	default: stop_len = UART_STOP_BIT_1;   break;
	}
	
	NVIC_DisableIRQ(UART2_IRQn);
	
	// Reset software FIFO
	Vcom.rx_bytes = 0;
	Vcom.rx_head = 0;
	Vcom.rx_tail = 0;

	Vcom.tx_bytes = 0;
	Vcom.tx_head = 0;
	Vcom.tx_tail = 0;

	// Reset hardware FIFO
	UART2->FIFO = 0x3;
	
	UART_SetLineConfig(UART2, LineCfg.u32DTERate, data_len, parity, stop_len);
	
	NVIC_EnableIRQ(UART2_IRQn);
}


void UART2_IRQHandler(void)
{
    uint8_t chr;
    int32_t size;

    if((UART2->INTSTS & UART_INTSTS_RDAIF_Msk) || (UART2->INTSTS & UART_INTSTS_RXTOIF_Msk))
    {
        while((UART2->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) == 0)
        {
            chr = UART2->DAT;

            if(Vcom.rx_bytes < RX_BUFF_SIZE)  // Check if buffer full
            {
                Vcom.rx_buff[Vcom.rx_tail++] = chr;
                if(Vcom.rx_tail >= RX_BUFF_SIZE)
                    Vcom.rx_tail = 0;
                Vcom.rx_bytes++;
            }
        }
    }

    if(UART2->INTSTS & UART_INTSTS_THREIF_Msk)
    {
        if(Vcom.tx_bytes)
        {
            /* Fill the Tx FIFO */
            size = Vcom.tx_bytes;
            if(size >= TX_FIFO_SIZE)
            {
                size = TX_FIFO_SIZE;
            }

            while(size)
            {
                UART2->DAT = Vcom.tx_buff[Vcom.tx_head++];
                if(Vcom.tx_head >= TX_BUFF_SIZE)
                    Vcom.tx_head = 0;
                Vcom.tx_bytes--;
                size--;
            }
        }
        else
        {
            /* No more data, just stop Tx (Stop work) */
            UART2->INTEN &= ~UART_INTEN_THREIEN_Msk;
        }
    }
}


void VCOM_TransferData(void)
{
    int32_t i, len;

    /* Check whether USB is ready for next packet or not */
    if(Vcom.in_bytes == 0)
    {
        /* Check whether we have new COM Rx data to send to USB or not */
        if(Vcom.rx_bytes)
        {
            len = Vcom.rx_bytes;
            if(len > EP5_MAX_PKT_SIZE)
                len = EP5_MAX_PKT_SIZE;

            for(i = 0; i < len; i++)
            {
                Vcom.in_buff[i] = Vcom.rx_buff[Vcom.rx_head++];
                if(Vcom.rx_head >= RX_BUFF_SIZE)
                    Vcom.rx_head = 0;
            }

            __disable_irq();
            Vcom.rx_bytes -= len;
            __enable_irq();

            Vcom.in_bytes = len;
            USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP5)), (uint8_t *)Vcom.in_buff, len);
            USBD_SET_PAYLOAD_LEN(EP5, len);
        }
        else
        {
            /* Prepare a zero packet if previous packet size is EP5_MAX_PKT_SIZE and
               no more data to send at this moment to note Host the transfer has been done */
            len = USBD_GET_PAYLOAD_LEN(EP5);
            if(len == EP5_MAX_PKT_SIZE)
                USBD_SET_PAYLOAD_LEN(EP5, 0);
        }
    }

    /* Process the Bulk out data when bulk out data is ready. */
    if(Vcom.out_ready && (Vcom.out_bytes <= TX_BUFF_SIZE - Vcom.tx_bytes))
    {
        for(i = 0; i < Vcom.out_bytes; i++)
        {
            Vcom.tx_buff[Vcom.tx_tail++] = Vcom.out_ptr[i];
            if(Vcom.tx_tail >= TX_BUFF_SIZE)
                Vcom.tx_tail = 0;
        }

        __disable_irq();
        Vcom.tx_bytes += Vcom.out_bytes;
        __enable_irq();

        Vcom.out_bytes = 0;
        Vcom.out_ready = 0;

        /* Ready to get next BULK out */
        USBD_SET_PAYLOAD_LEN(EP6, EP6_MAX_PKT_SIZE);
    }

    /* Process the software Tx FIFO */
    if(Vcom.tx_bytes)
    {
        /* Check if Tx is working */
        if((UART2->INTEN & UART_INTEN_THREIEN_Msk) == 0)
        {
            /* Send one bytes out */
            UART2->DAT = Vcom.tx_buff[Vcom.tx_head++];
            if(Vcom.tx_head >= TX_BUFF_SIZE)
                Vcom.tx_head = 0;

            __disable_irq();
            Vcom.tx_bytes--;
            __enable_irq();

            /* Enable Tx Empty Interrupt. (Trigger first one) */
            UART2->INTEN |= UART_INTEN_THREIEN_Msk;
        }
    }
}
