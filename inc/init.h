#ifndef _init_h_
#define _init_h_
//User includes
#include "stm8s_conf.h"
#include "stm8s_itc.h"
#include "stm8s_flash.h"
#include "stm8s_tim2.h"
#include "stm8s_gpio.h"
#include "lin.h"
#include "uart.h"
#include "fifo.h"
#include "softuart.h"
//User defines
#define F_CPU 16000000UL
#define BAUD_SW 19200
#define BAUD_HW 9600
#define LED_PORT GPIOB
#define LED_PIN (1 << 5)
//User function definitions
void Clk_Config(void);
void UART_SW_Config(void);
void UART_HW_Config(void);
void Tim1_Config(void);
void Tim4_Config(void);
void GPIO_Config(void);
void SetSynchMode(void);
void SetExtIRQ(void);
void GetIndicate(void);
//User variables
extern uint16_t BAUD_LIN;
extern uint32_t BAUD_ADDR;
void CRC8(uint8_t *CRC, uint8_t data, bool isEnd);
#endif