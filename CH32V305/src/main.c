#include "ch32v30x.h"
#include "ch32v30x_usb.h"
#include "ch32v30x_usbhs_device.h"

#include "DAP.h"
#include "vcom_serial.h"
#include "hid_transfer.h"


void main(void)
{
    DAP_Setup();

    VCOM_Init();

    USBHS_RCC_Init();
    USBHS_Device_Init(ENABLE);

    while(1)
    {
        usbd_hid_process();

        VCOM_TransferData();
    }
}
