#include <stdio.h>
#include "NuMicro.h"
#include "vcom_serial.h"
#include "hid_transfer.h"

#include "DAP.h"


void systemInit(void);
void SerialInit(void);
void USB_Config(void);

int main(void)
{	
	systemInit();
	
	SerialInit();
	
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
    
    HID_Init();		// Endpoint configuration
	
	VCOM_Init();	// Endpoint configuration
	
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
	GPIO_SetPullCtl(PB, BIT0, GPIO_PUSEL_PULL_UP);
	SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk       | SYS_GPB_MFPL_PB1MFP_Msk);
    SYS->GPB_MFPL |=  (SYS_GPB_MFPL_PB0MFP_UART2_RXD | SYS_GPB_MFPL_PB1MFP_UART2_TXD);
	
	CLK_SetModuleClock(UART2_MODULE, CLK_CLKSEL3_UART2SEL_HXT, CLK_CLKDIV4_UART2(1));
	CLK_EnableModuleClock(UART2_MODULE);
	
	UART_Open(UART2, 115200);
	UART_ENABLE_INT(UART2, (UART_INTEN_RDAIEN_Msk | UART_INTEN_THREIEN_Msk | UART_INTEN_RXTOIEN_Msk));

    NVIC_EnableIRQ(UART2_IRQn);
}
