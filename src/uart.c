#include "uart.h"
#include "init.h"
#include "lin.h"
#include "communication.h"
#include "help.h"
//Function declaration
inline static void UART_RX_IRQ(uint8_t UART_DR);
inline static void UART_TX_IRQ(void);
//static inline void UART_Send(uint8_t data)
//{
//  while ((UART1->SR & UART1_SR_TXE) != UART1_SR_TXE)
//  {
//    asm("nop");
//  }
//  UART1->DR = data;
//}
//Variables
uint8_t u8RxCnt, u8TxCnt;
uint8_t u8RxSize, u8TxSize;
uint8_t u8RxData[RxBufSize];
uint8_t u8TxData[TxBufSize];
bool bTransmitted = false;
bool bReceived = false;

//UART initializations
void UART_Init(void)
{
  CLK->PCKENR1 |= CLK_PCKENR1_UART1; //ENABLE CLOCKING
  UART1->BRR1 = 0x68;
  UART1->BRR2 = 0x02;
  UART1->CR2 |= UART1_CR2_REN | UART1_CR2_TEN;
}
//This function configure recieve for IRQ RXNE
bool UART_Receive_IT(uint8_t *u8Buf, uint8_t u8Size)
{
  bReceived = false;
  u8RxCnt = 0x00;
  u8RxSize = u8Size;
  UART1->CR2 |= UART1_CR2_REN;
  UART1->CR2 |= UART1_CR2_RIEN;
  return true;
}
//This function configure transmit for IRQ TXCIE
bool UART_Transmit_IT(uint8_t *u8Buf, uint8_t u8Size)
{
  bTransmitted = false;
  u8TxCnt = 0x00;
  u8TxSize = u8Size;
  UART1->CR2 |= UART1_CR2_TIEN; //Enable IRQ for complete send
  return true;
}
//This function clear RXE buff, and disable it
void UART_AbortReceive(void)
{
  asm("nop");
}
//TX IRQ handler
inline static void UART_TX_IRQ(void)
{
  if (u8TxCnt < u8TxSize)
  {
    UART1->DR = u8TxData[u8TxCnt++];
  }
  else
  {
    bTransmitted = true;
    u8TxCnt = 0x00;
    u8TxSize = 0x00;
    UART1->CR2 &= ~UART1_CR2_TIEN;
  }
}
//RX IRQ handler
inline static void UART_RX_IRQ(uint8_t UART_DR)
{
  volatile uint8_t u8SynchField;
  volatile uint8_t u8PIDField;
  switch (currentHeader)
  {
  case wait_break: //It's mistake cases
    //UART1->CR2 &= ~UART1_CR2_RIEN;
    asm("nop");
    return;
    break;

  case wait_synch:
    u8SynchField = UART_DR;
    if (u8SynchField == 0x55U)
    {
      currentHeader = wait_pid;
      header.synch = UART_DR;
#ifdef DEBUG
      UART_Send(u8SynchField);
#endif
      return;
    }
    else
    {
      currentHeader = wait_break;
#ifdef DEBUG
      UART_Send(0x64);
#endif
      SetExtIRQ();
    }
    break;

  case wait_pid:
    //u8PIDField = UART_DR;
    header.pid = GetPID(UART_DR);
#ifdef DEBUG
    UART_Send(u8PIDField);
#endif
    if (header.pid < 0x20)
    { //2 bytes of data
      Lin_size = bytes_2;
    }
    else if (header.pid < 0x30)
    { //4 bytes of data
      Lin_size = bytes_4;
    }
    else
    { //8 bytes of data
      Lin_size = bytes_8;
    }
    header.size = Lin_size;
//    if (SendLIN)
//    { //If we send packet from RS232 -> LIN into the slave mode
//      if (GetPID(LIN_Send.PID) == GetPID(header.pid))
//      {
//        send_response(&LIN_Send, false);
//        SendLIN = false;
//        ResetState();
//        break;
//      }
//      else
//      {
//        asm("nop"); //Other PID received
//      }
//    }
    currentHeader = wait_data;
    countReceived = 0x00U;
    response.CRC = 0x00U;
    //Without parity check!!
    break;

  case wait_data:
        if (countReceived < header.size)
        {
    //      response.CRC ^= UART_DR;
          response.data[countReceived++] = UART_DR;
        }
    else if (countReceived == header.size)
    {
      //TODO: we calculate CRC into the PC
      currentHeader = wait_break;
      response.CRC = GetCRC(&header, &response);
      if(UART_DR == response.CRC){
        asm("sim");
        Reflect_LIN(header, response, true);
        asm("rim");
      }
      else{
        asm("sim");
        Reflect_LIN(header, response, false);
        asm("rim");
      }
      SetExtIRQ();
    }
    break;

  default:
    SetExtIRQ();
    break;
  }
}
//UART1 TX Interrupt routine.
INTERRUPT_HANDLER(UART1_TX_IRQHandler, 17)
{
  UART_TX_IRQ();
}

//UART1 RX Interrupt routine.
INTERRUPT_HANDLER(UART1_RX_IRQHandler, 18)
{
  uint8_t u8Data = UART1->DR;
  UART_RX_IRQ(u8Data);
}
//This function return sum for CRC
static inline void GetSum(uint8_t* cSum, uint8_t* nData){
  if(*cSum + *nData > 0xFF){
    ++(*cSum);
    *cSum += *nData;
  }
  else{
    *cSum += *nData;
  }
}
//This function calculate CRC
uint8_t GetCRC(LIN_Header* header, LIN_Response* response){
uint8_t sum = 0x00;
if(LIN_ver == LIN_2_1){
GetSum(&sum, &header ->pid);
}
if(header ->size == bytes_2){
  for(uint8_t i = 0; i < 2; ++i){
    GetSum(&sum, &response -> data[i]);
  }
}
else if(header -> size == bytes_4){
  for(uint8_t i = 0; i < 4; ++i){
    GetSum(&sum, &response -> data[i]);    
  } 
}
else if(header -> size == bytes_8){
  for(uint8_t i = 0; i < 8; ++i){
    GetSum(&sum, &response -> data[i]);    
  }  
}
return sum ^ 0xFF;
}

