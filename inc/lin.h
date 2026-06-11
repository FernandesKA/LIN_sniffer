#ifndef _lin_h_
#define _lin_h_
//User includes
#include "stm8s_conf.h"
#include "fifo.h"
/******************************************************************************/
//Enum definitions
enum LIN_VER{
  LIN_1_3, 
  LIN_2_1
};
extern enum LIN_VER LIN_ver;
enum LIN_MODE{
  MASTER,
  SLAVE, 
  SLAVE_ZD, //Send packet with zeros delay
  UNDEF
};

enum LIN_Size
{
  bytes_2 = 2,
  bytes_4 = 4,
  bytes_8 = 8
};
//Structure definition
struct LIN_SEND{
  uint8_t PID;
  enum LIN_Size SIZE;
  uint8_t Data[8U];
  uint8_t CRC;
  LIN_MODE Mode;
};
//Structure declaration
extern struct LIN_SEND LIN_Send;
//FSM for frame header
enum LIN_HEADER
{
  wait_break,
  wait_synch,
  wait_pid,
  wait_data
};

struct LIN_Header
{
  uint8_t synch;
  uint8_t pid;
  enum LIN_Size size;
};

struct LIN_Response
{
  uint8_t data[8U];
  uint8_t CRC;
};
/******************************************************************************/
//Variable declarations
extern struct LIN_Header header;
extern struct LIN_Response response;
extern enum LIN_Size Lin_size;
extern uint8_t countReceived;
extern LIN_HEADER currentHeader;
extern uint16_t u16BreakLength;
/******************************************************************************/
//User function definition
bool BreakDetection(void);
bool GetSynch(uint8_t data);
uint8_t GetPID(uint8_t u8PIDReceive);
void GetNextState(LIN_HEADER *current);
void send_response(struct LIN_SEND* lin, bool isMaster);
void ResetState(void);
void UpdateBAUD_EEPROM(uint16_t BAUD_VAL, uint32_t address);
#endif