// uio_gendev.cpp (64-pin ATSAM4S)

#include "hwintflash.h"
#include "hwspi.h"
#include "uio_device.h"


#define SPI_CS_PIN      11 // A11

#define UIOFUNC_SPI     0x0100
#define UIOFUNC_I2C     0x0200
#define UIOFUNC_UART    0x0400
#define UIOFUNC_CLKOUT  0x0800

typedef struct
{
  uint16_t   flags; // bit0: 0 0 = disabled
  uint32_t   adc;  // 0 = none
  uint32_t   dac;
  uint32_t   pwm;
//
} TPinInfo;

#define NOTEXISTING  0, 0, 0, 0
#define RESERVED     0, 0, 0, 0
#define UIOPWM(devnum, outnum, altfunc) (0x80000000 | (devnum << 0) | (outnum << 4) | (altfunc << 8))
#define UIOADC(adcnum, innum)           (0x80000000 | (adcnum << 8) | (innum << 0))
#define UIODAC(outnum)                  (0x80000000 | (outnum << 0))

const TPinInfo g_pininfo[UIO_PIN_COUNT] =
{
/*  0   A0 */ { 1,   0, 0, UIOPWM(0, 0, 0) },
/*  1   A1 */ { 1,   0, 0, UIOPWM(0, 1, 0) }, // On board LED !
/*  2   A2 */ { 1,   0, 0, UIOPWM(0, 2, 0) },
/*  3   A3 */ { 1 | UIOFUNC_I2C,  0, 0, 0 },  // TWD0  / SDA
/*  4   A4 */ { 1 | UIOFUNC_I2C,  0, 0, 0 },  // TWCK0 / SCL
/*  5   A5 */ { 1,   0, 0, 0 },
/*  6   A6 */ { RESERVED },                   // trace out (USART0_TX)
/*  7   A7 */ { 1,   0, 0, UIOPWM(0, 3, 0) },

/*  8   A8 */ { 1,   0, 0, 0 },
/*  9   A9 */ { 1 | UIOFUNC_UART,  0, 0, 0 }, // UART0_RX
/* 10  A10 */ { 1 | UIOFUNC_UART,  0, 0, 0 }, // UART0_TX
/* 11  A11 */ { 1 | UIOFUNC_SPI,   0, 0, UIOPWM(0, 0, 1) },  // SPI_CS (GPIO mode)
/* 12  A12 */ { 1 | UIOFUNC_SPI,   0, 0, UIOPWM(0, 1, 1) },  // SPI_MISO
/* 13  A13 */ { 1 | UIOFUNC_SPI,   0, 0, UIOPWM(0, 2, 1) },  // SPI_MOSI
/* 14  A14 */ { 1 | UIOFUNC_SPI,   0, 0, UIOPWM(0, 3, 1) },  // SPI_CLK
/* 15  A15 */ { 1,   0, 0, UIOPWM(0, 3, 2) },

/* 16  A16 */ { 1,   0, 0, UIOPWM(0, 2, 2) },
/* 17  A17 */ { 1,   UIOADC(0, 0), 0, 0 },
/* 18  A18 */ { 1,   UIOADC(0, 1), 0, 0 },
/* 19  A19 */ { 1,   UIOADC(0, 2), 0, 0 },
/* 20  A20 */ { 1,   UIOADC(0, 3), 0, 0 },
/* 21  A21 */ { 1,   UIOADC(0, 8), 0, 0 },
/* 22  A22 */ { 1,   UIOADC(0, 9), 0, 0 },
/* 23  A23 */ { 1,   0, 0, UIOPWM(0, 0, 1) },

/* 24  A24 */ { 1,   0, 0, UIOPWM(0, 1, 1) },
/* 25  A25 */ { 1,   0, 0, UIOPWM(0, 2, 1) },
/* 26  A26 */ { 1,   0, 0, 0 },
/* 27  A27 */ { 1,   0, 0, 0 },
/* 28  A28 */ { 1,   0, 0, 0 },
/* 29  A29 */ { 1,   0, 0, 0 },
/* 30  A30 */ { 1,   0, 0, 0 },
/* 31  A31 */ { 1 | UIOFUNC_CLKOUT,   0, 0, 0 }, // PCK2 = 12 MHz output (AF_B)

/* 32   B0 */ { 1,   UIOADC(0, 4), 0, 0 },
/* 33   B1 */ { 1,   UIOADC(0, 5), 0, 0 },
/* 34   B2 */ { 1,   UIOADC(0, 6), 0, 0 },
/* 35   B3 */ { 1,   UIOADC(0, 7), 0, 0 },
/* 36   B4 */ { RESERVED }, // TDI
/* 37   B5 */ { RESERVED }, // TDO
/* 38   B6 */ { RESERVED }, // SWDIO
/* 39   B7 */ { RESERVED }, // SWCLK

/* 40   B8 */ { RESERVED }, // XOUT
/* 41   B9 */ { RESERVED }, // XIN
/* 42  B10 */ { RESERVED }, // USB DDM
/* 43  B11 */ { RESERVED }, // USB DDP
/* 44  B12 */ { RESERVED }, // ERASE
/* 45  B13 */ { 1,   0, UIODAC(0), 0 },
/* 46  B14 */ { 1,   0, UIODAC(1), 0 },
/* 47  B15 */ { NOTEXISTING },
};

