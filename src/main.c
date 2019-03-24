#include <stdio.h>
#include "M480.h"
#include "hid_transfer.h"

#include "DAP.h"

void systemInit(void);
void SerialInit(void);
void USB_Config(void);

int main(void)
{
	uint32_t i;
	
	systemInit();
	
	SerialInit();
	
	DAP_Setup();
	
	USB_Config();
	
	GPIO_SetMode(PB, BIT14, GPIO_MODE_OUTPUT);
	
    while(1)
    {	
		usbd_hid_process();
		
		if(++i%100000 == 0) PB14 = 1 - PB14;
    }
}


void USB_Config(void)
{
	SYS_UnlockReg();
	
	SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA12MFP_Msk      | SYS_GPA_MFPH_PA13MFP_Msk     | SYS_GPA_MFPH_PA14MFP_Msk     | SYS_GPA_MFPH_PA15MFP_Msk);
    SYS->GPA_MFPH |=  (SYS_GPA_MFPH_PA12MFP_USB_VBUS | SYS_GPA_MFPH_PA13MFP_USB_D_N | SYS_GPA_MFPH_PA14MFP_USB_D_P | SYS_GPA_MFPH_PA15MFP_USB_OTG_ID);
	
	SYS->USBPHY = (SYS->USBPHY & ~SYS_USBPHY_USBROLE_Msk) | SYS_USBPHY_USBEN_Msk | SYS_USBPHY_SBO_Msk;
	
	CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_USBDIV_Msk) | CLK_CLKDIV0_USB(4);
	
    CLK_EnableModuleClock(USBD_MODULE);

	SYS_LockReg();
	
	
    USBD_Open(&gsInfo, HID_ClassRequest, NULL);
    
    HID_Init();		// Endpoint configuration
	
    USBD_Start();

    NVIC_EnableIRQ(USBD_IRQn);
}


void systemInit(void)
{
	SYS_UnlockReg();
	
	SYS->GPF_MFPH &= ~(SYS_GPF_MFPL_PF2MFP_Msk     | SYS_GPF_MFPL_PF3MFP_Msk);
    SYS->GPF_MFPH |=  (SYS_GPF_MFPL_PF2MFP_XT1_OUT | SYS_GPF_MFPL_PF3MFP_XT1_IN);
	
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);		// 使能HXT（外部晶振，12MHz）
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);	// 等待HXT稳定

    CLK_SetCoreClock(192000000);				// 用PLL产生指定频率作为系统时钟
												// 若HXT使能，则PLL时钟源选择HXT，须根据实际情况修改__HXT的值
	
	CLK->PCLKDIV = (CLK_PCLKDIV_PCLK0DIV2 | CLK_PCLKDIV_PCLK1DIV2);
	
	SYS_LockReg();
	
	SystemCoreClock = 192000000;
	
	CyclesPerUs = SystemCoreClock / 1000000;
}


void SerialInit(void)
{	
	SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk       | SYS_GPA_MFPL_PA1MFP_Msk);
    SYS->GPA_MFPL |=  (SYS_GPA_MFPL_PA0MFP_UART0_RXD | SYS_GPA_MFPL_PA1MFP_UART0_TXD);
	
	CLK_EnableModuleClock(UART0_MODULE);
	
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HXT, CLK_CLKDIV0_UART0(1));
 	
	UART_Open(UART0, 115200);
}


int fputc(int ch, FILE *stream)
{
	while(UART0->FIFOSTS & UART_FIFOSTS_TXFULL_Msk) __NOP();
    UART0->DAT = ch;
	
    if(ch == '\n')
    {
        while(UART0->FIFOSTS & UART_FIFOSTS_TXFULL_Msk) __NOP();
        UART0->DAT = '\r';
    }
	
    return ch;
}
