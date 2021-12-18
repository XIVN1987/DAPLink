#include <string.h>
#include "NuMicro.h"
#include "vcom_serial.h"


void EP5_Handler(void)	// Bulk IN
{
    vcom.in_bytes = 0;
}

void EP6_Handler(void)	// Bulk OUT
{
    vcom.out_bytes = USBD_GET_PAYLOAD_LEN(EP6);
    vcom.out_ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP6));

    vcom.out_ready = 1;
}


void VCOM_Init(void)
{
    /*****************************************************/
	 /* EP4 ==> Interrupt IN endpoint, address 3 */
    USBD_CONFIG_EP(EP4, USBD_CFG_EPMODE_IN | CDC_INT_IN_EP);
    /* Buffer offset for EP4 ->  */
    USBD_SET_EP_BUF_ADDR(EP4, EP4_BUF_BASE);
	
    /* EP5 ==> Bulk IN endpoint, address 4 */
    USBD_CONFIG_EP(EP5, USBD_CFG_EPMODE_IN | CDC_BULK_IN_EP);
    /* Buffer offset for EP5 */
    USBD_SET_EP_BUF_ADDR(EP5, EP5_BUF_BASE);

    /* EP6 ==> Bulk Out endpoint, address 5 */
    USBD_CONFIG_EP(EP6, USBD_CFG_EPMODE_OUT | CDC_BULK_OUT_EP);
    /* Buffer offset for EP6 */
    USBD_SET_EP_BUF_ADDR(EP6, EP6_BUF_BASE);
    /* trigger receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP6, EP6_MAX_PKT_SIZE);
}


void VCOM_LineCoding(void)
{
    uint32_t data_len, parity, stop_len;

	NVIC_DisableIRQ(UART2_IRQn);
	
	// Reset software FIFO
	vcom.rx_bytes = 0;
	vcom.rx_head = 0;
	vcom.rx_tail = 0;

	vcom.tx_bytes = 0;
	vcom.tx_head = 0;
	vcom.tx_tail = 0;

	// Reset hardware FIFO
	UART2->FIFO = 0x3;
	
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
	
	UART_SetLineConfig(UART2, LineCfg.u32DTERate, data_len, parity, stop_len);

	NVIC_EnableIRQ(UART2_IRQn);
}


STR_VCOM_LINE_CODING LineCfg = {115200, 0, 0, 8};   // Baud rate : 115200, stop bits, parity bits, data bits


volatile VCOM vcom;


void UART2_IRQHandler(void)
{
    uint8_t chr;
    int32_t size;

    if((UART2->INTSTS & UART_INTSTS_RDAIF_Msk) || (UART2->INTSTS & UART_INTSTS_RXTOIF_Msk))
    {
        while((UART2->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) == 0)
        {
            chr = UART2->DAT;

            if(vcom.rx_bytes < RX_BUFF_SIZE)  // Check if buffer full
            {
                vcom.rx_buff[vcom.rx_tail++] = chr;
                if(vcom.rx_tail >= RX_BUFF_SIZE)
                    vcom.rx_tail = 0;
                vcom.rx_bytes++;
            }
        }
    }

    if(UART2->INTSTS & UART_INTSTS_THREIF_Msk)
    {
        if(vcom.tx_bytes)
        {
            /* Fill the Tx FIFO */
            size = vcom.tx_bytes;
            if(size >= TX_FIFO_SIZE)
            {
                size = TX_FIFO_SIZE;
            }

            while(size)
            {
                UART2->DAT = vcom.tx_buff[vcom.tx_head++];
                if(vcom.tx_head >= TX_BUFF_SIZE)
                    vcom.tx_head = 0;
                vcom.tx_bytes--;
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
    if(vcom.in_bytes == 0)
    {
        /* Check whether we have new COM Rx data to send to USB or not */
        if(vcom.rx_bytes)
        {
            len = vcom.rx_bytes;
            if(len > EP5_MAX_PKT_SIZE)
                len = EP5_MAX_PKT_SIZE;

            for(i = 0; i < len; i++)
            {
                vcom.in_buff[i] = vcom.rx_buff[vcom.rx_head++];
                if(vcom.rx_head >= RX_BUFF_SIZE)
                    vcom.rx_head = 0;
            }

            __disable_irq();
            vcom.rx_bytes -= len;
            __enable_irq();

            vcom.in_bytes = len;
            USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP5)), (uint8_t *)vcom.in_buff, len);
            USBD_SET_PAYLOAD_LEN(EP5, len);
        }
        else
        {
            /* Prepare a zero packet if previous packet size is EP2_MAX_PKT_SIZE and
               no more data to send at this moment to note Host the transfer has been done */
            len = USBD_GET_PAYLOAD_LEN(EP5);
            if(len == EP5_MAX_PKT_SIZE)
                USBD_SET_PAYLOAD_LEN(EP5, 0);
        }
    }

    /* Process the Bulk out data when bulk out data is ready. */
    if(vcom.out_ready && (vcom.out_bytes <= TX_BUFF_SIZE - vcom.tx_bytes))
    {
        for(i = 0; i < vcom.out_bytes; i++)
        {
            vcom.tx_buff[vcom.tx_tail++] = vcom.out_ptr[i];
            if(vcom.tx_tail >= TX_BUFF_SIZE)
                vcom.tx_tail = 0;
        }

        __disable_irq();
        vcom.tx_bytes += vcom.out_bytes;
        __enable_irq();

        vcom.out_bytes = 0;
        vcom.out_ready = 0; /* Clear bulk out ready flag */

        /* Ready to get next BULK out */
        USBD_SET_PAYLOAD_LEN(EP6, EP6_MAX_PKT_SIZE);
    }

    /* Process the software Tx FIFO */
    if(vcom.tx_bytes)
    {
        /* Check if Tx is working */
        if((UART2->INTEN & UART_INTEN_THREIEN_Msk) == 0)
        {
            /* Send one bytes out */
            UART2->DAT = vcom.tx_buff[vcom.tx_head++];
            if(vcom.tx_head >= TX_BUFF_SIZE)
                vcom.tx_head = 0;

            __set_PRIMASK(1);
            vcom.tx_bytes--;
            __set_PRIMASK(0);

            /* Enable Tx Empty Interrupt. (Trigger first one) */
            UART2->INTEN |= UART_INTEN_THREIEN_Msk;
        }
    }
}
