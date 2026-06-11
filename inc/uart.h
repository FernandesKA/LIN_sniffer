#ifndef _uart_h_
#define _uart_h_
//Includes
#include <stm8s.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stm8s_conf.h"
#include "lin.h"
//Defines
#define TxBufSize 8U
#define RxBufSize 8U
//Users functions
void UART_Init(void);
bool UART_Receive_IT(uint8_t *u8Buf, uint8_t u8Size);
bool UART_Transmit_IT(uint8_t *u8Buf, uint8_t u8Size);
void UART_AbortReceive(void);
//Users variasbles for bufers
extern uint8_t u8TxData[TxBufSize];
extern uint8_t u8RxData[RxBufSize];
//Variables
extern uint8_t u8RxCnt, u8TxCnt;
extern uint8_t u8RxSize, u8TxSize;
extern bool bTransmitted, bReceived;
uint8_t GetCRC(LIN_Header* header, LIN_Response* response);
#endif