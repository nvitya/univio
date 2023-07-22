/*
 *  file:     uio_gendev_impl.cpp
 *  brief:    ESP32 Specific implementation
 *  created:  2023-06-26
 *  authors:  nvitya
*/

#include "udoslave.h"
#include "uio_device.h"

#define UIOFUNC_SPI     0x0100
#define UIOFUNC_I2C     0x0200
#define UIOFUNC_UART    0x0400

typedef struct
{
  uint16_t   flags; // bit0: 0 0 = disabled
  uint32_t   adc;
  uint32_t   pwm;
//
} TPinInfo;

#define NOTEXISTING  0, 0, 0
#define RESERVED     0, 0, 0
#define UIOADC(adcnum, innum)     (0x80000000 | (adcnum << 8) | (innum << 0))
#define UIOPWM(chnum)             (0x80000000 | (chnum << 0))

#if defined(ARDUINO_LOLIN_C3_MINI)
  #define PIN_LED        7
  #define PIN_IRQ        0
  #define PIN_IRQ_TASK   1
#elif defined(ARDUINO_ESP32C3_DEV)
  #define PIN_LED       18
  #define PIN_IRQ        0
  #define PIN_IRQ_TASK   1
  #define NO_FLOAT       1
#elif defined(ARDUINO_LOLIN_S2_MINI)
  #define PIN_LED       15
  #define PIN_IRQ        3
  #define PIN_IRQ_TASK   5
#elif defined(ARDUINO_ESP32S3_DEV)
  #define PIN_LED        2
  #define PIN_IRQ       16
  #define PIN_IRQ_TASK  17
#else
  #error "unknown board!"
#endif


const TPinInfo g_pininfo[UIO_PIN_COUNT] =
{
/*  0 */ { 1,   UIOADC(1, 0), 0 },  // does not work always
/*  1 */ { 1,   UIOADC(1, 1), 0 },
/*  2 */ { 1,   UIOADC(1, 2), 0 },
/*  3 */ { 1,   UIOADC(1, 3), 0 },
/*  4 */ { 1,   UIOADC(1, 4), 0 },
/*  5 */ { 1,   0, UIOPWM(0) },
/*  6 */ { 1,   0, UIOPWM(1) },
/*  7 */ { 1,   0, UIOPWM(2) },

/*  8 */ { 1,   0, UIOPWM(3) },
/*  9 */ { 1,   0, UIOPWM(4) },
/* 10 */ { 1,   0, UIOPWM(5) },
/* 11 */ { RESERVED },  // SPIFL-VDD
/* 12 */ { RESERVED },  // SPIFL-HD
/* 13 */ { RESERVED },  // SPIFL-WP
/* 14 */ { RESERVED },  // SPIFL-CS0
/* 15 */ { RESERVED },  // SPIFL-CLK

/* 16 */ { RESERVED },  // SPIFL-D
/* 17 */ { RESERVED },  // SPIFL-Q
/* 18 */ { 1,   0, 0 }, // USB-D- !!!
/* 19 */ { 1,   0, 0 }, // USB-D+ !!!
/* 20 */ { RESERVED },  // UART-RX
/* 21 */ { RESERVED },  // UART-TX
/* 22 */ { RESERVED },  // -
/* 23 */ { RESERVED }   // -

};

//------------------------------------------------------------------------------------------------------
// TUnivioDevice Implementations
//------------------------------------------------------------------------------------------------------

bool TUioGenDevImpl::InitBoard()
{
  unsigned n;
  uint32_t tmp;

  return true;
}

bool TUioGenDevImpl::PinFuncAvailable(TPinCfg * pcf)
{
  const TPinInfo * pinfo = &g_pininfo[pcf->pinid];

  uint8_t pintype = pcf->pintype;

  if (0 == (pinfo->flags & 1)) // ignored pin ?
  {
    return false;
  }
  else if (UIO_PINTYPE_ADC_IN == pintype)
  {
    return (pinfo->adc != 0);
  }
  else if (UIO_PINTYPE_PWM_OUT == pintype)
  {
    return (pinfo->pwm != 0);
  }
  else if (UIO_PINTYPE_SPI == pintype)
  {
    return (0 != (pinfo->flags & UIOFUNC_SPI));
  }
  else if (UIO_PINTYPE_I2C == pintype)
  {
    return (0 != (pinfo->flags & UIOFUNC_I2C));
  }
  else if (UIO_PINTYPE_UART == pintype)
  {
    return (0 != (pinfo->flags & UIOFUNC_UART));
  }
  else
  {
    return true;
  }
}

void TUioGenDevImpl::SetupAdc(TPinCfg * pcf)
{
  const TPinInfo * pinfo = &g_pininfo[pcf->pinid];

  adc_channel[pcf->unitnum] = 0x80 | (pinfo->adc & 15);
  pcf->hwpinflags = PINCFG_INPUT | PINCFG_ANALOGUE;
}

void TUioGenDevImpl::SetupDac(TPinCfg * pcf)
{
  // TODO: implement
  pcf->hwpinflags = PINCFG_INPUT | PINCFG_PULLUP;
}

void TUioGenDevImpl::SetupPwm(TPinCfg * pcf)
{
  const TPinInfo *  pinfo = &g_pininfo[pcf->pinid];
  THwPwmChannel *   pwm = &g_pwm[pcf->unitnum];

  pcf->hwpinflags = IGNORE_PINFLAGS;  // instructs the caller to not change the pin settings

  unsigned chnum = (pinfo->pwm & 0xF);
  if (!pwm->Init(pcf->pinid, chnum))
  {
    return;
  }
}

void TUioGenDevImpl::SetupSpi(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];
}

void TUioGenDevImpl::SetupI2c(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];
}

void TUioGenDevImpl::SetupUart(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];
}

void TUioGenDevImpl::SetupClockOut(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];
}

bool TUioGenDevImpl::LoadBuiltinConfig(uint8_t anum)
{
  return false;
}
