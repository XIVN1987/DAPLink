#include "NuMicro.h"
#include "vcom_serial.h"
#include "hid_transfer.h"

#include "DAP.h"


void systemInit(void);
void USB_Config(void);

int main(void)
{
	systemInit();
	
	DAP_Setup();
	
	USB_Config();
	
	GPIO_SetMode(PC, (1 << 0), GPIO_MODE_OUTPUT);	// PC0 => UART RXD 状态指示
	GPIO_SetMode(PC, (1 << 1), GPIO_MODE_OUTPUT);	// PC1 => UART TXD 状态指示
	
    while(1)
    {
		usbd_hid_process();
		
		VCOM_TransferData();
		
		PC0 = !(UART2->FIFOSTS & UART_FIFOSTS_RXIDLE_Msk);
		PC1 = !(UART2->FIFOSTS & UART_FIFOSTS_TXEMPTYF_Msk);
    }
}


void systemInit(void)
{
	SYS_UnlockReg();
	
	SYS->GPF_MFPH &= ~(SYS_GPF_MFPL_PF2MFP_Msk     | SYS_GPF_MFPL_PF3MFP_Msk);
    SYS->GPF_MFPH |=  (SYS_GPF_MFPL_PF2MFP_XT1_OUT | SYS_GPF_MFPL_PF3MFP_XT1_IN);
	
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);		// 使能HXT（外部晶振，12MHz）
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);	// 等待HXT稳定

    CLK_SetCoreClock(192000000);				// 用PLL产生指定频率作为系统时钟
	
	CLK->PCLKDIV = (CLK_PCLKDIV_PCLK0DIV2 | CLK_PCLKDIV_PCLK1DIV2);
	
	SYS_LockReg();
	
	SystemCoreClock = 192000000;
	
	CyclesPerUs = SystemCoreClock / 1000000;
}


void USB_Config(void)
{
	SYS_UnlockReg();
	
	SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA12MFP_Msk      | SYS_GPA_MFPH_PA13MFP_Msk     | SYS_GPA_MFPH_PA14MFP_Msk);
    SYS->GPA_MFPH |=  (SYS_GPA_MFPH_PA12MFP_USB_VBUS | SYS_GPA_MFPH_PA13MFP_USB_D_N | SYS_GPA_MFPH_PA14MFP_USB_D_P);
	
	SYS->USBPHY = (SYS->USBPHY & ~SYS_USBPHY_USBROLE_Msk) | SYS_USBPHY_USBEN_Msk | SYS_USBPHY_SBO_Msk;
	
	CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_USBDIV_Msk) | CLK_CLKDIV0_USB(4);
	
    CLK_EnableModuleClock(USBD_MODULE);

	SYS_LockReg();
	
	
    USBD_Open(&gsInfo, HID_ClassRequest, NULL);
    
    HID_Init();
	
	VCOM_Init();
	
    USBD_Start();

    NVIC_EnableIRQ(USBD_IRQn);
}
