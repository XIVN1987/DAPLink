#include "at32f425.h"
#include "usbd_sdr.h"
#include "usbd_core.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


volatile VCOM Vcom = {.in_ready = 1};

VCOM_LINE_CODING LineCfg = {115200, 0, 0, 8};   // Baud rate, stop bits, parity bits, data bits


void VCOM_Init(void)
{
	crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
	crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
	
	gpio_init_type gpio_init_struct;
	gpio_default_para_init(&gpio_init_struct);
	gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	gpio_init_struct.gpio_pins = GPIO_PINS_2 | GPIO_PINS_3;
	gpio_init(GPIOA, &gpio_init_struct);

	gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE2, GPIO_MUX_1);		// PA2 => USART2_TX
	gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE3, GPIO_MUX_1);		// PA3 => USART2_RX
	
	usart_init(USART2, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
	usart_interrupt_enable(USART2, USART_RDBF_INT, TRUE);
	usart_transmitter_enable(USART2, TRUE);
	usart_receiver_enable(USART2, TRUE);
	usart_enable(USART2, TRUE);
	
	NVIC_EnableIRQ(USART2_IRQn);
}


void VCOM_LineCoding(VCOM_LINE_CODING * LineCfgx)
{
    usart_data_bit_num_type data_len;
	usart_stop_bit_num_type stop_len;
	usart_parity_selection_type parity;
    
    switch(LineCfgx->u8DataBits)
    {
    case 7:  data_len = USART_DATA_7BITS; break;
    case 8:  data_len = USART_DATA_8BITS; break;
    default: data_len = USART_DATA_8BITS; break;
    }
    
    switch(LineCfgx->u8ParityType)
    {
    case 0:  parity = USART_PARITY_NONE;  break;
    case 1:  parity = USART_PARITY_ODD;   break;
    case 2:  parity = USART_PARITY_EVEN;  break;
    default: parity = USART_PARITY_NONE;  break;
    }
    
    switch(LineCfgx->u8CharFormat)
    {
    case 0:  stop_len = USART_STOP_1_BIT;   break;
    case 1:  stop_len = USART_STOP_1_5_BIT; break;
    case 2:  stop_len = USART_STOP_2_BIT;   break;
    default: stop_len = USART_STOP_1_BIT;   break;
    }
    
    __disable_irq();
    
    // Reset software FIFO
    Vcom.rx_bytes = 0;
	Vcom.rx_wrptr = 0;
    Vcom.rx_rdptr = 0;

    Vcom.tx_bytes = 0;
	Vcom.tx_wrptr = 0;
    Vcom.tx_rdptr = 0;
    
    usart_init(USART2, LineCfgx->u32DTERate, data_len, stop_len);
    usart_parity_selection_config(USART2, parity);
    
    __enable_irq();
}


void USART2_IRQHandler(void)
{
	if(usart_flag_get(USART2, USART_RDBF_FLAG) != RESET)
	{
		uint16_t chr = usart_data_receive(USART2);
		
		if(Vcom.rx_bytes < RX_BUFF_SIZE)
		{
			Vcom.rx_buff[Vcom.rx_wrptr++] = chr;
			if(Vcom.rx_wrptr == RX_BUFF_SIZE)
				Vcom.rx_wrptr = 0;
			
			Vcom.rx_bytes++;
		}
	}
	
	if(usart_flag_get(USART2, USART_TDBE_FLAG) != RESET)
	{
		if(Vcom.tx_bytes)
        {
			usart_data_transmit(USART2, Vcom.tx_buff[Vcom.tx_rdptr++]);
			if(Vcom.tx_rdptr == TX_BUFF_SIZE)
				Vcom.tx_rdptr = 0;
			
			Vcom.tx_bytes--;
        }
        else
        {
            /* No more data, just stop Tx (Stop work) */
            usart_interrupt_enable(USART2, USART_TDBE_INT, FALSE);
        }
	}
}


void VCOM_InComplete(void)
{
    Vcom.in_ready = 1;
}


void VCOM_GetOutData(uint8_t * buf, uint32_t len)
{
	Vcom.out_pbuf  = buf;
    Vcom.out_bytes = len;

    Vcom.out_ready = 1;
}


extern otg_core_type Otg;

void VCOM_TransferData(void)
{
    if(Vcom.in_ready)		// 可以向主机发送数据
    {
        if(Vcom.rx_bytes)	// 有新的数据可以发送
        {
            Vcom.in_bytes = Vcom.rx_bytes;
            if(Vcom.in_bytes > CDC_BULK_IN_SZ)
                Vcom.in_bytes = CDC_BULK_IN_SZ;

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
			usbd_ept_send(&Otg.dev, CDC_BULK_IN_EP, (uint8_t *)Vcom.in_buff, Vcom.in_bytes);
        }
        else
        {
            /* Prepare a zero packet if previous packet size is CDC_BULK_IN_SZ and
               no more data to send at this moment to note Host the transfer has been done */
            if(Vcom.in_bytes == CDC_BULK_IN_SZ)
			{
				Vcom.in_bytes = 0;
				
                usbd_ept_send(&Otg.dev, CDC_BULK_IN_EP, 0, 0);
			}
        }
    }

	/* 从主机接收到数据，且 tx_buff 能够装下它们 */
    if(Vcom.out_ready && (Vcom.out_bytes <= TX_BUFF_SIZE - Vcom.tx_bytes))
    {
        for(int i = 0; i < Vcom.out_bytes; i++)
        {
            Vcom.tx_buff[Vcom.tx_wrptr++] = Vcom.out_pbuf[i];
            if(Vcom.tx_wrptr >= TX_BUFF_SIZE)
                Vcom.tx_wrptr = 0;
        }

        __disable_irq();
        Vcom.tx_bytes += Vcom.out_bytes;
        __enable_irq();

        Vcom.out_ready = 0;

        /* Ready for next BULK OUT */
		extern uint8_t cdc_rx_buff[64];
        usbd_ept_recv(&Otg.dev, CDC_BULK_OUT_EP, cdc_rx_buff, CDC_BULK_OUT_SZ);
    }

    if((Vcom.tx_bytes) && (USART2->ctrl1_bit.tdbeien == 0))
    {
        usart_interrupt_enable(USART2, USART_TDBE_INT, TRUE);
    }
}
