/*
 *  file:     board.h
 *  brief:    UnivIO definitions for the RP2040 (Raspberry Pi Pico)
 *  created:  2022-06-11
 *  authors:  nvitya
*/

#ifndef BOARD_H_
#define BOARD_H_

#define BOARD_NAME "RP2040 56-pin"
#define MCU_RP2040
#define EXTERNAL_XTAL_HZ   12000000
#define MCU_CLOCK_SPEED   132000000   // for proper 2.4 MHz SPI, it must be divisible by 2.4 MHz

#define HAS_SPI_FLASH     1
#define SPI_SELF_FLASHING 1

#define USB_ENABLE        1
#define UART_CTRL_ENABLE  0
#define USB_UART_ENABLE   1

#define UART_CTRL         0

// UnivID Device settings

#define UIO_FW_ID   "GenIO-RP2040"
#define UIO_FW_VER    ((0 << 24) | (5 << 16) | 0)
#define UIO_MEM_SIZE       0x8000 // for OBJ#0002

#define UIO_MAX_DATA_LEN     4096
#define UIO_MPRAM_SIZE       8192

#define TRACE_BUFFER_SIZE    4096

// UnivIO Generic Device settings

#define UIO_PINS_PER_PORT  32
#define UIO_PIN_COUNT      32

#define UIOMCU_ADC_COUNT    1

// DMA Channel allocation

#define DMACH_USBUART_TX    0
#define DMACH_USBUART_RX    1  // helper allocated here !
#define DMACH_SPI_TX        2
#define DMACH_SPI_RX        3
#define DMACH_I2C_TX        4
#define DMACH_I2C_RX        5
#define DMACH_ADC           6  // helper allocated here !
#define DMACH_QSPI          7
// do not assign channels 10-11, they are used as helper channels for the circular buffers !

#endif /* BOARD_H_ */
