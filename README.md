# DAPLink
DAPLink (CMSIS-DAP) porting to Artery AT32F425, WCH CH32V203 and WCH CH32V305.

## DAPLink-AT32F425
DAPLink (CMSIS-DAP) based on Artery AT32F425 (support crystal-less USB), supports SWD, JTAG and CDC.

### Pin map
|  FUNC    | Pin   |
|  :----   | :---- |
| SWD_CLK  | PA.1  |
| SWD_DIO  | PA.0  |
| SWD_RST  | PA.4  |
| CDC_TXD  | PA.2  |
| CDC_RXD  | PA.3  |
| JTAG_TDI | PA.7  |
| JTAG_TDO | PA.6  |

Sch & PCB: [https://oshwhub.com/xivn1987/daplink](https://oshwhub.com/xivn1987/daplink)

## DAPLink-CH32V203
DAPLink (CMSIS-DAP) based on WCH CH32V203 (support crystal-less USB), supports SWD, JTAG and CDC.

### Pin map
|  FUNC    | Pin   |
|  :----   | :---- |
| SWD_CLK  | PA.1  |
| SWD_DIO  | PA.0  |
| SWD_RST  | PA.4  |
| CDC_TXD  | PA.2  |
| CDC_RXD  | PA.3  |
| JTAG_TDI | PA.7  |
| JTAG_TDO | PA.6  |

Sch & PCB: [https://oshwhub.com/xivn1987/daplink](https://oshwhub.com/xivn1987/daplink)

## DAPLink-CH32V305
DAPLink (CMSIS-DAP) based on WCH CH32V305 (High Speed USB with on-chip PHY), supports SWD, JTAG and CDC.

### Pin map
|  FUNC    | Pin   |
|  :----   | :---- |
| SWD_CLK  | PB.13 |
| SWD_DIO  | PB.12 |
| SWD_RST  | PC.8  |
| CDC_TXD  | PB.10 |
| CDC_RXD  | PB.11 |
| JTAG_TDI | PB.15 |
| JTAG_TDO | PB.14 |

Sch & PCB: [https://oshwhub.com/xivn1987/DAPLink-HS](https://oshwhub.com/xivn1987/DAPLink-HS)
