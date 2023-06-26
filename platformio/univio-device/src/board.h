#ifndef _BOARD_H_
#define _BOARD_H_

#define BOARD_NAME   "STM32F103 48-pin"

#define USB_ENABLE        0
#define UART_CTRL_ENABLE  0

#define UART_CTRL         0

// UnivID Device settings

#define UIO_HW_ID       "ESP32-32"

#define UIO_MAX_DATA_LEN     1024
#define UIO_MPRAM_SIZE       4096

// UnivIO Generic Device settings

#define UIO_PINS_PER_PORT  64
#define UIO_PIN_COUNT      24

#define UIOMCU_ADC_COUNT    1


#endif // ndef _BOARD_H_