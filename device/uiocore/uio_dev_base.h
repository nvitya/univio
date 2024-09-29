/* -----------------------------------------------------------------------------
 * This file is a part of the UNIVIO project: https://github.com/nvitya/univio
 * Copyright (c) 2022 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     uio_gendev_base.h
 *  brief:    UNIVIO device target independent parts
 *  date:     2022-02-13
 *  authors:  nvitya
*/

#ifndef _UIO_GENDEV_BASE_H
#define _UIO_GENDEV_BASE_H

#include "hwpins.h"
#include "hwadc.h"
#include "hwpwm.h"
#include "hwspi.h"
#include "hwi2c.h"
#include "hwdma.h"

#include "udo.h"
#include "udoslave.h"

#include "usbfunc_cdc_uart.h"
#include "simple_partable.h"

#include "uio_common.h"

#include "uio_i2c_control.h"
#include "uio_spi_control.h"
#include "uio_can_control.h"
#include "uio_spiflash_control.h"

#define UIO_DEVICE_TYPE_ID   "UIO-V3"   // Index 0x0100

// fix maximums
#define UIO_PWM_COUNT       8
#define UIO_ADC_COUNT      32
#define UIO_DAC_COUNT       8
#define UIO_DOUT_COUNT     64
#define UIO_DIN_COUNT      64
#define UIO_LEDBLP_COUNT   16

#define UIO_ADC_ERROR_VALUE     0x0000

#define UIO_PINTYPE_PASSIVE          0 // default configuration
#define UIO_PINTYPE_DIG_IN           1 // pullup by default
#define UIO_PINTYPE_DIG_OUT          2
#define UIO_PINTYPE_ADC_IN           3
#define UIO_PINTYPE_DAC_OUT          4
#define UIO_PINTYPE_PWM_OUT          5
#define UIO_PINTYPE_LEDBLP           6
#define UIO_PINTYPE_SPI              7
#define UIO_PINTYPE_I2C              8
#define UIO_PINTYPE_UART             9
#define UIO_PINTYPE_CLKOUT          10
#define UIO_PINTYPE_CAN             11

#define UIO_PINFLAG_DIN_PULLUP       (0x0000 << 16)
#define UIO_PINFLAG_DIN_PULLDN       (0x0001 << 16)
#define UIO_PINFLAG_DIN_FLOAT        (0x0002 << 16)

#define UIO_PINFLAG_DOUT_INVERT      (0x0001 << 16)
#define UIO_PINFLAG_DOUT_OD          (0x0002 << 16)
#define UIO_PINFLAG_DOUT_HIZ         (0x0010 << 16)
#define UIO_PINFLAG_DOUT_HIZ_PD      (0x0020 << 16)
#define UIO_PINFLAG_DOUT_HIZ_FLOAT   (0x0040 << 16)
#define UIO_PINFLAG_PWM_INVERT       (0x0001 << 16)
#define UIO_PINFLAG_LEDBLP_INVERT    (0x0001 << 16)

#define UIO_I2C_CMD_WRITE            (1 << 0)

#define UIOCFG_V2_SIGNATURE   0xA566CF5A

// error codes
#define UIOERR_PINTYPE              0x5001  // invalid pin type
#define UIOERR_FUNC_NOT_AVAIL       0x5002  // the selected function is not available for this pin
#define UIOERR_UNIT_ALREADY_IN_USE  0x5003  // the selected function is not available for this pin
#define UIOERR_UNIT_INIT            0x5004  // unit initialization
#define UIOERR_UNIT_PARAMS          0x5005  // wrong parameters
#define UIOERR_RUN_MODE             0x5101  // config mode required
#define UIOERR_UNITSEL              0x5102  // the referenced unit is not existing

#define UIO_INFOIDX_CBITS      0
#define UIO_INFOIDX_DIN        1
#define UIO_INFOIDX_DOUT       2
#define UIO_INFOIDX_ADC        3
#define UIO_INFOIDX_DAC        4
#define UIO_INFOIDX_PWM        5
#define UIO_INFOIDX_LEDBLP     6
#define UIO_INFOIDX_DIN_32     7
#define UIO_INFOIDX_DOUT_32    8
#define UIO_INFOIDX_HIZ_0      9
#define UIO_INFOIDX_HIZ_32    10

#define UIO_INFO_COUNT        11

#define UIO_INFOCBIT_CLKOUT (1  << 0)
#define UIO_INFOCBIT_UART   (1  << 1)
#define UIO_INFOCBIT_SPI    (1  << 2)
#define UIO_INFOCBIT_I2C    (1  << 3)
#define UIO_INFOCBIT_CAN    (1  << 4)

#define UIO_INFOCBIT_UARTB  (16 << 1)
#define UIO_INFOCBIT_SPIB   (16 << 2)
#define UIO_INFOCBIT_I2CB   (16 << 3)
#define UIO_INFOCBIT_CANB   (16 << 4)

