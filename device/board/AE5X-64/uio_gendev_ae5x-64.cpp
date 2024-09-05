// uio_gendev.cpp (64-pin ATSAM4S)

#include "hwintflash.h"
#include "hwspi.h"
#include "uio_device.h"


#define SPI_CS_PIN       18 // A18
#define UART_TX_PIN      12 // A12

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

#define UIO_AF_F  5
#define UIO_AF_G  6

const TPinInfo g_pininfo[UIO_PIN_COUNT] =
{
/*  0   A0 */ { RESERVED },  // SERCOM1[0]: TRACE_TX
/*  1   A1 */ { 1,   0, 0, 0 },  // On board LED !
/*  2   A2 */ { 1,   0, UIODAC(0), 0 },
/*  3   A3 */ { 1,   UIOADC(0, 1), 0, 0 },
/*  4   A4 */ { 1,   UIOADC(0, 4), 0, 0 },
/*  5   A5 */ { 1,   0, UIODAC(1), 0 },
/*  6   A6 */ { 1,   UIOADC(0, 6), 0, 0 },
/*  7   A7 */ { 1,   UIOADC(0, 7), 0, 0 },

/*  8   A8 */ { 1,   UIOADC(0,  8), 0, UIOPWM(0, 0, UIO_AF_F) },
/*  9   A9 */ { 1,   UIOADC(0,  9), 0, UIOPWM(0, 1, UIO_AF_F) },
/* 10  A10 */ { 1,   UIOADC(0, 10), 0, UIOPWM(0, 2, UIO_AF_F) },
/* 11  A11 */ { 1,   UIOADC(0, 11), 0, UIOPWM(0, 3, UIO_AF_F) },
/* 12  A12 */ { 1 | UIOFUNC_UART,   0, 0, 0 },  // SERCOM2[0]: UART_TX
/* 13  A13 */ { 1 | UIOFUNC_UART,   0, 0, 0 },  // SERCOM2[1]: UART_RX
/* 14  A14 */ { RESERVED },  // XIN
/* 15  A15 */ { RESERVED },  // XOUT

/* 16  A16 */ { 1 | UIOFUNC_SPI,   0, 0, 0 },  // SERCOM3[1]: SPI_SCK
/* 17  A17 */ { 1 | UIOFUNC_SPI,   0, 0, 0 },  // SERCOM3[0]: SPI_MOSI
/* 18  A18 */ { 1 | UIOFUNC_SPI,   0, 0, 0 },  // SERCOM3[2]: SPI_CS
/* 19  A19 */ { 1 | UIOFUNC_SPI,   0, 0, 0 },  // SERCOM3[3]: SPI_MISO
/* 20  A20 */ { 1,   0, 0, UIOPWM(1, 4, UIO_AF_F) },
/* 21  A21 */ { 1,   0, 0, UIOPWM(1, 5, UIO_AF_F) },
/* 22  A22 */ { 1,   0, 0, UIOPWM(0, 2, UIO_AF_F) },
/* 23  A23 */ { 1,   0, 0, UIOPWM(0, 3, UIO_AF_F) },

/* 24  A24 */ { RESERVED },  // USB D-
/* 25  A25 */ { RESERVED },  // USB D+
/* 26  A26 */ { NOTEXISTING },
/* 27  A27 */ { 1,   0, 0, 0 },
/* 28  A28 */ { NOTEXISTING },
/* 29  A29 */ { NOTEXISTING },
/* 30  A30 */ { RESERVED },  // SWCLK
/* 31  A31 */ { RESERVED },  // SWDIO

/* 32   B0 */ { 1,   UIOADC(0, 12), 0, 0 },
/* 33   B1 */ { 1,   UIOADC(0, 13), 0, 0 },
/* 34   B2 */ { 1,   UIOADC(0, 14), 0, UIOPWM(4, 1, UIO_AF_F) },
/* 35   B3 */ { 1,   UIOADC(0, 15), 0, 0 },
/* 36   B4 */ { 1,   0, 0, 0 },
/* 37   B5 */ { 1,   0, 0, 0 },
/* 38   B6 */ { 1,   0, 0, 0 },
/* 39   B7 */ { 1,   0, 0, 0 },

/* 40   B8 */ { 1,   UIOADC(0, 2), 0, 0 },
/* 41   B9 */ { 1,   UIOADC(0, 3), 0, 0 },
/* 42  B10 */ { 1,   0, 0, UIOPWM(1, 0, UIO_AF_G) },
/* 43  B11 */ { 1,   0, 0, UIOPWM(1, 1, UIO_AF_G) },
/* 44  B12 */ { 1,   0, 0, 0 },
/* 45  B13 */ { 1 | UIOFUNC_CLKOUT,   0, 0, 0 }, // GCLK_IO_7 = 12 MHz clock output
/* 46  B14 */ { 1,   0, 0, UIOPWM(4, 0, UIO_AF_F) },
/* 47  B15 */ { 1,   0, 0, UIOPWM(4, 1, UIO_AF_F) },

/* 48  B16 */ { 1 | UIOFUNC_I2C,   0, 0, 0 },  // SERCOM5[0]: SDA
/* 49  B17 */ { 1 | UIOFUNC_I2C,   0, 0, 0 },  // SERCOM5[1]: SCL
/* 50  B18 */ { NOTEXISTING },
/* 51  B19 */ { NOTEXISTING },
/* 52  B20 */ { NOTEXISTING },
/* 53  B21 */ { NOTEXISTING },
/* 54  B22 */ { 1,   0, 0, 0 },
/* 55  B23 */ { 1,   0, 0, 0 },

/* 56  B24 */ { NOTEXISTING },
/* 57  B25 */ { NOTEXISTING },
/* 58  B26 */ { NOTEXISTING },
/* 59  B27 */ { NOTEXISTING },
/* 60  B28 */ { NOTEXISTING },
/* 61  B29 */ { NOTEXISTING },
/* 62  B30 */ { RESERVED },  // SWO
/* 63  B31 */ { 1,   0, 0, UIOPWM(4, 1, UIO_AF_F) },

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
  // the erase size depend on flash size, the highest value is 8k
  nvsaddr_setup  = hwintflash.start_address + hwintflash.bytesize - 32 * 1024;
  nvsaddr_nvdata = hwintflash.start_address + hwintflash.bytesize - 16 * 1024;
  nvs_sector_size = hwintflash.EraseSize(nvsaddr_setup);


  // Setup B13 to GCLK_IO7
  GCLK->GENCTRL[7].reg = 0
    | (0         << 0) // SRC(5): 0 = XOSC0
    | (1        <<  8) // GENEN: 1 = ENABLE
    | (1        <<  9) // IDC
    | (0        << 10) // OOV
    | (1        << 11) // OE: output enable
    | (0        << 12) // DIVSEL: 0 = normal division, 1 = 2^DIVSEL division
    | (1        << 13) // RUNSTDBY: 1 = run in standby
    | (1        << 16) // DIV(16): 1 = no division
  ;

  // Init The ADC
  g_adc[0].dmachannel     = DMACH_ADC_DATA;
  g_adc[0].seq_dmachannel = DMACH_ADC_SQ;
  g_adc[0].Init(0, 0xFFDE); // enable all 16 channels except the DOUTs (0, 5)

  // SPI initialization (SERCOM3)
#if UIO_SPI_COUNT > 0
  g_spi[0].manualcspin = &g_pins[SPI_CS_PIN];
  g_spi[0].Init(3);
  g_dma_spi_tx[0].Init(DMACH_SPI_TX, SERCOM3_DMAC_ID_TX);
  g_dma_spi_rx[0].Init(DMACH_SPI_RX, SERCOM3_DMAC_ID_RX);
  g_spi[0].DmaAssign(true,  &g_dma_spi_tx[0]);
  g_spi[0].DmaAssign(false, &g_dma_spi_rx[0]);
#endif


  // I2C initialization (SERCOM5)
#if UIO_I2C_COUNT > 0
  g_i2c[0].Init(5);
  g_dma_i2c_tx[0].Init(DMACH_I2C_TX, SERCOM5_DMAC_ID_TX);
  g_dma_i2c_rx[0].Init(DMACH_I2C_RX, SERCOM5_DMAC_ID_RX);
  g_i2c[0].DmaAssign(true,  &g_dma_i2c_tx[0]);
  g_i2c[0].DmaAssign(false, &g_dma_i2c_rx[0]);
#endif

  // UART Initialization (SERCOM2)
#if UIO_UART_COUNT > 0
  g_uart[0].Init(2);
  g_dma_uart_tx[0].Init(DMACH_UART_TX, SERCOM2_DMAC_ID_TX);
  g_dma_uart_rx[0].Init(DMACH_UART_RX, SERCOM2_DMAC_ID_RX);
  g_uart[0].DmaAssign(true,  &g_dma_uart_tx[0]);
  g_uart[0].DmaAssign(false, &g_dma_uart_rx[0]);
#endif

  // USB PINS
  hwpinctrl.PinSetup(PORTNUM_A, 24, PINCFG_AF_H | PINCFG_DRIVE_STRONG);  // USB DM
  hwpinctrl.PinSetup(PORTNUM_A, 25, PINCFG_AF_H | PINCFG_DRIVE_STRONG);  // USB DP

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
  //pcf->hwpinflags = PINCFG_INPUT | PINCFG_PULLUP;
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
  // A16: SERCOM3[1]: SPI_SCK
  // A17: SERCOM3[0]: SPI_MOSI
  // A18: SERCOM3[2]: SPI_CS
  // A19: SERCOM3[3]: SPI_MISO

  TGpioPin * ppin = &g_pins[pcf->pinid];

  if (SPI_CS_PIN == pcf->pinid)
  {
    pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_GPIO_INIT_1;
    ppin->Assign(ppin->portnum, ppin->pinnum, false);   // reassign required because it was maybe inverted
  }
  else
  {
    pcf->hwpinflags = PINCFG_AF_D | PINCFG_SPEED_FAST;
  }
}

