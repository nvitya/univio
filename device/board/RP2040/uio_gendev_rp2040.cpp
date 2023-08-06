/*
 *  file:     uio_gendev_rp2040.cpp
 *  brief:    MCU Specific implementation for the RP2040
 *  created:  2022-06-11
 *  authors:  nvitya
*/

#include "hwintflash.h"
#include "hwspi.h"
#include "uio_device.h"
#include "clockcnt.h"
#include "board_pins.h"

#define SPI_CS_PIN      13

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
#define UIOPWM(channel, outnum)         (0x80000000 | (channel << 0) | (outnum << 4))
#define UIOADC(adcnum, innum)           (0x80000000 | (adcnum << 8) | (innum << 0))

const TPinInfo g_pininfo[UIO_PIN_COUNT] =
{
/*  0   A0 */ { RESERVED },  // TRACE_OUT = USART0_TX - reserved for programming
/*  1   A1 */ { RESERVED },  // USART0_RX - reserved for bootloader programming
/*  2   A2 */ { 1 | UIOFUNC_I2C,   0, 0, UIOPWM(1, 0) },  // I2C1_SDA
/*  3   A3 */ { 1 | UIOFUNC_I2C,   0, 0, UIOPWM(1, 1) },  // I2C1_SCL
/*  4   A4 */ { 1 | UIOFUNC_UART,  0, 0, UIOPWM(2, 0) },  // UART_TX_OUT = UART1_TX
/*  5   A5 */ { 1 | UIOFUNC_UART,  0, 0, UIOPWM(2, 1) },  // UART_RX_IN  = UART1_RX
/*  6   A6 */ { 1,   0, 0, UIOPWM(3, 0) },
/*  7   A7 */ { 1,   0, 0, UIOPWM(3, 1) },

/*  8   A8 */ { 1,   0, 0, UIOPWM(4, 0) },
/*  9   A9 */ { 1,   0, 0, UIOPWM(4, 1) },
/* 10  A10 */ { 1 | UIOFUNC_SPI,   0, 0, UIOPWM(5, 0) },  // SPI1_SCK
/* 11  A11 */ { 1 | UIOFUNC_SPI,   0, 0, UIOPWM(5, 1) },  // SPI1_TX = MOSI
/* 12  A12 */ { 1 | UIOFUNC_SPI,   0, 0, UIOPWM(6, 0) },  // SPI1_RX = MISO
/* 13  A13 */ { 1 | UIOFUNC_SPI,   0, 0, UIOPWM(6, 1) },  // SPI1_CSn
/* 14  A14 */ { 1,   0, 0, UIOPWM(7, 0) },
/* 15  A15 */ { 1,   0, 0, UIOPWM(7, 1) },

/* 16  A16 */ { 1,   0, 0, UIOPWM(0, 0) },
/* 17  A17 */ { 1,   0, 0, UIOPWM(0, 1) },
/* 18  A18 */ { 1,   0, 0, UIOPWM(1, 0) },
/* 19  A19 */ { 1,   0, 0, UIOPWM(1, 1) },
/* 20  A20 */ { 1,   0, 0, UIOPWM(2, 0) },
/* 21  A21 */ { 1 | UIOFUNC_CLKOUT,   0, 0, UIOPWM(2, 1) },  // CLOCK GPOUT0: 12 MHz Output
/* 22  A22 */ { 1,   0, 0, UIOPWM(3, 0) },
/* 23  A23 */ { 1,   0, 0, UIOPWM(3, 1) }, // RP-PICO: internal supply light load control (output)

/* 24  A24 */ { 1,   0, 0, UIOPWM(4, 0) }, // RP-PICO: USB VBUS detect (input)
/* 25  A25 */ { 1,   0, 0, UIOPWM(4, 1) }, // RP-PICO: internal LED (output)
/* 26  A26 */ { 1,   UIOADC(0, 0), 0, 0 },
/* 27  A27 */ { 1,   UIOADC(0, 1), 0, 0 },
/* 28  A28 */ { 1,   UIOADC(0, 2), 0, 0 },
/* 29  A29 */ { 1,   UIOADC(0, 3), 0, 0 },  // RP-PICO: measures supply input voltage (VSYS)
/* 30  A30 */ { NOTEXISTING },
/* 31  A31 */ { NOTEXISTING }
};

//------------------------------------------------------------------------------------------------------
// TUnivioDevice Implementations
//------------------------------------------------------------------------------------------------------

#define TEST_I2C 0

#if TEST_I2C

#include "traces.h"

TI2cTransaction  g_i2ctra;

