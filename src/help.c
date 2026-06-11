#include "help.h"
#include "softuart.h"

void PrintHelp(void){
  print("LIN_adapter, ver.1.2. 2022-02-02\r\n", 34);
  print("Only for receive LIN packets, transmit doesn't work\n\r", 53); 
}
void print(char* pText, uint8_t size){
  for(uint8_t i = 0; i < size; ++i){
    while(test_status(transmit_data_reg_empty) != transmit_data_reg_empty) {asm("nop");}
    uart_send(pText[i]);
  }
}
