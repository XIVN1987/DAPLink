#include "ch32v30x.h"
#include "ch32v30x_usb.h"
#include "ch32v30x_usbhs_device.h"

#include "DAP.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


volatile uint32_t SysTick_ms = 0;


void SysTick_Config(uint32_t ticks);


void main(void)
{
    DAP_Setup();

    VCOM_Init();

    USBHS_RCC_Init();
    USBHS_Device_Init(ENABLE);

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
