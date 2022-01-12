## DAPLink
DAPLink (CMSIS-DAP) based on Nuvoton M480, supports SWD and CDC.

In the C/C++ page of Keil Option Window:
* Define `DAP_FW_V1`: generate CMSIS-DAP V1 firmware, using HID transport protocol.
* Do not define `DAP_FW_V1`: Generate CMSIS-DAP V2 firmware, using WINUSB transfer protocol.

### Pin map
|  FUNC   | Pin   |
|  :----  | :---- |
| SWD_CLK | PA.1  |
| SWD_DIO | PA.0  |
| SWD_RST | PF.4  |
| CDC_TXD | PB.1  |
| CDC_RXD | PB.0  |
| LED_SWD | PA.2  |
| LED_TXD | PC.1  |
| LED_RXD | PC.0  |
