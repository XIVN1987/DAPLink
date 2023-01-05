#ifndef __VCOM_SERIAL_H__
#define __VCOM_SERIAL_H__


#define RX_BUFF_SIZE    1024 	// RX buffer size
#define TX_BUFF_SIZE    1024 	// RX buffer size

typedef struct {
	uint8_t  rx_buff[RX_BUFF_SIZE];
	uint16_t rx_bytes;
	uint16_t rx_wrptr;
	uint16_t rx_rdptr;
	uint8_t  tx_buff[TX_BUFF_SIZE];
	uint16_t tx_bytes;
	uint16_t tx_wrptr;
	uint16_t tx_rdptr;
	
	uint16_t hw_flow;			// BIT0: DTR(Data Terminal Ready) , BIT1: RTS(Request To Send)
	
	uint8_t  in_buff[64];
	uint16_t in_bytes;
	uint16_t in_ready;
	uint8_t  out_buff[64];
	uint16_t out_bytes;
	uint16_t out_ready;
} VCOM;

extern volatile VCOM Vcom;


typedef struct __attribute__((packed)) {
    uint32_t u32DTERate;     	// Baud rate
    uint8_t  u8CharFormat;   	// stop bit:  0 - 1 Stop bit, 1 - 1.5 Stop bits, 2 - 2 Stop bits
    uint8_t  u8ParityType;   	// parity:    0 - None, 1 - Odd, 2 - Even, 3 - Mark, 4 - Space
    uint8_t  u8DataBits;     	// data bits: 5, 6, 7, 8, 16
} VCOM_LINE_CODING;

extern VCOM_LINE_CODING LineCfg;


void VCOM_Init(void);
void VCOM_TransferData(void);
void VCOM_LineCoding(VCOM_LINE_CODING * LineCfgx);


#endif // __VCOM_SERIAL_H__
