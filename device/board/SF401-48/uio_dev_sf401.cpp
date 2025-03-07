/*
 *  file:     uio_gendev_sf401.cpp
 *  brief:    MCU Specific implementation for the 48 pin STM32F401
 *  created:  2022-01-30
 *  authors:  nvitya
*/

#include "hwintflash.h"
#include "hwspi.h"
#include "uio_device.h"
#include "clockcnt.h"

#define SPI_CS_PIN      28 // B12

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
#define UIODAC(dacnum, outnum)          (0x80000000 | (dacnum << 8) | (outnum << 0))

const TPinInfo g_pininfo[UIO_PIN_COUNT] =
{
/*  0   A0 */ { 1,   UIOADC(1, 0), 0, UIOPWM(2, 1, 1) },
/*  1   A1 */ { 1,   UIOADC(1, 1), 0, UIOPWM(2, 2, 1) },
/*  2   A2 */ { 1,   UIOADC(1, 2), 0, UIOPWM(2, 3, 1) },
/*  3   A3 */ { 1,   UIOADC(1, 3), 0, UIOPWM(2, 4, 1) },
/*  4   A4 */ { 1,   UIOADC(1, 4), 0, 0 },
/*  5   A5 */ { 1,   UIOADC(1, 5), 0, 0 },
/*  6   A6 */ { 1,   UIOADC(1, 6), 0, UIOPWM(3, 1, 2) },
/*  7   A7 */ { 1,   UIOADC(1, 7), 0, UIOPWM(3, 2, 2) },

/*  8   A8 */ { 1 | UIOFUNC_CLKOUT, 0, 0, UIOPWM(3, 1, 2) },  // 25 MHz Output
/*  9   A9 */ { RESERVED },  // TRACE_OUT = USART1_TX
/* 10  A10 */ { 1,   0, 0, UIOPWM(1, 3, 1) },
/* 11  A11 */ { RESERVED },  // USB D-
/* 12  A12 */ { RESERVED },  // USB D+
/* 13  A13 */ { RESERVED },  // SWDIO
/* 14  A14 */ { RESERVED },  // SWDCLK
/* 15  A15 */ { 1,   0, 0, 0 },  // JTAG_TDI !

/* 16   B0 */ { 1,   UIOADC(1, 8), 0, UIOPWM(3, 3, 2) },
/* 17   B1 */ { 1,   UIOADC(1, 9), 0, UIOPWM(3, 4, 2) },
/* 18   B2 */ { 1,   0, 0, 0 }, // BOOT-1!
/* 19   B3 */ { 1,   0, 0, 0 }, // JTAG_TDO / TRACESWO, remap required
/* 20   B4 */ { 1,   0, 0, 0 }, // JNTRST
/* 21   B5 */ { 1,   0, 0, UIOPWM(3, 2, 2) },
/* 22   B6 */ { 1 | UIOFUNC_I2C,   0, 0, UIOPWM(4, 1, 2) },
/* 23   B7 */ { 1 | UIOFUNC_I2C,   0, 0, UIOPWM(4, 2, 2) },

/* 24   B8 */ { 1,  0, 0, UIOPWM(4, 3, 2) },
/* 25   B9 */ { 1,  0, 0, UIOPWM(4, 4, 2) },
/* 26  B10 */ { 1,  0, 0, UIOPWM(2, 3, 2) },
/* 27  B11 */ { 1,  0, 0, UIOPWM(2, 4, 2) },
/* 28  B12 */ { 1 | UIOFUNC_SPI,   0, 0, 0 },  // CS (GPIO)
/* 29  B13 */ { 1 | UIOFUNC_SPI,   0, 0, 0 },  // SPI2_SCK
/* 30  B14 */ { 1 | UIOFUNC_SPI,   0, 0, 0 },  // SPI2_MISO
/* 31  B15 */ { 1 | UIOFUNC_SPI,   0, 0, 0 },  // SPI2_MOSI

/* 32   C0 */ { NOTEXISTING },
/* 33   C1 */ { NOTEXISTING },
/* 34   C2 */ { NOTEXISTING },
/* 35   C3 */ { NOTEXISTING },
/* 36   C4 */ { NOTEXISTING },
/* 37   C5 */ { NOTEXISTING },
/* 38   C6 */ { NOTEXISTING },
/* 39   C7 */ { NOTEXISTING },

/* 40   C8 */ { NOTEXISTING },
/* 41   C9 */ { NOTEXISTING },
/* 42  C10 */ { NOTEXISTING },
/* 43  C11 */ { NOTEXISTING },
/* 44  C12 */ { NOTEXISTING },
/* 45  C13 */ { 1,   0, 0, 0 },  // on board led
/* 46  C14 */ { 1,   0, 0, 0 },  // OSC32 in, weak high drive!
/* 47  C15 */ { 1,   0, 0, 0 },  // OSC32 out, weak high drive!
};

//------------------------------------------------------------------------------------------------------
// TUnivioDevice Implementations
//------------------------------------------------------------------------------------------------------

