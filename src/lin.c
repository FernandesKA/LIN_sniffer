/* Includes ------------------------------------------------------------------*/

#include "lin.h"
#include "uart.h"
#include "init.h"
/******************************************************************************/
//User variables
uint16_t u16BreakLength;
LIN_HEADER currentHeader;
struct LIN_Header header;
struct LIN_Response response;
enum LIN_Size Lin_size = bytes_2;
uint8_t countReceived = 0x00;
enum LIN_VER LIN_ver = LIN_2_1;
/******************************************************************************/
/******************************************************************************/
//User function definitions
/******************************************************************************/
//This function receive PID frame
uint8_t GetPID(uint8_t u8PIDReceive)
{
  uint8_t u8PID = u8PIDReceive & 0x3F;
  return u8PID;
}
/******************************************************************************/
//This is IRQ handler for detecting break on the LIN mode
#ifdef EXTI_PORTD_IRQ
//External Interrupt PORTD Interrupt routine.
INTERRUPT_HANDLER(EXTI_PORTD_IRQHandler, 6)
{
    if ((GPIOD->IDR & (1 << 6)) == (1 << 6))
    { //Rising edge
      uint16_t u16TempTime = 0x0000;
      u16TempTime = (TIM1->CNTRH) << 8;
      u16TempTime |= TIM1->CNTRL;
      uint16_t LowTime = 1200;
      uint16_t HighTime = 1450;
      //Check break length
      if(BAUD_LIN == 9600){
        LowTime = 1100;
        HighTime = 1450;
      }
      else{
        LowTime = 600;
        HighTime = 725;
      }
      //Detect break
      if (u16TempTime > LowTime && u16TempTime < HighTime)
      {
        currentHeader = wait_synch;
        SetSynchMode();
#ifdef DEBUG
        UART1->DR = 0x00U;
#endif
      }
    }
    else
    { //Falling edge
      TIM1->CNTRH = 0x00;
      TIM1->CNTRL = 0x00;
    }
}
/******************************************************************************/
//This function send LIN response
void send_response(struct LIN_SEND* lin, bool isMaster){
  if(isMaster){
    UART1->CR3|=UART1_CR3_LINEN;
    UART1->CR2|=UART1_CR2_SBK;
    while((UART1->SR & UART1_SR_TXE) != UART1_SR_TXE) {asm("nop");}
    UART1->DR = 0x55;
    while((UART1->SR & UART1_SR_TXE) != UART1_SR_TXE) {asm("nop");}
    UART1->DR = lin->PID;
    for(uint8_t i = 0; i < lin ->SIZE; ++i){
      while((UART1->SR & UART1_SR_TXE) != UART1_SR_TXE) {asm("nop");}
      UART1->DR = lin->Data[i];
    }
    while((UART1->SR & UART1_SR_TXE) != UART1_SR_TXE) {asm("nop");}
    UART1->DR = lin ->CRC;
    //TODO Add CRC field
    UART1->CR3&=~UART1_CR3_LINEN;
    UART1->CR2&=~UART1_CR2_SBK;
  }
  else{
  for(uint8_t i = 0; i < lin->SIZE; ++i){//Send data 
    while((UART1->SR & UART1_SR_TXE) != UART1_SR_TXE) {asm("nop");}
    UART1->DR = lin->Data[i];
  }
  while((UART1->SR & UART1_SR_TXE) != UART1_SR_TXE) {asm("nop");}
  UART1->DR = lin->CRC;
  }
}
/******************************************************************************/
void UpdateBAUD_EEPROM(uint16_t BAUD_VAL, uint32_t address){
  FLASH_Unlock(FLASH_MEMTYPE_DATA);
  FLASH_EraseByte(address);
  FLASH_EraseByte(address+1);
  FLASH_ProgramByte(address, (uint8_t) ((BAUD_VAL >> 8)&0xFF));//Write MSB
  FLASH_ProgramByte(address + 1, (uint8_t) (BAUD_VAL & 0xFF));//Write LSB
  FLASH_Lock(FLASH_MEMTYPE_DATA);
}
#endif
