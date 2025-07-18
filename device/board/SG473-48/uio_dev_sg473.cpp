/*
 *  file:     uio_gendev_sg473.cpp
 *  brief:    MCU Specific implementation for the 48 pin STM32G473
 *  created:  2022-01-30
 *  authors:  nvitya
*/

#include "hwintflash.h"
#include "hwspi.h"
//#include "univio_comm.h"
#include "uio_device.h"
#include "clockcnt.h"

#define SPI_CS_PIN      28 // B12

#define UIOFUNC_DISABLED  0x0001
#define UIOFUNC_SPI       0x0100
#define UIOFUNC_I2C       0x0200
#define UIOFUNC_UART      0x0400
#define UIOFUNC_CLKOUT    0x0800
#define UIOFUNC_DAC       0x1000
#define UIOFUNC_CAN       0x2000

typedef struct
{
  uint32_t   flags; // bit0: 0 0 = disabled
  uint32_t   adc;  // 0 = none
  uint32_t   dac;  // not used anymore
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
/*  0   A0 */ { 1,   UIOADC(1, 1), 0, UIOPWM(2, 1, 1) },
/*  1   A1 */ { 1,   UIOADC(1, 2), 0, UIOPWM(2, 2, 1) },
/*  2   A2 */ { 1,   UIOADC(1, 3), 0, UIOPWM(2, 3, 1) },
/*  3   A3 */ { 1,   UIOADC(1, 4), 0, UIOPWM(2, 4, 1) },
/*  4   A4 */ { 1 | UIOFUNC_DAC,   0,             0, 0 },
/*  5   A5 */ { 1 | UIOFUNC_DAC,   0,  0, 0 },
/*  6   A6 */ { 1 | UIOFUNC_DAC,   0,  0, UIOPWM(3, 1, 2) },
/*  7   A7 */ { 1,   UIOADC(2, 4), 0, UIOPWM(3, 2, 2) },

/*  8   A8 */ { 1 | UIOFUNC_CLKOUT, 0, 0, UIOPWM(3, 1, 2) },  // 25 MHz Output
/*  9   A9 */ { RESERVED },  // TRACE_OUT = USART1_TX
/* 10  A10 */ { 1,   0, 0, UIOPWM(2, 3, 10) },
/* 11  A11 */ { RESERVED },  // USB D-
/* 12  A12 */ { RESERVED },  // USB D+
/* 13  A13 */ { RESERVED },  // SWDIO
/* 14  A14 */ { RESERVED },  // SWDCLK
/* 15  A15 */ { 1 | UIOFUNC_I2C,  0, 0, 0 },  // JTAG_TDI, I2C1_SCL

/* 16   B0 */ { 1,   UIOADC(1, 15), 0, UIOPWM(3, 3, 2) },
/* 17   B1 */ { 1,   UIOADC(1, 12), 0, UIOPWM(3, 4, 2) },
/* 18   B2 */ { 1,   UIOADC(2, 12), 0, 0 },
/* 19   B3 */ { 1 | UIOFUNC_CAN,   0, 0, 0 }, // JTAG_TDO / TRACESWO, remap required
/* 20   B4 */ { 1 | UIOFUNC_CAN,   0, 0, 0 }, // JNTRST  !! 5k internal pull-down to PA10!
/* 21   B5 */ { 1,   0, 0, UIOPWM(3, 2, 2) },
/* 22   B6 */ { 1,   0, 0, UIOPWM(4, 1, 2) },  // !! 5k internal pull-down to PA9!
/* 23   B7 */ { 1 | UIOFUNC_I2C,   0, 0, UIOPWM(4, 2, 2) },  // I2C1_SDA

/* 24   B8 */ { 1,   0, 0, UIOPWM(4, 3, 2) },  // BOOT0 control, no I2C here !
/* 25   B9 */ { 1,   0, 0, UIOPWM(4, 4, 2) },
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
/* 46  C14 */ { 1,   0, 0, 0 },  // OSC32 in, weak high drive!
/* 47  C15 */ { 1,   0, 0, 0 },  // OSC32 out, weak high drive!
};

//------------------------------------------------------------------------------------------------------
// TUnivioDevice Implementations
//------------------------------------------------------------------------------------------------------

// allocate the CAN SW buffers:
TCanMsg   can_rxbuf[UIO_CAN_COUNT][16];
TCanMsg   can_txbuf[UIO_CAN_COUNT][16];

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

#if 0
  // Use the last 32 kByte for data storage, the erase size must be smaller than 8k
  nvsaddr_setup  = hwintflash.start_address + hwintflash.bytesize - 32 * 1024;
  nvsaddr_nvdata = hwintflash.start_address + hwintflash.bytesize - 16 * 1024;
