/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_istr.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/08/08
 * Description        : ISTR events interrupt service routines
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/ 
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_istr.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "vcom_serial.h"


/* Private variables */
__IO uint16_t wIstr;  
__IO uint8_t bIntPackSOF = 0;  
__IO uint32_t esof_counter =0;
__IO uint32_t wCNTR=0;

/* function pointers to non-control endpoints service routines */
void (*pEpInt_IN[7])(void) ={
	EP1_IN_Callback,
	EP2_IN_Callback,
	EP3_IN_Callback,
	EP4_IN_Callback,
	EP5_IN_Callback,
	EP6_IN_Callback,
	EP7_IN_Callback,
};

void (*pEpInt_OUT[7])(void) ={
	EP1_OUT_Callback,
	EP2_OUT_Callback,
	EP3_OUT_Callback,
	EP4_OUT_Callback,
	EP5_OUT_Callback,
	EP6_OUT_Callback,
	EP7_OUT_Callback,
};

/*******************************************************************************
 * @fn        USB_Istr
 *
 * @brief     ISTR events interrupt service routine
 *
 * @return    None.
 */
void USB_Istr(void)
{
    uint32_t i=0;
 __IO uint32_t EP[8];
  
  wIstr = _GetISTR();
#if (IMR_MSK & ISTR_SOF)
  if (wIstr & ISTR_SOF & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_SOF);
    bIntPackSOF++;

#ifdef SOF_CALLBACK
    SOF_Callback();
		
#endif
  }
#endif
 
  
#if (IMR_MSK & ISTR_CTR)
  if (wIstr & ISTR_CTR & wInterrupt_Mask)
  {
    CTR_LP();
#ifdef CTR_CALLBACK
    CTR_Callback();
#endif
  }
#endif
 
#if (IMR_MSK & ISTR_RESET)
  if (wIstr & ISTR_RESET & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_RESET);
    Device_Property.Reset();
    // SetDeviceAddress(0);
#ifdef RESET_CALLBACK
    RESET_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_DOVR)
  if (wIstr & ISTR_DOVR & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_DOVR);
#ifdef DOVR_CALLBACK
    DOVR_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_ERR)
  if (wIstr & ISTR_ERR & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_ERR);
#ifdef ERR_CALLBACK
    ERR_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_WKUP)
  if (wIstr & ISTR_WKUP & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_WKUP);
    Resume(RESUME_EXTERNAL);
#ifdef WKUP_CALLBACK
    WKUP_Callback();
#endif
  }
#endif
#if (IMR_MSK & ISTR_SUSP)
  if (wIstr & ISTR_SUSP & wInterrupt_Mask)
  {
    if (fSuspendEnabled)
    {
      Suspend();
    }
    else
    {
      Resume(RESUME_LATER);
    }
    _SetISTR((uint16_t)CLR_SUSP);
#ifdef SUSP_CALLBACK
    SUSP_Callback();
#endif
  }
#endif

#if (IMR_MSK & ISTR_ESOF)
  if (wIstr & ISTR_ESOF & wInterrupt_Mask)
  {
    _SetISTR((uint16_t)CLR_ESOF);
    
    if ((_GetFNR()&FNR_RXDP)!=0)
    {
      esof_counter ++;
      
      if ((esof_counter >3)&&((_GetCNTR()&CNTR_FSUSP)==0))
      {           

        wCNTR = _GetCNTR(); 
      
        for (i=0;i<8;i++) EP[i] = _GetENDPOINT(i);
      
        wCNTR|=CNTR_FRES;
        _SetCNTR(wCNTR);
 
        wCNTR&=~CNTR_FRES;
        _SetCNTR(wCNTR);
      
        while((_GetISTR()&ISTR_RESET) == 0);
  
        _SetISTR((uint16_t)CLR_RESET);
   
        for (i=0;i<8;i++)
        _SetENDPOINT(i, EP[i]);
      
        esof_counter = 0;
      }
    }
    else
    {
        esof_counter = 0;
    }
    
    Resume(RESUME_ESOF); 

#ifdef ESOF_CALLBACK
    ESOF_Callback();
#endif
  }
#endif
} /* USB_Istr */


void HID_SetInReport(void);
void EP1_IN_Callback(void)
{
    HID_SetInReport();
}


