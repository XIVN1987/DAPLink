#include "ch32v20x.h"
#include "usb_lib.h"
#include "usb_istr.h"

#include "DAP.h"
#include "vcom_serial.h"

extern uint8_t usbd_hid_process(void);

volatile uint32_t SysTick_ms = 0;

void SysTick_Config(uint32_t ticks);
void USB_Config(void);

int main(void)
{
    DAP_Setup();

    VCOM_Init();

    USB_Config();

    SysTick_Config(SystemCoreClock / 1000);
 	
	while(1)
	{
	    usbd_hid_process();

	    VCOM_TransferData();
	}
}


void SysTick_Config(uint32_t ticks)
{
    SysTick->CTLR= 0;
    SysTick->SR  = 0;
    SysTick->CNT = 0;
    SysTick->CMP = ticks;
    SysTick->CTLR= 0x0F;

    NVIC_EnableIRQ(SysTicK_IRQn);
}


void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)
{
    SysTick->SR = 0;

    SysTick_ms++;
}


void USB_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div3);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

    USB_Init();

    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_ClearITPendingBit(EXTI_Line18);
    EXTI_InitStructure.EXTI_Line = EXTI_Line18;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
}


void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USBWakeUp_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));


void USB_LP_CAN1_RX0_IRQHandler(void)
{
    USB_Istr();
}


void USBWakeUp_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line18);
}


/*******************************************************************************
 * @fn         USB_Port_Set
 *
 * @brief      Set USB IO port.
 *
 * @param      NewState: DISABLE or ENABLE.
 *             Pin_In_IPU: Enables or Disables intenal pull-up resistance.
 *
 * @return     None
 */
void USB_Port_Set(FunctionalState NewState, FunctionalState Pin_In_IPU)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    if(NewState)
    {
        _SetCNTR(_GetCNTR()&(~(1<<1)));
        GPIOA->CFGHR&=0XFFF00FFF;
        GPIOA->OUTDR&=~(3<<11); //PA11/12=0
        GPIOA->CFGHR|=0X00044000; //float
    }
    else
    {
        _SetCNTR(_GetCNTR()|(1<<1));
        GPIOA->CFGHR&=0XFFF00FFF;
        GPIOA->OUTDR&=~(3<<11); //PA11/12=0
        GPIOA->CFGHR|=0X00033000;   // LOW
    }

    if(Pin_In_IPU) EXTEN->EXTEN_CTR |=  EXTEN_USBD_PU_EN;
    else           EXTEN->EXTEN_CTR &= ~EXTEN_USBD_PU_EN;
}


void Delay_Ms(uint32_t n)
{
    for(int i = 0; i < SystemCoreClock / 4000 * n; i++) __NOP();
}

