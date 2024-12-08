/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_prop.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2021/08/08
 * Description        : All processing related to Virtual Com Port Demo
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/ 
#include <stdlib.h>
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"

#include "vcom_serial.h"

uint8_t Request = 0;

volatile uint8_t HID_Idle_Value[2] = {0};
volatile uint8_t HID_Protocol_Value[2] = {0};

DEVICE Device_Table =
{
	EP_NUM,
	1
};

DEVICE_PROP Device_Property =
{
	USBD_init,
	USBD_Reset,
	USBD_Status_In,
	USBD_Status_Out,
	USBD_Data_Setup,
	USBD_NoData_Setup,
	USBD_Get_Interface_Setting,
	USBD_GetDeviceDescriptor,
	USBD_GetConfigDescriptor,
	USBD_GetStringDescriptor,
	0,
	USB_MAX_EP0_SZ
};

USER_STANDARD_REQUESTS User_Standard_Requests =
{
	USBD_GetConfiguration,
	USBD_SetConfiguration,
	USBD_GetInterface,
	USBD_SetInterface,
	USBD_GetStatus, 
	USBD_ClearFeature,
	USBD_SetEndPointFeature,
	USBD_SetDeviceFeature,
	USBD_SetDeviceAddress
};

ONE_DESCRIPTOR Device_Descriptor =
{
	(uint8_t*)USBD_DeviceDescriptor,
	USBD_SIZE_DEVICE_DESC
};

ONE_DESCRIPTOR Config_Descriptor =
{
	(uint8_t*)USBD_ConfigDescriptor,
	USBD_SIZE_CONFIG_TOTAL
};

ONE_DESCRIPTOR String_Descriptor[5] =
{
	{(uint8_t*)USBD_StringLangID,   USBD_SIZE_STRING_LANGID},
	{(uint8_t*)USBD_StringVendor,   USBD_SIZE_STRING_VENDOR},
	{(uint8_t*)USBD_StringProduct,  USBD_SIZE_STRING_PRODUCT},
	{(uint8_t*)USBD_StringSerial,   USBD_SIZE_STRING_SERIAL},
	{(uint8_t*)USBD_StringInterface,USBD_SIZE_STRING_PRODUCT}
};

ONE_DESCRIPTOR Report_Descriptor[1] =
{
	{(uint8_t*)USBD_HidRepDesc, USBD_SIZE_REPORT_DESC},
};

ONE_DESCRIPTOR Hid_Descriptor[1] =
{
	{(uint8_t*)&USBD_ConfigDescriptor[93], 0x09},
};
/*********************************************************************
 * @fn      USBD_SetConfiguration.
 *
 * @brief     Update the device state to configured.
 *
 * @return    None.
 */
void USBD_SetConfiguration(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  if (pInfo->Current_Configuration != 0)
  {
    bDeviceState = CONFIGURED;
  }
}

/*******************************************************************************
 * @fn         USBD_SetDeviceAddress.
 *
 * @brief      Update the device state to addressed.
 *
 * @return     None.
 */
void USBD_SetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}


/*********************************************************************
 * @fn      USBD_SetDeviceFeature.
 *
 * @brief   SetDeviceFeature Routine.
 *
 * @return  none
 */
void USBD_SetDeviceFeature (void)
{

}



/*********************************************************************
 * @fn      USBD_ClearFeature.
 *
 * @brief   ClearFeature Routine.
 *
 * @return  none
 */
void USBD_ClearFeature(void)
{

}





/*********************************************************************
 * @fn      USBD_Status_In.
 *
 * @brief    USBD Status In Routine.
 *
 * @return   None.
 */
void USBD_Status_In(void)
{
    uint32_t Request_No = pInformation->USBbRequest;
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
    {
        if (Request_No == CDC_SET_LINE_CODING)
        {
            VCOM_LineCoding(&LineCfg);
        }
    }
}

/*******************************************************************************
 * @fn       USBD_Status_Out
 *
 * @brief    USBD Status OUT Routine.
 *
 * @return   None.
 */
void USBD_Status_Out(void)
{
    
}

/*******************************************************************************
 * @fn       USBD_init.
 *
 * @brief    init routine.
 * 
 * @return   None.
 */
