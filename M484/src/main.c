#include "NuMicro.h"
#include "vcom_serial.h"
#include "hid_transfer.h"

#include "DAP.h"


/* 在 Option 弹窗的 C/C++ 页中：
   定义 DAP_FW_V1：   生成 CMSIS-DAP v1 固件，使用 HID 传输协议
   不定义 DAP_FW_V1： 生成 CMSIS-DAP v2 固件，使用 WINUSB 传输协议
*/


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
	
	SYS->USBPHY &= ~SYS_USBPHY_HSUSBROLE_Msk;	// Device only
	
    SYS->USBPHY &= ~SYS_USBPHY_HSUSBACT_Msk;
    SYS->USBPHY |=  SYS_USBPHY_HSUSBEN_Msk;
    for(int i = 0; i < CyclesPerUs*10; i++) __NOP();	// at lease 10uS
    SYS->USBPHY |=  SYS_USBPHY_HSUSBACT_Msk;

    CLK_EnableModuleClock(HSUSBD_MODULE);

	SYS_LockReg();
	
	
	HSUSBD_Open(&gsHSInfo, HID_ClassRequest, NULL);
    HSUSBD_SetVendorRequest(WINUSB_VendorRequest);

    HID_Init();		// Endpoint configuration
	
	VCOM_Init();

    HSUSBD_Start();
    
    NVIC_EnableIRQ(USBD20_IRQn);
}
