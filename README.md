# LIN Sniffer

Passive LIN bus monitor for **STM8S003**. Captures LIN frames and outputs decoded packets over RS232 in hex ASCII. Version 1.2 (2022-02-02).

## Hardware connections

| Pin  | Function                        |
|------|---------------------------------|
| PD2  | RS232 TX — PC interface (SW UART) |
| PD3  | RS232 RX — PC interface (SW UART) |
| PD5  | LIN TX (HW UART1)               |
| PD6  | LIN RX / break detection        |
| PB5  | Activity LED                    |

RS232 baud rate is fixed at **19200**. LIN baud rate is configurable (see below).

## Output format

Each received LIN frame is printed as space-separated uppercase hex bytes:

```
PP D0 D1 ... Dn CC
```

Where `PP` is the 6-bit frame ID, `D0..Dn` are data bytes, and `CC` is the CRC byte.  
If the calculated CRC does not match the received value, `# ` is inserted before `CC`.

Example (valid frame, ID=0x10, 2 data bytes):
```
10 A4 3B E2
```

## Commands

Send `0x0C` followed by a command byte to change settings. All settings are stored in EEPROM.

| Byte  | Action                      |
|-------|-----------------------------|
| `0x10`| Classic CRC mode (LIN 1.3)  |
| `0x15`| Enhanced CRC mode (LIN 2.1) |
| `0x20`| Set LIN baud to 9600        |
| `0x25`| Set LIN baud to 19200       |
| `0x30`| Print current settings      |

Default: **19200 baud**, **Enhanced CRC (LIN 2.1)**.

## Build

Toolchain: **IAR Embedded Workbench for STM8 v3.11**.  
Open `LIN.eww` and build the `Debug` configuration. The output hex is written to `Debug/Exe/LIN_conv.hex`.