void test_i2c_basic()
{
  TRACE("I2C basic test\r\n");

  unsigned devaddr = 0x77;
  unsigned addr = 0xD0;
  unsigned len = 1;

  uint8_t  rxbuf[64];

  TRACE("Reading memory at %04X...\r\n", addr);

  //g_i2c.Init(1);

  g_i2c.StartRead(&g_i2ctra, devaddr, addr | I2CEX_1, &rxbuf[0], len);
  g_i2c.WaitFinish(&g_i2ctra);
  if (g_i2ctra.error)
  {
    TRACE("  I2C error = %i\r\n", g_i2ctra.error);
  }
  else
  {
    TRACE("  OK.\r\n");
  }

  TRACE("I2C test finished.\r\n");
}

#endif

bool TUioGenDevImpl::InitBoard()
{
  unsigned n;
  uint32_t tmp;

  // use the last 64 kByte as data storage
  nvsaddr_setup  = (spiflash.bytesize - 0x10000);
  nvsaddr_nvdata = nvsaddr_setup + 0x8000;
  nvs_sector_size = 4096;

  // SETUP RESERVED PINS / FUNCTIONS

  // Setup 21 to CLOCK GPOUT0 (12 MHz)

  clock_hw_t * pclk_gpout0 = &clocks_hw->clk[clk_gpout0];
  pclk_gpout0->ctrl &= ~(1 << 11);  // disable
  pclk_gpout0->ctrl = (0
    | (0  << 20)  // NUDGE
    | (0  << 16)  // PHASE(2)
    | (0  << 12)  // DC50
    | (0  << 11)  // ENABLE
    | (0  << 10)  // KILL
    | (5  <<  5)  // AUXSRC(4); 0 = sys, 7 = USB CLOCK, 5 = XOSC CLKSRC
  );
  pclk_gpout0->div = (1 << 8); // do not divide
  pclk_gpout0->ctrl |= (1 << 11); // enable

  // Init The ADC
  g_adc[0].dmachannel = DMACH_ADC;
  g_adc[0].Init(0, 0xF); // enable all the 4 channels

  // SPI initialization
  g_spi.manualcspin = &g_pins[SPI_CS_PIN];
  g_spi.Init(1);
  g_dma_spi_tx.Init( DMACH_SPI_TX, DREQ_SPI1_TX );
  g_dma_spi_rx.Init( DMACH_SPI_RX, DREQ_SPI1_RX);
  g_spi.DmaAssign(true,  &g_dma_spi_tx);
  g_spi.DmaAssign(false, &g_dma_spi_rx);

  // I2C initialization
  g_i2c.Init(1);
  g_dma_i2c_tx.Init( DMACH_I2C_TX, DREQ_I2C1_TX );
  g_dma_i2c_rx.Init( DMACH_I2C_RX, DREQ_I2C1_RX);
  g_i2c.DmaAssign(true,  &g_dma_i2c_tx);
  g_i2c.DmaAssign(false, &g_dma_i2c_rx);

#if TEST_I2C

  hwpinctrl.PinSetup(0, 2, PINCFG_AF_3 | PINCFG_PULLUP); // I2C1_SDA
  hwpinctrl.PinSetup(0, 3, PINCFG_AF_3 | PINCFG_PULLUP); // I2C1_SCL

  test_i2c_basic();
  //test_i2c_basic();

  //TRACE_FLUSH();

#endif

  // UART1 Initialization
  g_uart.Init(1);
  g_dma_uart_tx.Init(DMACH_USBUART_TX, DREQ_UART1_TX);
  g_dma_uart_rx.Init(DMACH_USBUART_RX, DREQ_UART1_RX);
  g_uart.DmaAssign(true,  &g_dma_uart_tx);
  g_uart.DmaAssign(false, &g_dma_uart_rx);

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

void TUioGenDevImpl::SetupPwm(TPinCfg * pcf)
{
  const TPinInfo *  pinfo = &g_pininfo[pcf->pinid];
  THwPwmChannel *   pwm = &g_pwm[pcf->unitnum];

  if (!pwm->Init(0, pinfo->pwm & 0xF, (pinfo->pwm >> 4) & 0xF))
  {
    return;
  }

  pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_4;
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

  pcf->hwpinflags = PINCFG_AF_3 | PINCFG_PULLUP;
}

void TUioGenDevImpl::SetupUart(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  pcf->hwpinflags = PINCFG_AF_2 | PINCFG_PULLUP;
}

void TUioGenDevImpl::SetupClockOut(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_8;
}

bool TUioGenDevImpl::LoadBuiltinConfig(uint8_t anum)
{
  return false;
}
