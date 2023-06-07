/*
 *  file:     board.h
 *  brief:    UnivIO definitions for the 64 pin ATSAME5x MCU
 *  created:  2022-01-22
 *  authors:  nvitya
*/

#ifndef BOARD_H_
#define BOARD_H_

#define BOARD_NAME "ATSAME5x 64-pin"
#define MCU_ATSAME51J18
#define EXTERNAL_XTAL_HZ   12000000
#define MCU_CLOCK_SPEED    96000000   // for proper 1 MBit/s it must be dividible with 16 MHz
                                      // for proper 2.4 MHz SPI, it must be divisible by 2.4 MHz

#define USB_ENABLE        1
#define UART_CTRL_ENABLE  0
#define USB_UART_ENABLE   1
#define HAS_SPI_FLASH     0
#define SPI_SELF_FLASHING 0

#define UART_CTRL         0

// UnivID Device settings

#define UIO_FW_ID   "GenIO-AE5X-64"
#define UIO_FW_VER    ((0 << 24) | (5 << 16) | 0)
#define UIO_MEM_SIZE       0x8000 // for OBJ#0002

#define UIO_MAX_DATA_LEN     4096 // this MCU has lot of memory
#define UIO_MPRAM_SIZE       8192

#define TRACE_BUFFER_SIZE    4096

// UnivIO Generic Device settings

#define UIO_PINS_PER_PORT  32
#define UIO_PIN_COUNT      64

#define UIOMCU_ADC_COUNT    1

// DMA Allocation

#define DMACH_UART_TX       2
#define DMACH_UART_RX       3
#define DMACH_SPI_TX        4
#define DMACH_SPI_RX        5
#define DMACH_I2C_TX        6
#define DMACH_I2C_RX        7
#define DMACH_ADC_DATA     12  // keep the default
#define DMACH_ADC_SQ       13  // keep the default

#endif /* BOARD_H_ */
