/*
 * uio_gendev_base.h
 *
 *  Created on: Nov 24, 2021
 *      Author: vitya
 */

#ifndef UNIVIO_UIO_GENDEV_BASE_H_
#define UNIVIO_UIO_GENDEV_BASE_H_

#include "hwpins.h"
#include "hwadc.h"
#include "hwpwm.h"
#include "hwspi.h"
#include "hwi2c.h"
#include "hwdma.h"
#include "hwintflash.h"

#include "usbfunc_cdc_uart.h"

#include "uio_dev_base.h"

// fix maximums
#define UIO_PWM_COUNT       8
#define UIO_ADC_COUNT      16
#define UIO_DAC_COUNT       8
#define UIO_DOUT_COUNT     32
#define UIO_DIN_COUNT      32
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

#define UIO_PINFLAG_DIN_PULLUP       (0x0000 << 16)
#define UIO_PINFLAG_DIN_PULLDN       (0x0001 << 16)
#define UIO_PINFLAG_DIN_FLOAT        (0x0002 << 16)

#define UIO_PINFLAG_DOUT_INVERT      (0x0001 << 16)
#define UIO_PINFLAG_DOUT_OD          (0x0002 << 16)
#define UIO_PINFLAG_PWM_INVERT       (0x0001 << 16)
#define UIO_PINFLAG_LEDBLP_INVERT    (0x0001 << 16)

#define UIO_I2C_CMD_WRITE            (1 << 8)

#define UIOCFG_SIGNATURE   0xAA66CF55
#define UIOMODE_SIGNATURE  0x5530DEAA

// error codes
#define UIOERR_PINTYPE              0x5001  // invalid pin type
#define UIOERR_FUNC_NOT_AVAIL       0x5002  // the selected function is not available for this pin
#define UIOERR_UNIT_ALREADY_IN_USE  0x5003  // the selected function is not available for this pin
#define UIOERR_UNIT_INIT            0x5004  // unit initialization
#define UIOERR_UNIT_PARAMS          0x5005  // wrong parameters


typedef struct // configuration storage header
{
  uint32_t          signature;
  uint32_t          length;
  uint32_t          _reserved;
  uint32_t          checksum;
//
} TUioCfgHead;

typedef struct
{
  uint32_t          pinsetup[UIO_PIN_COUNT];

  uint32_t          dv_douts;
  uint16_t          dv_dac[UIO_DAC_COUNT];
  uint16_t          dv_pwm[UIO_PWM_COUNT];
  uint32_t          dv_ledblp[UIO_LEDBLP_COUNT];

  uint32_t          pwm_freq[UIO_PWM_COUNT];
//
} TUioDeviceCfg;

typedef struct
{
  uint32_t          signature;

  uint32_t          base_length;
  uint32_t          base_csum;

  uint32_t          cfg_length;
  uint32_t          cfg_csum;

  TUioDevBaseCfg    basecfg;
  TUioDeviceCfg     cfg;
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

class TUioGenDevBase: public TUioDevBase
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
  TUioDeviceCfg     cfg;

  uint8_t *         mpram = nullptr;

  THwSpi *          spi = nullptr;
  uint16_t          spi_rx_offs = 0;
  uint16_t          spi_tx_offs = 0;
  uint32_t          spi_speed = 1000000;
  uint16_t          spi_trlen = 0;
  uint8_t           spi_status = 0;

  THwI2c *          i2c = nullptr;
  uint16_t          i2c_data_offs = 0;
  uint32_t          i2c_speed = 100000;
  uint32_t          i2c_eaddr = 0;
  uint32_t          i2c_cmd = 0;
  uint16_t          i2c_trlen = 0;
  uint16_t          i2c_result = 0;

  THwUart *         uart = nullptr;
  bool              uart_active = false;

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

  void              Run();
  void              ClearConfig();
  void              ResetConfig();
  virtual void      ConfigurePins(bool active);
  void              SetPwmDuty(uint8_t apwmnum, uint16_t aduty);
  void              SetDacOutput(uint8_t adacnum, uint16_t avalue);
  uint16_t          GetAdcValue(uint8_t adc_idx, uint16_t * rvalue);

  uint16_t          SpiStart();
  uint16_t          I2cStart();

public:  // base class mandatory implementations
  virtual bool      InitDevice();
  virtual bool      HandleDeviceRequest(TUnivioRequest * rq);
  virtual void      SaveSetup();
  virtual void      LoadSetup();
  virtual void      SetRunMode(uint8_t arunmode);

public: // board specific virtuals
  virtual bool      InitBoard() { return false; }
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

extern THwSpi           g_spi;
extern THwI2c           g_i2c;
extern THwUart          g_uart;

extern THwDmaChannel    g_dma_spi_tx;
extern THwDmaChannel    g_dma_spi_rx;
extern THwDmaChannel    g_dma_i2c_tx;
extern THwDmaChannel    g_dma_i2c_rx;
extern THwDmaChannel    g_dma_uart_tx;
extern THwDmaChannel    g_dma_uart_rx;

uint32_t uio_content_checksum(void * adataptr, uint32_t adatalen);

#endif /* UNIVIO_UIO_GENDEV_BASE_H_ */
