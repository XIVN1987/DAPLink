#include "ch32v30x.h"
#include "ch32v30x_usb.h"
#include "ch32v30x_usbhs_device.h"

#include "DAP.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


/* 在 Properties for DAPLink -> C/C++ Build -> Settings -> C Compiler -> Preprocessor 中：
   定义 DAP_FW_V1：   生成 CMSIS-DAP v1 固件，使用 HID 传输协议
   不定义 DAP_FW_V1： 生成 CMSIS-DAP v2 固件，使用 WINUSB 传输协议
*/


volatile uint32_t SysTick_ms = 0;


void SysTick_Config(uint32_t ticks);
void int_to_unicode(uint32_t value , uint8_t *pbuf , uint8_t len);

void main(void)
{
    DAP_Setup();

    VCOM_Init();

    /* update serial number string with chip unique ID */
    extern uint8_t USB_StringSerialNbr[];
    uint32_t serial0 = *(uint32_t*)0x1FFFF7E8;
    uint32_t serial1 = *(uint32_t*)0x1FFFF7EC;
    uint32_t serial2 = *(uint32_t*)0x1FFFF7F0;

    int_to_unicode(serial0 + serial2, &USB_StringSerialNbr[2],  8);
    int_to_unicode(serial1 + serial2, &USB_StringSerialNbr[18], 4);

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


void int_to_unicode(uint32_t value , uint8_t *pbuf , uint8_t len)
{
    for(int i = 0 ; i < len ; i++)
    {
        if((value >> 28) < 0xA)
        {
            pbuf[2 * i] = (value >> 28) + '0';
        }
        else
        {
            pbuf[2 * i] = (value >> 28) + 'A' - 10;
        }

        pbuf[2 * i + 1] = 0;

        value = value << 4;
    }
}