#else
  nvsaddr_setup  = hwintflash.start_address + 96 * 1024;
  nvsaddr_nvdata = nvsaddr_setup + 16 * 1024;
#endif
  nvs_sector_size = hwintflash.EraseSize(nvsaddr_setup);

  // SETUP RESERVED PINS / FUNCTIONS

  // disable

  PWR->CR3 |= PWR_CR3_UCPD_DBDIS;  // disable USB charging pull-down-s on the PB4 and PB6

  // Setup A8 to MCO (8 MHz)

  tmp = RCC->CFGR;
  tmp &= ~(RCC_CFGR_MCOPRE | RCC_CFGR_MCOSEL);
  tmp |= (4 << RCC_CFGR_MCOSEL_Pos); // 4 = HSE
  RCC->CFGR = tmp;
  //hwpinctrl.PinSetup(PORTNUM_A,  8,  PINCFG_OUTPUT | PINCFG_AF_0);

  // Init The ADC
  g_adc[0].dmaalloc = DMACH_ADC1;
  g_adc[0].Init(1, 0x901E); // enable channels  1,2,3,4, 12, 15
  g_adc[1].dmaalloc = DMACH_ADC2;
  g_adc[1].Init(2, 0x3018); // enable channels 3, 4, 12, 13

  // SPI initialization
	#if UIO_SPI_COUNT > 0
    //g_spi[0].datasample_late = true;
		g_spi[0].manualcspin = &g_pins[SPI_CS_PIN];
		g_spi[0].Init(2);
		g_dma_spi_tx[0].Init((DMACH_SPI_TX >> 8), DMACH_SPI_TX & 7, 13);
		g_dma_spi_rx[0].Init((DMACH_SPI_RX >> 8), DMACH_SPI_RX & 7, 12);
		g_spi[0].DmaAssign(true,  &g_dma_spi_tx[0]);
		g_spi[0].DmaAssign(false, &g_dma_spi_rx[0]);
	#endif

  // I2C initialization
	#if UIO_I2C_COUNT > 0
		g_i2c[0].Init(1);
		g_dma_i2c_tx[0].Init((DMACH_I2C_TX >> 8), DMACH_I2C_TX & 7, 17);
		g_dma_i2c_rx[0].Init((DMACH_I2C_RX >> 8), DMACH_I2C_RX & 7, 16);
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

	#if UIO_CAN_COUNT > 0
		// CAN-B initialization
		//hwpinctrl.PinSetup(PORTNUM_B,  3,  PINCFG_AF_11);  // CAN3_RX
		//hwpinctrl.PinSetup(PORTNUM_B,  4,  PINCFG_AF_11);  // CAN3_TX
		g_can[0].raw_timestamp = true;
		g_can[0].Init(3, &can_rxbuf[0][0], sizeof(can_rxbuf[0]) / sizeof(TCanMsg), &can_txbuf[0][0], sizeof(can_txbuf[0]) / sizeof(TCanMsg));
	#endif


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
    return (0 != (pinfo->flags & UIOFUNC_DAC));
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
  else if (UIO_PINTYPE_CAN == pintype)
  {
    return (0 != (pinfo->flags & UIOFUNC_CAN));
  }
  else
  {
    return true;
  }
}

void TUioDevImpl::SetupAdc(TPinCfg * pcf)
{
  const TPinInfo * pinfo = &g_pininfo[pcf->pinid];

  uint8_t adcch  = (pinfo->adc & 0x1F);
  uint8_t adcnum = ((pinfo->adc >> 8) & 3) - 1;

  adc_channel[pcf->unitnum] = ( 0x80 | adcch | (adcnum << 5) );
  pcf->hwpinflags = PINCFG_INPUT | PINCFG_ANALOGUE;
}

void TUioDevImpl::SetupDac(TPinCfg * pcf)
{
  THwDacChannel *   dac = &g_dac[pcf->unitnum];

  if (5 == pcf->pinid) // PA5 ?
  {
  	dac->Init(1, 2);  // DAC1_OUT2
  }
  else if (6 == pcf->pinid) // PA6 ?
  {
  	dac->Init(2, 1);  // DAC2_OUT1
  }
  else // PA4
  {
  	dac->Init(1, 1);  // DAC1_OUT1
  }

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

  pcf->hwpinflags = PINCFG_AF_4 | PINCFG_OPENDRAIN;
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

void TUioDevImpl::SetupCan(TPinCfg * pcf)
{
  const TPinInfo *  pinfo = &g_pininfo[pcf->pinid];
  pcf->hwpinflags = PINCFG_AF_11;
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
