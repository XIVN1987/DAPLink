#include "at32f425.h"
#include "usb_core.h"
#include "usbd_int.h"

#include "DAP.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


otg_core_type Otg;


void systemInit(void);
void USB_Config(void);

int main(void)
{
	systemInit();
	
	DAP_Setup();

	USB_Config();

	SysTick_Config(SystemCoreClock / 1000);		// 1ms interrupt

	while(1)
	{
		usbd_hid_process();
		
		VCOM_TransferData();
	}
}


void systemInit(void)
{
	flash_psr_set(FLASH_WAIT_CYCLE_2);

	crm_pll_config(CRM_PLL_SOURCE_HICK, CRM_PLL_MULT_24);	// 4MHz * 24 = 96MHz
	crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);
	while(crm_flag_get(CRM_PLL_STABLE_FLAG) != SET)
	{
	}

	crm_ahb_div_set(CRM_AHB_DIV_1);
	crm_apb1_div_set(CRM_APB1_DIV_1);
	crm_apb2_div_set(CRM_APB2_DIV_1);

	crm_sysclk_switch(CRM_SCLK_PLL);
	while(crm_sysclk_switch_status_get() != CRM_SCLK_PLL)
	{
	}

	system_core_clock_update();
}


void USB_Config(void)
{
	crm_periph_clock_enable(CRM_OTGFS1_PERIPH_CLOCK, TRUE);
	crm_periph_clock_enable(CRM_ACC_PERIPH_CLOCK, TRUE);

	crm_usb_clock_source_select(CRM_USB_CLOCK_SOURCE_HICK);	// => crm_hick_sclk_frequency_select(CRM_HICK_SCLK_48MHZ);
															//	 => crm_hick_divider_select(CRM_HICK48_NODIV);
	
    /* HICK 模块输出的原始时钟频率为 48MHz，但校准模块使用的采样时钟是除频（1/6）电路输出的时钟，频率约 8MHz */
    acc_write_c1(7980);
    acc_write_c2(8000);
    acc_write_c3(8020);
    acc_calibration_mode_enable(ACC_CAL_HICKTRIM, TRUE);
	
	extern usbd_desc_handler  hid_desc_handler;
	extern usbd_class_handler hid_class_handler;
	usbd_init(&Otg, USB_FULL_SPEED_CORE_ID, USB_OTG1_ID, &hid_class_handler, &hid_desc_handler);

    NVIC_EnableIRQ(OTGFS1_IRQn);
}


void OTGFS1_IRQHandler(void)
{
	usbd_irq_handler(&Otg);
}


volatile uint32_t SysTick_ms = 0;

void SysTick_Handler(void)
{
	SysTick_ms++;
}


void usb_delay_ms(uint32_t ms)
{
	uint32_t start = SysTick_ms;
	
	while(SysTick_ms - start < ms) __NOP();
}
