/*
 *  file:     board.h
 *  brief:    UnivIO definitions for the 48 pin STM32F103 MCU
 *  created:  2021-12-08
 *  authors:  nvitya
*/

#ifndef BOARD_H_
#define BOARD_H_

#define BOARD_NAME "STM32F103 48-pin"
#define MCU_STM32F103C8
#define EXTERNAL_XTAL_HZ    8000000
#define MCU_CLOCK_SPEED    72000000   // for proper 2.4 MHz SPI, it must be divisible by 2.4 MHz

#define USB_ENABLE        1
#define UART_CTRL_ENABLE  0
#define USB_UART_ENABLE   1
#define HAS_SPI_FLASH     0
#define SPI_SELF_FLASHING 0

#define UART_CTRL         0

// UnivID Device settings

#define UIO_FW_ID       "SF103-48"

#define UIO_MAX_DATA_LEN     1024
#define UIO_MPRAM_SIZE       4096

// UnivIO Generic Device settings

#define UIO_PINS_PER_PORT  16
#define UIO_PIN_COUNT      48

#define UIOMCU_ADC_COUNT    1

#endif /* BOARD_H_ */
