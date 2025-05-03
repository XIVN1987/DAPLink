# DAPLink
DAPLink (CMSIS-DAP) porting to Artery AT32F425, WCH CH32V203 and WCH CH32V305.

## DAPLink-AT32F425
DAPLink (CMSIS-DAP) based on Artery AT32F425 (support crystal-less USB), supports SWD, JTAG and CDC.

On C/C++ page of Keil Option Window:
* Define `DAP_FW_V1`: generate CMSIS-DAP V1 firmware, using HID transport protocol.
* Do not define `DAP_FW_V1`: generate CMSIS-DAP V2 firmware, using WINUSB transfer protocol.

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

On Properties for DAPLink -> C/C++ Build -> Settings -> C Compiler -> Preprocessor page:
* Define `DAP_FW_V1`: generate CMSIS-DAP V1 firmware, using HID transport protocol.
* Do not define `DAP_FW_V1`: generate CMSIS-DAP V2 firmware, using WINUSB transfer protocol.

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

On Properties for DAPLink -> C/C++ Build -> Settings -> C Compiler -> Preprocessor page:
* Define `DAP_FW_V1`: generate CMSIS-DAP V1 firmware, using HID transport protocol.
* Do not define `DAP_FW_V1`: generate CMSIS-DAP V2 firmware, using WINUSB transfer protocol.

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

## Speed Test
**test command**
``` shell
pyocd erase -t stm32f411ce -f 5mhz --chip
pyocd flash -t stm32f411ce -f 5mhz random_512k.bin
```

**test result**
| probe           | pyocd print                                                                                                   |
| ----            | ----                                                                                                          |
| AT32F425 HID    | Erased 524288 bytes (8 sectors), programmed 524288 bytes (512 pages), skipped 0 bytes (0 pages) at 17.24 kB/s |
| AT32F425 WINUSB | Erased 524288 bytes (8 sectors), programmed 524288 bytes (512 pages), skipped 0 bytes (0 pages) at 38.34 kB/s |
| CH32V203 HID    | Erased 524288 bytes (8 sectors), programmed 524288 bytes (512 pages), skipped 0 bytes (0 pages) at 17.26 kB/s |
| CH32V203 WINUSB | Erased 524288 bytes (8 sectors), programmed 524288 bytes (512 pages), skipped 0 bytes (0 pages) at 38.16 kB/s |
| CH32V305 HID    | Erased 524288 bytes (8 sectors), programmed 524288 bytes (512 pages), skipped 0 bytes (0 pages) at 35.44 kB/s |
| CH32V305 WINUSB | Erased 524288 bytes (8 sectors), programmed 524288 bytes (512 pages), skipped 0 bytes (0 pages) at 39.40 kB/s |