void TUioGenDevImpl::SetupI2c(TPinCfg * pcf)
{
  // B16: SERCOM5[0]: SDA
  // B17: SERCOM5[1]: SCL

  TGpioPin * ppin = &g_pins[pcf->pinid];

  pcf->hwpinflags = PINCFG_AF_C | PINCFG_PULLUP;
}

void TUioGenDevImpl::SetupUart(TPinCfg * pcf)
{
  // A12: SERCOM2[0]: UART_TX
  // A13: SERCOM2[1]: UART_RX

  TGpioPin * ppin = &g_pins[pcf->pinid];

  if (UART_TX_PIN == pcf->pinid)  // UART0_TX
  {
    pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_C | PINCFG_SPEED_FAST;
  }
  else
  {
    pcf->hwpinflags = PINCFG_INPUT  | PINCFG_AF_C | PINCFG_PULLUP | PINCFG_SPEED_FAST;
  }
}

void TUioGenDevImpl::SetupClockOut(TPinCfg * pcf)
{
  TGpioPin * ppin = &g_pins[pcf->pinid];

  pcf->hwpinflags = PINCFG_OUTPUT | PINCFG_AF_M;
}


bool TUioGenDevImpl::LoadBuiltinConfig(uint8_t anum)
{
  return false;
}
