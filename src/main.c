#include "init.h"
#include "help.h"
#include "communication.h"

//#define DEBUG
static void SysInit(void);
static void BAUD_Restore(uint16_t *BAUD_VAR, uint32_t address);
static void MODE_Restore(enum LIN_VER *lin, uint32_t address);
static void MODE_Update(enum LIN_VER *lin, uint32_t address);

uint16_t BAUD_LIN;
static FIFO sw_receive;
uint32_t BAUD_ADDR = 0x00004000;
uint32_t MODE_ADDR = 0x00004010;

enum FSM_REC
{
  w_mode
};
static FSM_REC fsm_receive = w_mode;
struct LIN_SEND LIN_Send;

void main(void)
{
  SysInit();
  BAUD_Restore(&BAUD_LIN, BAUD_ADDR);
  MODE_Restore(&LIN_ver, MODE_ADDR);
  PrintHelp();
  UART1->CR2 &= ~UART1_CR2_TEN;
  if (BAUD_LIN == 9600)
  {
    print("Baud 9600\r\n", 11);
  }
  else
  {
    print("Baud 19200\r\n", 12);
  }

  if (LIN_ver == LIN_1_3)
  {
    print("Classic CRC\r\n", 14);
  }
  else
  {
    print("Enhanced CRC\r\n", 15);
  }

  currentHeader = wait_break;
  sw_receive.isEmpty = true;
  bool commandRecieved = false;
  asm("rim");
  for (;;)
  {
    if (test_status(receive_buffer_full) == receive_buffer_full)
    {
      uint8_t u8Data;
      uart_read(&u8Data);
      Push(&sw_receive, u8Data);
    }

    if (!sw_receive.isEmpty)
    {
      switch (fsm_receive)
      {
      case w_mode:
        static uint8_t data = 0xFF;
        data = Pull(&sw_receive);
        if (data == 0x0C && !commandRecieved)
        {
#ifdef DEBUG
          print("Command mode\n\r", 14);
#endif
          commandRecieved = true;
          break;
        }
        else if (data != 0x0C && !commandRecieved)
        {
          print("Not valid command\r\n", 19);
        }
        if (commandRecieved)
        {
          if (data == 0x10)
          {
            LIN_ver = LIN_1_3;
            MODE_Update(&LIN_ver, MODE_ADDR);
            print("Classic CRC\r\n", 13);
          }
          else if (data == 0x15)
          {
            LIN_ver = LIN_2_1;
            MODE_Update(&LIN_ver, MODE_ADDR);
            print("Enhanced CRC\r\n", 14);
          }
          else if (data == 0x20)
          {
            BAUD_LIN = 9600;
            UpdateBAUD_EEPROM(BAUD_LIN, BAUD_ADDR);
            UART_HW_Config();
            print("Baud 9600\r\n", 11);
          }
          else if (data == 0x25)
          {
            BAUD_LIN = 19200;
            UpdateBAUD_EEPROM(BAUD_LIN, BAUD_ADDR);
            UART_HW_Config();
            print("Baud 19200\r\n", 12);
          }
          else if (data == 0x30)
          {
            if (BAUD_LIN == 9600)
            {
              print("Baud 9600\r\n", 11);
            }
            else
            {
              print("Baud 19200\r\n", 12);
            }
            if (LIN_ver == LIN_1_3)
            {
              print("Classic CRC\r\n", 14);
            }
            else
            {
              print("Enhanced CRC\r\n", 15);
            }
            SysInit();
          }
          else
          {
            fsm_receive = w_mode;
            print("Not valid command\r\n", 19);
            ResetState();
          }
        }
        commandRecieved = false;
        break;
      }
    }
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(u8 *file, u32 line)
{
  return;
}
#endif

static void SysInit(void)
{
  Clk_Config();
  UART_SW_Config();
  UART_HW_Config();
  Tim1_Config();
  Tim4_Config();
  GPIO_Config();
  SetExtIRQ();
  asm("rim");
}

void ResetState(void)
{
  while (!sw_receive.isEmpty)
  {
    Pull(&sw_receive);
  }
  fsm_receive = w_mode;
  LIN_Send.CRC = 0x00;
  LIN_Send.PID = 0x00;
  LIN_Send.SIZE = bytes_2;
  LIN_Send.Mode = UNDEF;
}

static void BAUD_Restore(uint16_t *BAUD_VAR, uint32_t address)
{
  *BAUD_VAR = (FLASH_ReadByte(address) << 8);
  *BAUD_VAR |= FLASH_ReadByte(address + 1);
  BAUD_LIN = *BAUD_VAR;
  if (*BAUD_VAR != 9600 && *BAUD_VAR != 19200)
  {
    *BAUD_VAR = 19200;
    UpdateBAUD_EEPROM(BAUD_LIN, address);
  }
  UART_HW_Config();
}

static void MODE_Restore(enum LIN_VER *lin, uint32_t address)
{
  uint8_t value = FLASH_ReadByte(address);
  *lin = (LIN_VER)value;
}

static void MODE_Update(enum LIN_VER *lin, uint32_t address)
{
  uint8_t writeVal = (uint8_t)*lin;
  FLASH_Unlock(FLASH_MEMTYPE_DATA);
  FLASH_ProgramByte(address, writeVal);
  FLASH_Lock(FLASH_MEMTYPE_DATA);
}