//------------------------------------------------------------------------------------------------------
// TUnivioDevice Implementations
//------------------------------------------------------------------------------------------------------

bool TUioGenDevImpl::InitBoard()
{
  unsigned n;

  #if UART_CTRL_ENABLE
    #if 0 == UART_CTRL
      hwpinctrl.PinSetup(PORTNUM_A,  9,  PINCFG_INPUT  | PINCFG_AF_A | PINCFG_SPEED_FAST);  // UART0_RX
      hwpinctrl.PinSetup(PORTNUM_A, 10,  PINCFG_OUTPUT | PINCFG_AF_A | PINCFG_SPEED_FAST);  // UART0_TX

      g_uartctrl.uart.baudrate = UNIVIO_UART_BAUDRATE;
      g_uartctrl.uart.Init(0);
      g_uartctrl.uart.PdmaInit(true,  &g_uartctrl.dma_tx);
      g_uartctrl.uart.PdmaInit(false, &g_uartctrl.dma_rx);

    #else
      #error "unknown control uart definition"
    #endif
  #endif

  // place the Non-Volatile data to the end of the flash, importantly into BANKB for concurrent access
  nvsaddr_setup  = hwintflash.start_address + hwintflash.bytesize - 16384;
  nvsaddr_nvdata = hwintflash.start_address + hwintflash.bytesize - 8192;
  nvs_sector_size = hwintflash.EraseSize(nvsaddr_setup);

  // Setup A31 to PCK2
  PMC->PMC_PCK[2] = (0
    | (1  <<  0)  // CSS(3): 1 = MAIN clock
    | (0  <<  4)  // PRES(3):  0 = div 1
  );

  // Init The ADC
  g_adc[0].Init(0, 0x3FF); // enable all 10 channels

  // SPI initialization
#if UIO_SPI_COUNT > 0
  g_spi[0].manualcspin = &g_pins[SPI_CS_PIN];
  g_spi[0].Init(0);
  g_spi[0].PdmaInit(true,  &g_dma_spi_tx[0]);
  g_spi[0].PdmaInit(false, &g_dma_spi_rx[0]);
#endif

  // I2C initialization
#if UIO_I2C_COUNT > 0
  g_i2c[0].Init(0);
  g_i2c[0].PdmaInit(true,  &g_dma_i2c_tx[0]);
  g_i2c[0].PdmaInit(false, &g_dma_i2c_rx[0]);
#endif

  // UART Initialization
#if UIO_UART_COUNT > 0
  g_uart[0].Init(0);
  g_uart[0].PdmaInit(true,  &g_dma_uart_tx[0]);
  g_uart[0].PdmaInit(false, &g_dma_uart_rx[0]);
#endif

  // Other Pin inits is not necessary here, because all pins will be initialized later to passive
  // before the config loading happens

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
  else if (UIO_PINTYPE_DAC_OUT == pintype)
  {
    return (pinfo->dac != 0);
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

  if (!pwm->Init(pinfo->pwm & 0xF, (pinfo->pwm >> 4) & 0xF))
  {
    return;
  }

  pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_0 | (((pinfo->pwm >> 8) & 0xF) << PINCFG_AF_SHIFT);

}

void TUioGenDevImpl::SetupSpi(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  if (SPI_CS_PIN == pcf->pinid)
  {
    pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_GPIO_INIT_1;
    ppin->Assign(ppin->portnum, ppin->pinnum, false);   // reassign required because it was maybe inverted
  }
  else
  {
    pcf->hwpinflags = PINCFG_AF_A | PINCFG_SPEED_FAST;
  }
}

void TUioGenDevImpl::SetupI2c(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  pcf->hwpinflags = PINCFG_AF_A | PINCFG_PULLUP;
}

void TUioGenDevImpl::SetupUart(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  if (pcf->pinid == 10)  // UART0_TX
  {
    pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_A | PINCFG_SPEED_FAST;
  }
  else
  {
    pcf->hwpinflags = PINCFG_INPUT  | PINCFG_AF_A | PINCFG_PULLUP | PINCFG_SPEED_FAST;
  }
}

void TUioGenDevImpl::SetupClockOut(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_B;
}


bool TUioGenDevImpl::LoadBuiltinConfig(uint8_t anum)
{
  return false;
}
