#ifndef _BOARD_H_
#define _BOARD_H_

#define UART_CTRL_ENABLE  0
#define UART_CTRL         0

// UnivID Device settings

#define UIO_MAX_DATA_LEN     1024
#define UIO_MPRAM_SIZE       4096

// UnivIO Generic Device settings

#define UIO_PINS_PER_PORT  64

#if defined(ESP32_C3)
  #define UIO_PIN_COUNT      24
  #define UIO_HW_ID       "ESP32-C3"
#elif defined(ESP32_S3)  
  #define UIO_PIN_COUNT      48  
  #define UIO_HW_ID       "ESP32-S3"
#elif defined(ESP32_S2)  
  #define UIO_PIN_COUNT      48  
  #define UIO_HW_ID       "ESP32-S2"
#else
  #define UIO_PIN_COUNT      48  
  #define UIO_HW_ID       "ESP32"
#endif

#define UIOMCU_ADC_COUNT    1


#endif // ndef _BOARD_H_