# DAPLink
DAPLink (CMSIS-DAP) porting to Artery AT32F425, WCH CH32V203, WCH CH32V305, Nuvoton M482 and Nuvoton M484.

## DAPLink-AT32F425
DAPLink (CMSIS-DAP) based on Artery AT32F425 (support crystal-less USB), supports SWD and CDC.

### Pin map
|  FUNC    | Pin   |
|  :----   | :---- |
| SWD_CLK  | PA.1  |
| SWD_DIO  | PA.0  |
| SWD_RST  | PA.4  |
| CDC_TXD  | PA.2  |
| CDC_RXD  | PA.3  |

Sch & PCB: [https://oshwhub.com/xivn1987/daplink](https://oshwhub.com/xivn1987/daplink)

## DAPLink-CH32V203
DAPLink (CMSIS-DAP) based on WCH CH32V203 (support crystal-less USB), supports SWD and CDC.

### Pin map
|  FUNC    | Pin   |
|  :----   | :---- |
| SWD_CLK  | PA.1  |
| SWD_DIO  | PA.0  |
| SWD_RST  | PA.4  |
| CDC_TXD  | PA.2  |
| CDC_RXD  | PA.3  |

Sch & PCB: [https://oshwhub.com/xivn1987/daplink](https://oshwhub.com/xivn1987/daplink)

## DAPLink-CH32V305
DAPLink (CMSIS-DAP) based on WCH CH32V305 (High Speed USB with on-chip PHY), supports SWD and CDC.

### Pin map
|  FUNC    | Pin   |
|  :----   | :---- |
| SWD_CLK  | PB.13 |
| SWD_DIO  | PB.12 |
| SWD_RST  | PC.8  |
| CDC_TXD  | PB.10 |
| CDC_RXD  | PB.11 |

Sch & PCB: [https://oshwhub.com/xivn1987/DAPLink-HS](https://oshwhub.com/xivn1987/DAPLink-HS)

## DAPLink-M482
DAPLink (CMSIS-DAP) based on Nuvoton M482, supports SWD and CDC.

In the C/C++ page of Keil Option Window:
* Define `DAP_FW_V1`: generate CMSIS-DAP V1 firmware, using HID transport protocol.
* Do not define `DAP_FW_V1`: Generate CMSIS-DAP V2 firmware, using WINUSB transfer protocol.

### Pin map
|  FUNC    | Pin   |
|  :----   | :---- |
| SWD_CLK  | PA.1  |
| SWD_DIO  | PA.0  |
| SWD_RST  | PF.4  |
| CDC_TXD  | PB.1  |
| CDC_RXD  | PB.0  |

## DAPLink-M484
DAPLink (CMSIS-DAP) based on Nuvoton M484 (High Speed USB with on-chip PHY), supports SWD and CDC.

### Pin map
|  FUNC    | Pin   |
|  :----   | :---- |
| SWD_CLK  | PA.1  |
| SWD_DIO  | PA.0  |
| SWD_RST  | PF.4  |
| CDC_TXD  | PB.1  |
| CDC_RXD  | PB.0  |
