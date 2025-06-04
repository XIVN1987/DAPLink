#include <string.h>
#include "at32f425.h"
#include "usbd_sdr.h"
#include "usbd_core.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


volatile VCOM Vcom;

VCOM_LINE_CODING LineCfg = {115200, 0, 0, 8};   // Baud rate, stop bits, parity bits, data bits


#define RXDMA_SZ  (CDC_BULK_IN_SZ * 2)
uint8_t RXBuffer[RXDMA_SZ] __attribute__((aligned(4)));
uint8_t TXBuffer[CDC_BULK_OUT_SZ] __attribute__((aligned(4)));


void VCOM_Init(void)
{
	gpio_init_type gpio_init_struct;
	dma_init_type  dma_init_struct;
	
	crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
	crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
	crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
	
	gpio_default_para_init(&gpio_init_struct);
	gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
	gpio_init_struct.gpio_pins = GPIO_PINS_2 | GPIO_PINS_3;
	gpio_init(GPIOA, &gpio_init_struct);

	gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE2, GPIO_MUX_1);		// PA2 => USART2_TX
	gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE3, GPIO_MUX_1);		// PA3 => USART2_RX
	
	
	dma_flexible_config(DMA1, FLEX_CHANNEL1, DMA_FLEXIBLE_UART2_TX);
	dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_base_addr = (uint32_t)TXBuffer;
	dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
	dma_init_struct.memory_inc_enable = TRUE;
	dma_init_struct.peripheral_base_addr = (uint32_t)&USART2->dt;
	dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
	dma_init_struct.peripheral_inc_enable = FALSE;
	dma_init_struct.buffer_size = 0;
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init_struct.loop_mode_enable = FALSE;
	dma_init(DMA1_CHANNEL1, &dma_init_struct);
	dma_channel_enable(DMA1_CHANNEL1, TRUE);

	dma_flexible_config(DMA1, FLEX_CHANNEL2, DMA_FLEXIBLE_UART2_RX);
	dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
	dma_init_struct.memory_base_addr = (uint32_t)RXBuffer;
	dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
	dma_init_struct.memory_inc_enable = TRUE;
	dma_init_struct.peripheral_base_addr = (uint32_t)&USART2->dt;
	dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
	dma_init_struct.peripheral_inc_enable = FALSE;
	dma_init_struct.buffer_size = RXDMA_SZ;
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init_struct.loop_mode_enable = TRUE;
	dma_init(DMA1_CHANNEL2, &dma_init_struct);
	dma_channel_enable(DMA1_CHANNEL2, TRUE);
	
	
	usart_init(USART2, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
	usart_dma_transmitter_enable(USART2, TRUE);
	usart_dma_receiver_enable(USART2, TRUE);
	usart_transmitter_enable(USART2, TRUE);
	usart_receiver_enable(USART2, TRUE);
	usart_enable(USART2, TRUE);
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
    
    usart_init(USART2, LineCfgx->u32DTERate, data_len, stop_len);
    usart_parity_selection_config(USART2, parity);
    
    __enable_irq();
}


extern otg_core_type Otg;
extern volatile uint32_t SysTick_ms;

void VCOM_TransferData(void)
{
	static uint32_t last_ms = 0;
    static uint32_t last_pos = 0;
	
    if(Vcom.in_ready)		// 可以向主机发送数据
    {
        uint32_t pos = RXDMA_SZ - dma_data_number_get(DMA1_CHANNEL2);
        if((pos - last_pos >= CDC_BULK_IN_SZ) || ((pos != last_pos) && (SysTick_ms != last_ms)))
        {
            if(pos < last_pos)
                pos = RXDMA_SZ;

            if(pos - last_pos > CDC_BULK_IN_SZ)
                pos = last_pos + CDC_BULK_IN_SZ;

            Vcom.in_bytes = pos - last_pos;
			
			Vcom.in_ready = 0;
			
			memcpy((uint8_t *)Vcom.in_buff, &RXBuffer[last_pos], Vcom.in_bytes);
			usbd_ept_send(&Otg.dev, CDC_BULK_IN_EP, (uint8_t *)Vcom.in_buff, Vcom.in_bytes);
			
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
				
                usbd_ept_send(&Otg.dev, CDC_BULK_IN_EP, (uint8_t *)0, 0);
			}
        }
    }

	/* 从主机接收到数据，且前面的数据 DMA 已发送完 */
    if(Vcom.out_bytes && (dma_data_number_get(DMA1_CHANNEL1) == 0))
    {
		memcpy(TXBuffer, (uint8_t *)Vcom.out_buff, Vcom.out_bytes);
		
		dma_channel_enable(DMA1_CHANNEL1, FALSE);
        dma_data_number_set(DMA1_CHANNEL1, Vcom.out_bytes);
        dma_channel_enable(DMA1_CHANNEL1, TRUE);
		
		Vcom.out_bytes = 0;

        /* Ready for next BULK OUT */
        usbd_ept_recv(&Otg.dev, CDC_BULK_OUT_EP, (uint8_t *)Vcom.out_buff, CDC_BULK_OUT_SZ);
    }
}