void HID_GetOutReport(uint8_t * buf, uint32_t len);
void EP1_OUT_Callback(void)
{
    uint8_t rcvbuff[64] = {0};

    USB_SIL_Read(EP1_OUT, rcvbuff);

    HID_GetOutReport(rcvbuff, 64);

    SetEPRxStatus(ENDP1, EP_RX_VALID);
}


void EP2_IN_Callback(void)
{
}


void EP3_IN_Callback(void)
{
    Vcom.in_ready = 1;
}


void EP3_OUT_Callback(void)
{
    Vcom.out_bytes = USB_SIL_Read(EP3_OUT, (uint8_t *)Vcom.out_buff);

    Vcom.out_ready = 1;
}


/***************************************************************/
#include "DAP_Config.h"
#include "DAP.h"

static volatile uint8_t  USB_RequestFull;       // Request  Buffer Full Flag
static volatile uint32_t USB_RequestIn;         // Request  Buffer In  Index
static volatile uint32_t USB_RequestOut;        // Request  Buffer Out Index

static volatile uint8_t  USB_ResponseIdle = 1;  // Response Buffer Idle Flag
static volatile uint8_t  USB_ResponseFull;      // Response Buffer Full Flag
static volatile uint32_t USB_ResponseIn;        // Response Buffer In  Index
static volatile uint32_t USB_ResponseOut;       // Response Buffer Out Index

static uint8_t  USB_Request [DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Request  Buffer
static uint8_t  USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Response Buffer
static uint16_t USB_ResponseSize[DAP_PACKET_COUNT];               // number of bytes in response


uint8_t usbd_hid_process(void)
{
    uint32_t n;

    if((USB_RequestOut != USB_RequestIn) || USB_RequestFull)
    {
        USB_ResponseSize[USB_ResponseIn] = DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);

        n = USB_RequestOut + 1;
        if(n == DAP_PACKET_COUNT)
            n = 0;
        USB_RequestOut = n;

        if(USB_RequestOut == USB_RequestIn)
            USB_RequestFull = 0;

        if(USB_ResponseIdle)
        {
            USB_ResponseIdle = 0;

#ifdef DAP_FW_V1
            USB_SIL_Write(EP1_IN, (void *)USB_Response[USB_ResponseIn], DAP_PACKET_SIZE);
#else
            USB_SIL_Write(EP1_IN, (void *)USB_Response[USB_ResponseIn], USB_ResponseSize[USB_ResponseIn]);
#endif
            SetEPTxValid(ENDP1);
        }
        else
        {
            n = USB_ResponseIn + 1;
            if(n == DAP_PACKET_COUNT)
                n = 0;
            USB_ResponseIn = n;

            if(USB_ResponseIn == USB_ResponseOut)
                USB_ResponseFull = 1;
        }

        return 1;
    }

    return 0;
}


void HID_GetOutReport(uint8_t * buf, uint32_t len)
{
    if(buf[0] == ID_DAP_TransferAbort)
    {
        DAP_TransferAbort = 1;
        return;
    }

    if((USB_RequestIn == USB_RequestOut) && USB_RequestFull)    // Request  Buffer Full
        return;

    memcpy(USB_Request[USB_RequestIn], buf, len);

    USB_RequestIn++;
    if(USB_RequestIn == DAP_PACKET_COUNT)
        USB_RequestIn = 0;

    if(USB_RequestIn == USB_RequestOut)
        USB_RequestFull = 1;
}


void HID_SetInReport(void)
{
    if((USB_ResponseOut != USB_ResponseIn) || USB_ResponseFull)
    {
#ifdef DAP_FW_V1
        USB_SIL_Write(EP1_IN, (void *)USB_Response[USB_ResponseOut], DAP_PACKET_SIZE);
#else
        USB_SIL_Write(EP1_IN, (void *)USB_Response[USB_ResponseOut], USB_ResponseSize[USB_ResponseOut]);
#endif
        SetEPTxValid(ENDP1);

        USB_ResponseOut++;
        if(USB_ResponseOut == DAP_PACKET_COUNT)
            USB_ResponseOut = 0;

        if(USB_ResponseOut == USB_ResponseIn)
            USB_ResponseFull = 0;
    }
    else
    {
        USB_ResponseIdle = 1;
    }
}
