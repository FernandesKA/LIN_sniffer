# LIN_converter

This project is an RS232 -> LIN adapter. 
The device can work as Slave, Master, repeat packets from LIN to RS232.
The structure of the repeated packet is PID + Size + Data + CRC. 
o work in Master mode, you need to send a packet in the Synch + PID + Data + CRC format. 
To work in Slave mode, you need to send a packet in the format 0x00 + PID + Data + CRC, the device will send a response if it matches the PID on the line.

## Parameters: 
```
BAUDRATE 9600 for RS232, LIN.
//PORT definitions
PORTD3 -> UART_SW_RX
PORTD2 -> UART_SW_TX

PORTD5 -> LIN_TX
PORTD6 -> LIN_RX
```