void USBD_init(void)
{
  uint8_t	i;
    
  pInformation->Current_Configuration = 0;
  PowerOn();
  for (i=0;i<8;i++) _SetENDPOINT(i,_GetENDPOINT(i) & 0x7F7F & EPREG_MASK);//all clear
  _SetISTR((uint16_t)0x00FF);//all clear
  USB_SIL_Init();
  bDeviceState = UNCONNECTED;
  
  USB_Port_Set(DISABLE, DISABLE);	
  Delay_Ms(20);
  USB_Port_Set(ENABLE, ENABLE);    
}

/*******************************************************************************
 * @fn      USBD_Reset
 *
 * @brief   USBD reset routine
 *
 * @return  none
 */
void USBD_Reset(void)
{
    pInformation->Current_Configuration = 0;
    pInformation->Current_Feature = USBD_ConfigDescriptor[7];
    pInformation->Current_Interface = 0;

    SetBTABLE(BTABLE_ADDRESS);

    /* Initialize Endpoint 0 */
    SetEPType(ENDP0, EP_CONTROL);
    SetEPTxAddr(ENDP0, ENDP0_TXADDR);
    SetEPRxAddr(ENDP0, ENDP0_RXADDR);
    SetEPTxCount(ENDP0, Device_Property.MaxPacketSize);
    SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
    SetEPTxStatus(ENDP0, EP_TX_STALL);
    SetEPRxStatus(ENDP0, EP_RX_VALID);
    _ClearDTOG_RX(ENDP0);
    _ClearDTOG_TX(ENDP0);

    /* Initialize Endpoint 1 */
    SetEPType(ENDP1, EP_INTERRUPT);
    SetEPTxAddr(ENDP1, ENDP1_TXADDR);
    SetEPRxAddr(ENDP1, ENDP1_RXADDR);
    SetEPTxCount(ENDP1, HID_INT_IN_SZ);
    SetEPRxCount(ENDP1, HID_INT_OUT_SZ);
    SetEPTxStatus(ENDP1, EP_TX_NAK);
    SetEPRxStatus(ENDP1, EP_RX_VALID);
    _ClearDTOG_TX(ENDP1);
    _ClearDTOG_RX(ENDP1);

    /* Initialize Endpoint 2 */
    SetEPType(ENDP2, EP_INTERRUPT);
    SetEPTxAddr(ENDP2, ENDP2_TXADDR);
    SetEPTxCount(ENDP2, CDC_INT_IN_SZ);
    SetEPTxStatus(ENDP2, EP_TX_NAK);
    SetEPRxStatus(ENDP2, EP_RX_DIS);
    _ClearDTOG_TX(ENDP1);

    /* Initialize Endpoint 3 */
    SetEPType(ENDP3, EP_BULK);
    SetEPTxAddr(ENDP3, ENDP3_TXADDR);
    SetEPRxAddr(ENDP3, ENDP3_RXADDR);
    SetEPTxCount(ENDP3, CDC_BULK_IN_SZ);
    SetEPRxCount(ENDP3, CDC_BULK_OUT_SZ);
    SetEPTxStatus(ENDP3, EP_TX_NAK);
    SetEPRxStatus(ENDP3, EP_RX_VALID);
    _ClearDTOG_TX(ENDP3);
    _ClearDTOG_RX(ENDP3);

    SetDeviceAddress(0);

    bDeviceState = ATTACHED;

    Vcom.in_ready = 1;
}

/*********************************************************************
 * @fn      USBD_GetDeviceDescriptor.
 *
 * @brief   Gets the device descriptor.
 *
 * @param   Length.
 *
 * @return  The address of the device descriptor.
 */
uint8_t *USBD_GetDeviceDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/*********************************************************************
 * @fn      USBD_GetConfigDescriptor.
 *
 * @brief   get the configuration descriptor.
 *
 * @param   Length.
 *
 * @return   The address of the configuration descriptor.
 */
uint8_t *USBD_GetConfigDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*********************************************************************
 * @fn      USBD_GetStringDescriptor
 *
 * @brief   Gets the string descriptors according to the needed index
 *
 * @param   Length.
 *
 * @return    The address of the string descriptors.
 */
uint8_t *USBD_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;
	
  if (wValue0 > 4)
  {
    return NULL;
  }
  else
  {
    return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
  }
}

/*********************************************************************
 * @fn      USBD_GetReportDescriptor
 *
 * @brief   Gets the report descriptors according to the needed index
 *
 * @param   Length.
 *
 * @return  The address of the device descriptor.
 */
