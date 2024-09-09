/*
 *  file:     board.h
 *  brief:    UnivIO definitions for the 64 pin ATSAM4S MCU
 *  created:  2021-11-18
 *  authors:  nvitya
*/

#ifndef BOARD_H_
#define BOARD_H_

#define BOARD_NAME "ATSAM4S 64-pin"
#define MCU_ATSAM4S2B
#define EXTERNAL_XTAL_HZ   12000000
#define MCU_CLOCK_SPEED    96000000   // for proper 1 MBit/s it must be dividible with 16 MHz
                                      // for proper 2.4 MHz SPI, it must be divisible by 2.4 MHz

#define USB_ENABLE        1
#define UART_CTRL_ENABLE  0
#define USB_UART_ENABLE   1
#define HAS_SPI_FLASH     0
#define SPI_SELF_FLASHING 0

#define UART_CTRL         0

#define UIO_UART_COUNT      1
#define UIO_I2C_COUNT       1
#define UIO_SPI_COUNT       1

// UnivID Device settings

#define UIO_FW_ID   "GenIO-A4S-64"
#define UIO_FW_VER    ((0 << 24) | (6 << 16) | 0)
#define UIO_MEM_SIZE       0x8000 // for OBJ#0002

#define UIO_MAX_DATA_LEN     4096 // this MCU has lot of memory
#define UIO_MPRAM_SIZE       8192

#define TRACE_BUFFER_SIZE    4096

// UnivIO Device settings

#define UIO_PINS_PER_PORT  32
#define UIO_PIN_COUNT      48

#define UIOMCU_ADC_COUNT    1

#endif /* BOARD_H_ */
