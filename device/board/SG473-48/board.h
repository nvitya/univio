/*
 *  file:     board.h
 *  brief:    UnivIO definitions for the 48 pin STM32FG473 MCU
 *  created:  2022-01-30
 *  authors:  nvitya
*/

#ifndef BOARD_H_
#define BOARD_H_

#define BOARD_NAME "STM32G473 48-pin"
#define MCU_STM32G473CB
#ifndef EXTERNAL_XTAL_HZ
  #define EXTERNAL_XTAL_HZ   24000000
#endif

#ifndef MCU_CLOCK_SPEED
  // warning the 144 MHz is not synthetisable with the 25 MHz Crystal !
  //    #define MCU_CLOCK_SPEED   144000000   // for proper 2.4 MHz SPI, it must be divisible by 2.4 MHz
  #define MCU_CLOCK_SPEED   120000000   // for proper 2.4 MHz SPI, it must be divisible by 2.4 MHz
#endif


#define USB_ENABLE        1
#define UART_CTRL_ENABLE  0
#define USB_UART_ENABLE   1
#define HAS_SPI_FLASH     0
#define SPI_SELF_FLASHING 0

#define UART_CTRL         0

#define UIO_UART_COUNT      1
#define UIO_CAN_COUNT       1
#define UIO_I2C_COUNT       1
#define UIO_SPI_COUNT       1
#define UIO_SPIFLASH_COUNT  1

#define UIO_SPI_CS_COUNT    1

// UnivID Device settings

#define UIO_FW_ID   "GenIO-SG473-48"
#define UIO_FW_VER         ((0 << 24) | (5 << 16) | 1)
#define UIO_MEM_SIZE       0x8000 // for OBJ#0002

#define UIO_MAX_DATA_LEN     1024
#define UIO_MPRAM_SIZE       (32*1024)

// UnivIO Generic Device settings

#define UIO_PINS_PER_PORT  16
#define UIO_PIN_COUNT      48

#define UIOMCU_ADC_COUNT    2


// DMA Allocation

#define DMACH_UART_TX     0x101
#define DMACH_UART_RX     0x102
#define DMACH_SPI_TX      0x103
#define DMACH_SPI_RX      0x104
#define DMACH_I2C_TX      0x105
#define DMACH_I2C_RX      0x106

#define DMACH_ADC1        0x201
#define DMACH_ADC2        0x202

#endif /* BOARD_H_ */
