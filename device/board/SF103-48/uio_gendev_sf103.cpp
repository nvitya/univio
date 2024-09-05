/*
 *  file:     uio_gendev_sf103.cpp
 *  brief:    MCU Specific implementation for the 48 pin STM32F103
 *  created:  2021-12-08
 *  authors:  nvitya
*/

#include "hwintflash.h"
#include "hwspi.h"
#include "udoslave.h"
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

const TPinInfo g_pininfo[UIO_PIN_COUNT] =
{
/*  0   A0 */ { 1,   UIOADC(1, 0), 0, 0 },
/*  1   A1 */ { 1,   UIOADC(1, 1), 0, UIOPWM(2, 2, 0) },
/*  2   A2 */ { 1,   UIOADC(1, 2), 0, UIOPWM(2, 3, 0) },
/*  3   A3 */ { 1,   UIOADC(1, 3), 0, UIOPWM(2, 4, 0) },
/*  4   A4 */ { 1,   UIOADC(1, 4), 0, 0 },
/*  5   A5 */ { 1,   UIOADC(1, 5), 0, 0 },
/*  6   A6 */ { 1,   UIOADC(1, 6), 0, UIOPWM(3, 1, 0) },
/*  7   A7 */ { 1,   UIOADC(1, 7), 0, UIOPWM(3, 2, 0) },

/*  8   A8 */ { 1 | UIOFUNC_CLKOUT,   0, 0, 0 },  // 8 MHz Output, no other special output function possible
/*  9   A9 */ { RESERVED },  // TRACE_OUT = USART1_TX
/* 10  A10 */ { 1,   0, 0, 0 },
/* 11  A11 */ { RESERVED },  // USB D-
/* 12  A12 */ { RESERVED },  // USB D+
/* 13  A13 */ { RESERVED },  // SWDIO
/* 14  A14 */ { RESERVED },  // SWDCLK
/* 15  A15 */ { 1,   0, 0, 0 },  // JTAG_TDI !

/* 16   B0 */ { 1,   UIOADC(1, 8), 0, UIOPWM(3, 3, 0) },
/* 17   B1 */ { 1,   UIOADC(1, 9), 0, UIOPWM(3, 4, 0) },
/* 18   B2 */ { 1,   0, 0, 0 },
/* 19   B3 */ { 1,   0, 0, 0 }, // JTAG_TDO / TRACESWO, remap required
/* 20   B4 */ { 1,   0, 0, 0 }, // JNTRST, remap required
/* 21   B5 */ { 1,   0, 0, 0 },
/* 22   B6 */ { 1 | UIOFUNC_I2C,   0, 0, UIOPWM(4, 1, 0) },  // I2C1_SCL
/* 23   B7 */ { 1 | UIOFUNC_I2C,   0, 0, UIOPWM(4, 2, 0) },  // I2C1_SDA

/* 24   B8 */ { 1,   0, 0, UIOPWM(4, 3, 0) },
/* 25   B9 */ { 1,   0, 0, UIOPWM(4, 4, 0) },
/* 26  B10 */ { 1 | UIOFUNC_UART,  0, 0, 0 },  // UART_TX_OUT = USART3_TX
/* 27  B11 */ { 1 | UIOFUNC_UART,  0, 0, 0 },  // UART_RX_IN  = USART3_RX
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
/* 46  C14 */ { RESERVED },      // OSC32 in
/* 47  C15 */ { RESERVED },      // OSC32 out
};

//------------------------------------------------------------------------------------------------------
// TUnivioDevice Implementations
//------------------------------------------------------------------------------------------------------

bool TUioGenDevImpl::InitBoard()
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

  nvsaddr_setup  = 0x0800E000;
  nvsaddr_nvdata = 0x0800F000;
  nvs_sector_size = 1024;

  // SETUP RESERVED PINS / FUNCTIONS

  // Setup A8 to MCO (8 MHz)

  tmp = RCC->CFGR;
  tmp &= ~(RCC_CFGR_MCO);
  tmp |= RCC_CFGR_MCO_HSE;
  RCC->CFGR = tmp;
  //hwpinctrl.PinSetup(PORTNUM_A,  8,  PINCFG_OUTPUT | PINCFG_AF_0);

  // setup REMAP bits

  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

  // Turn off jtag for B3, B4
  tmp = AFIO->MAPR;
  tmp &= ~AFIO_MAPR_SWJ_CFG;
  tmp |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;  // JTAG disabled, SWD enabled
  tmp = AFIO->MAPR;


  // Init The ADC
  g_adc[0].Init(1, 0x3FF); // enable all 10 channels

  // SPI initialization
#if UIO_SPI_COUNT > 0
  g_spi[0].manualcspin = &g_pins[SPI_CS_PIN];
  g_spi[0].Init(2);
  g_dma_spi_tx[0].Init(1, 5, 1);
  g_dma_spi_rx[0].Init(1, 4, 1);
  g_spi[0].DmaAssign(true,  &g_dma_spi_tx[0]);
  g_spi[0].DmaAssign(false, &g_dma_spi_rx[0]);
#endif

  // I2C initialization
#if UIO_I2C_COUNT > 0
  g_i2c[0].Init(1);
  g_dma_i2c_tx[0].Init(1, 7, 1);
  g_dma_i2c_rx[0].Init(1, 6, 1);
  g_i2c[0].DmaAssign(true,  &g_dma_i2c_tx[0]);
  g_i2c[0].DmaAssign(false, &g_dma_i2c_rx[0]);
#endif

  // UART Initialization
#if UIO_UART_COUNT > 0
  //hwpinctrl.PinSetup(PORTNUM_B, 10,  PINCFG_OUTPUT | PINCFG_AF_0);    // USART3_TX
  //hwpinctrl.PinSetup(PORTNUM_B, 11,  PINCFG_INPUT  | PINCFG_PULLUP);  // USART3_RX, no AF here!
  g_uart[0].Init(3);
  g_dma_uart_tx[0].Init(1, 2, 1);
  g_dma_uart_rx[0].Init(1, 3, 1);
  g_uart[0].DmaAssign(true,  &g_dma_uart_tx[0]);
  g_uart[0].DmaAssign(false, &g_dma_uart_rx[0]);
#endif

  // Other Pin inits is not necessary here, because all pins will be initialized later to passive
  // before the config loading happens

  // USB RE-CONNECT

  // The Blue Pill has a fix external pull-up on the USB D+ = PA12, which always signalizes a connected device
  // in order to reinit the device upon restart we pull this down:

  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_OUTPUT | PINCFG_OPENDRAIN | PINCFG_GPIO_INIT_0);
  delay_us(10000); // 10 ms
  hwpinctrl.PinSetup(PORTNUM_A, 12, PINCFG_AF_0);

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

  if (!pwm->Init(pinfo->pwm & 0xF, (pinfo->pwm >> 4) & 0xF, 0))
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
    pcf->hwpinflags = PINCFG_AF_0;
  }
}

void TUioGenDevImpl::SetupI2c(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  pcf->hwpinflags = PINCFG_AF_0;
}

void TUioGenDevImpl::SetupUart(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  if (pcf->pinid == 16 * PORTNUM_B + 10)  // USART3_TX
  {
    pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_0;
  }
  else
  {
    pcf->hwpinflags = PINCFG_INPUT  | PINCFG_PULLUP; // no AF here (F1 only)
  }
}

void TUioGenDevImpl::SetupClockOut(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_0;
}

bool TUioGenDevImpl::LoadBuiltinConfig(uint8_t anum)
{
  return false;
}