typedef struct
{
  uint32_t          signature;
  uint32_t          length;
  uint32_t          _reserved_008;
  uint32_t          checksum;

  uint16_t          usb_vendor_id;
  uint16_t          usb_product_id;
  uint32_t          _reserved_020;
  char              manufacturer[32];
  char              device_id[32];
  char              serial_number[32];
  uint32_t          _reserved_120;
  uint32_t          _reserved_124;

  uint32_t          pinsetup[UIO_PIN_COUNT];

  uint32_t          dv_douts;
  uint16_t          dv_dac[UIO_DAC_COUNT];
  uint16_t          dv_pwm[UIO_PWM_COUNT];
  uint32_t          dv_ledblp[UIO_LEDBLP_COUNT];

  uint32_t          pwm_freq[UIO_PWM_COUNT];
//
} TUioCfgStb;

typedef struct
{
  uint8_t           pinid;
  uint8_t           pintype;
  uint8_t           unitnum;
  uint16_t          flags;
  uint32_t          pincfg;
  uint32_t          hwpinflags;
//
} TPinCfg;

class TUioDevBase : public TClass
{
public: // internal state
  uint8_t           blp_idx = 0;
  uint32_t          blp_mask = 1;
  unsigned          last_blp_time = 0;
  unsigned          blp_bit_clocks = 0;

public: // NVS info
  uint32_t          nvsaddr_setup = 0;
  uint32_t          nvsaddr_nvdata = 0;
  uint32_t          nvs_sector_size = 0;

public:
  uint8_t           runmode = 0;  // 0 = CONFIG mode, 1 = RUN mode

  bool              initialized = false;

  TUioCfgStb        cfg;

  uint8_t *         mpram = nullptr;

  THwUart *         uart[UIO_UART_COUNT];
  bool              uart_active[UIO_UART_COUNT];

  // inputs
  TGpioPin *        dig_in[UIO_DIN_COUNT] = {0};
  uint8_t           adc_channel[UIO_ADC_COUNT] = {0};

  // outputs
  uint32_t          dout_value = 0;
  uint16_t          dac_value[UIO_DAC_COUNT] = {0};
  uint16_t          pwm_value[UIO_PWM_COUNT] = {0};
  uint32_t          ledblp_value[UIO_LEDBLP_COUNT] = {0};

  TGpioPin *        dig_out[UIO_DOUT_COUNT] = {0};
  TGpioPin *        ledblp[UIO_LEDBLP_COUNT] = {0};
  THwPwmChannel *   pwmch[UIO_PWM_COUNT] = {0};

  uint32_t          cfginfo[UIO_INFO_COUNT]; // bits signalize configured units

  virtual           ~TUioDevBase() { }

  bool              Init();
  void              Run();
  void              ClearConfig();
  void              ResetConfig();
  virtual void      ConfigurePins(bool active);
  void              SetPwmDuty(uint8_t apwmnum, uint16_t aduty);
  void              SetDacOutput(uint8_t adacnum, uint16_t avalue);
  virtual uint16_t  GetAdcValue(uint8_t adc_idx, uint16_t * rvalue);
  virtual uint16_t  GetAdcValueF32(uint8_t adc_idx, float * rvalue);


public:  // base class mandatory implementations
  virtual bool      InitDevice();
  virtual void      SaveSetup();
  virtual void      LoadSetup();
  virtual void      SetRunMode(uint8_t arunmode);

public: // board specific virtuals
  virtual bool      InitBoard() { return false; }
  virtual void      RunBoard() { }
  virtual uint16_t  PinSetup(uint8_t pinid, uint32_t pincfg, bool active);
  virtual bool      PinFuncAvailable(TPinCfg * pcf) { return false; }
  virtual void      SetupAdc(TPinCfg * pcf) { }
  virtual void      SetupDac(TPinCfg * pcf) { }
  virtual void      SetupPwm(TPinCfg * pcf) { }
  virtual void      SetupSpi(TPinCfg * pcf) { }
  virtual void      SetupI2c(TPinCfg * pcf) { }
  virtual void      SetupUart(TPinCfg * pcf) { }
  virtual void      SetupClockOut(TPinCfg * pcf) { }

  virtual bool      LoadBuiltinConfig(uint8_t anum) { return false; }

};

extern uint8_t          g_mpram[UIO_MPRAM_SIZE];

extern THwAdc           g_adc[UIOMCU_ADC_COUNT];
extern THwPwmChannel    g_pwm[UIO_PWM_COUNT];
extern TGpioPin         g_pins[UIO_PIN_COUNT];

extern THwUart          g_uart[UIO_UART_COUNT];

extern THwDmaChannel    g_dma_spi_tx[UIO_SPI_COUNT];
extern THwDmaChannel    g_dma_spi_rx[UIO_SPI_COUNT];
extern THwDmaChannel    g_dma_i2c_tx[UIO_I2C_COUNT];
extern THwDmaChannel    g_dma_i2c_rx[UIO_I2C_COUNT];
extern THwDmaChannel    g_dma_uart_tx[UIO_UART_COUNT];
extern THwDmaChannel    g_dma_uart_rx[UIO_UART_COUNT];

uint32_t uio_content_checksum(void * adataptr, uint32_t adatalen);

#endif