bool TUioDevImpl::InitBoard()
{
  unsigned n;
  uint32_t tmp;

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

  // WARNING: the Firmware size is limited to 32k, use -Os (optimize to size)

  // the flash here is divided to bigger blocks
  nvsaddr_nvdata = 0x08008000; // 2x16k sectors
  nvs_sector_size = hwintflash.EraseSize(nvsaddr_nvdata);

  nvsaddr_setup  = 0x08010000; // 1x64k sector

  // SETUP RESERVED PINS / FUNCTIONS

  // Setup A8 to MCO (8 MHz)

  tmp = RCC->CFGR;
  tmp &= ~(RCC_CFGR_MCO1PRE | RCC_CFGR_MCO1);
  tmp |= (2 << RCC_CFGR_MCO1_Pos); // 2 = HSE
  RCC->CFGR = tmp;
  //hwpinctrl.PinSetup(PORTNUM_A,  8,  PINCFG_OUTPUT | PINCFG_AF_0);

  // Init The ADC
  g_adc[0].Init(1, 0xFFFF); // enable all 16 channels

  // SPI initialization
#if UIO_SPI_COUNT > 0
  g_spi[0].manualcspin = &g_pins[SPI_CS_PIN];
  g_spi[0].Init(2);
  g_dma_spi_tx[0].Init(1, 4, 0);
  g_dma_spi_rx[0].Init(1, 3, 0);
  g_spi[0].DmaAssign(true,  &g_dma_spi_tx[0]);
  g_spi[0].DmaAssign(false, &g_dma_spi_rx[0]);
#endif

  // I2C initialization
#if UIO_I2C_COUNT > 0
  g_i2c[0].Init(1);
  g_dma_i2c_tx[0].Init(1, 7, 1);
  g_dma_i2c_rx[0].Init(1, 0, 1);
  g_i2c[0].DmaAssign(true,  &g_dma_i2c_tx[0]);
  g_i2c[0].DmaAssign(false, &g_dma_i2c_rx[0]);
#endif

  // UART Initialization
#if UIO_UART_COUNT > 0
  g_uart[0].Init(3);
  g_dma_uart_tx[0].Init((DMACH_UART_TX >> 8), DMACH_UART_TX & 7, 29);
  g_dma_uart_rx[0].Init((DMACH_UART_RX >> 8), DMACH_UART_RX & 7, 28);
  g_uart[0].DmaAssign(true,  &g_dma_uart_tx[0]);
  g_uart[0].DmaAssign(false, &g_dma_uart_rx[0]);
#endif

  // USB PINS
  hwpinctrl.PinSetup(PORTNUM_A, 11, PINCFG_INPUT | PINCFG_AF_10 | PINCFG_SPEED_FAST);  // USB DM
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_INPUT | PINCFG_AF_10 | PINCFG_SPEED_FAST);  // USB DP

  // Other Pin inits is not necessary here, because all pins will be initialized later to passive
  // before the config loading happens

  return true;
}

bool TUioDevImpl::PinFuncAvailable(TPinCfg * pcf)
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

void TUioDevImpl::SetupAdc(TPinCfg * pcf)
{
  const TPinInfo * pinfo = &g_pininfo[pcf->pinid];

  adc_channel[pcf->unitnum] = 0x80 | (pinfo->adc & 15);
  pcf->hwpinflags = PINCFG_INPUT | PINCFG_ANALOGUE;
}

void TUioDevImpl::SetupDac(TPinCfg * pcf)
{
  // TODO: implement
  pcf->hwpinflags = PINCFG_ANALOGUE;
}

void TUioDevImpl::SetupPwm(TPinCfg * pcf)
{
  const TPinInfo *  pinfo = &g_pininfo[pcf->pinid];
  THwPwmChannel *   pwm = &g_pwm[pcf->unitnum];

  if (!pwm->Init(pinfo->pwm & 0xF, (pinfo->pwm >> 4) & 0xF, 0))
  {
    return;
  }

  pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_0 | (((pinfo->pwm >> 8) & 0xF) << PINCFG_AF_SHIFT);

}

void TUioDevImpl::SetupSpi(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  if (SPI_CS_PIN == pcf->pinid)
  {
    pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_GPIO_INIT_1;
    ppin->Assign(ppin->portnum, ppin->pinnum, false);   // reassign required because it was maybe inverted
  }
  else
  {
    pcf->hwpinflags = PINCFG_AF_5;
  }
}

void TUioDevImpl::SetupI2c(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  pcf->hwpinflags = PINCFG_AF_4;
}

void TUioDevImpl::SetupUart(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  if (pcf->pinid == 16 * PORTNUM_B + 10)  // USART3_TX
  {
    pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_7;
  }
  else
  {
    pcf->hwpinflags = PINCFG_INPUT  | PINCFG_AF_7 | PINCFG_PULLUP;
  }
}

void TUioDevImpl::SetupClockOut(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_0;
}

bool TUioDevImpl::LoadBuiltinConfig(uint8_t anum)
{
  return false;
}