uint8_t *USBD_GetReportDescriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &Report_Descriptor[0]);
}

/*********************************************************************
 * @fn      USBD_GetHidDescriptor
 *
 * @brief   Gets the report descriptors according to the needed index
 *
 * @param   Length.
 *
 * @return  The address of the device descriptor.
 */
uint8_t *USBD_GetHidDescriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &Hid_Descriptor[0]);
}

/*********************************************************************
 * @fn      USBD_Get_Interface_Setting.
 *
 * @brief   test the interface and the alternate setting according to the
 *          supported one.
 *
 * @param   Interface - interface number.
 *          AlternateSetting - Alternate Setting number.
 *
 * @return  USB_UNSUPPORT or USB_SUCCESS.
 */
RESULT USBD_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;
  }
  else if (Interface > 1)
  {
    return USB_UNSUPPORT;
  }
	
  return USB_SUCCESS;
}

/*********************************************************************
 * @fn      USB_CDC_GetLineCoding.
 *
 * @brief   send the linecoding structure to the PC host.
 *
 * @param   Length
 *
 * @return  Inecoding structure base address.
 */
uint8_t *USB_CDC_GetLineCoding( uint16_t Length )
{
    if( Length == 0 )
    {
        pInformation->Ctrl_Info.Usb_wLength = 7;
        return( NULL );
    }
    return (uint8_t *)&LineCfg;
}

/*********************************************************************
 * @fn      USB_CDC_SetLineCoding.
 *
 * @brief   Set the linecoding structure fields.
 *
 * @param   Length
 *
 * @return  Inecoding structure base address.
 */
uint8_t *USB_CDC_SetLineCoding( uint16_t Length )
{
    if( Length == 0 )
    {
        pInformation->Ctrl_Info.Usb_wLength = 7;
        return( NULL );
    }
    return (uint8_t *)&LineCfg;
}


/*********************************************************************
 * @fn      USBD_Data_Setup
 *
 * @brief   handle the data class specific requests
 *
 * @param   Request Nb.
 *
 * @return  USB_UNSUPPORT or USB_SUCCESS.
 */
RESULT USBD_Data_Setup(uint8_t RequestNo)
{
  uint32_t Request_No;
  uint8_t *(*CopyRoutine)(uint16_t);
  uint8_t wValue1;
  Request_No = pInformation->USBbRequest;
  CopyRoutine = NULL;
  wValue1 = pInformation->USBwValue1;
  if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
  {
    if (wValue1 == HID_REPORT_DESCRIPTOR)
    {
      CopyRoutine = USBD_GetReportDescriptor;
    }
    else if (wValue1 == HID_DESCRIPTOR)
    {
      CopyRoutine = USBD_GetHidDescriptor;
    }
    if (CopyRoutine)
    {
      pInformation->Ctrl_Info.CopyData = CopyRoutine;
      (*CopyRoutine)(0);
    }
    else
    {
      return USB_UNSUPPORT;
    }

  }
  else if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
  {
    if (Request_No == CDC_GET_LINE_CODING)
    {
      CopyRoutine = &USB_CDC_GetLineCoding;
    }
    else if (Request_No == CDC_SET_LINE_CODING)
    {
      CopyRoutine = &USB_CDC_SetLineCoding;
    }
    else
    {
      return USB_UNSUPPORT;
    }
  }
  if (CopyRoutine)
  {
    pInformation->Ctrl_Info.CopyData = CopyRoutine;
    pInformation->Ctrl_Info.Usb_wOffset = 0;
    (*CopyRoutine)( 0 );
  }
  else
  {
    return( USB_UNSUPPORT );
  }
  return USB_SUCCESS;
}

/*********************************************************************
 * @fn      USBD_NoData_Setup.
 *
 * @brief   handle the no data class specific requests.
 *
 * @param   Request Nb.
 *
 * @return  USB_UNSUPPORT or USB_SUCCESS.
 */
RESULT USBD_NoData_Setup(uint8_t RequestNo)
{      
  uint32_t Request_No = pInformation->USBbRequest;

  if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
  {
    if (Request_No == CDC_SET_LINE_CTLSTE)
    {

    }
    else if (Request_No == CDC_SEND_BREAK)
    {

    }
    else
    {
      return USB_UNSUPPORT;
    }    
  }             
  return USB_SUCCESS;
}
