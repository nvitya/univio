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
 *  file:     uio_gendev_base.cpp
 *  brief:    UNIVIO GENDEV target independent parts
 *  created:  2022-02-13
 *  authors:  nvitya
*/

#include "string.h"
#include "hwspi.h"
#include "hwi2c.h"
#include <uio_gendev_base.h>
#include "uio_nvdata.h"
#include "uio_nvstorage.h"
#include "traces.h"

THwSpi           g_spi;
THwI2c           g_i2c;
THwUart          g_uart;
THwAdc           g_adc[UIOMCU_ADC_COUNT];
THwPwmChannel    g_pwm[UIO_PWM_COUNT];
TGpioPin         g_pins[UIO_PIN_COUNT];

THwDmaChannel    g_dma_spi_tx;
THwDmaChannel    g_dma_spi_rx;

THwDmaChannel    g_dma_i2c_tx;
THwDmaChannel    g_dma_i2c_rx;

THwDmaChannel    g_dma_uart_tx;
THwDmaChannel    g_dma_uart_rx;

uint8_t          g_mpram[UIO_MPRAM_SIZE];

TUioCfgStb       g_cfgstb;

bool TUioGenDevBase::Init()
{
  initialized = false;

  // initialize configuration defaults
  cfg.usb_vendor_id = 0xDEAD;
  cfg.usb_product_id = 0xBEEF;

  strncpy(cfg.device_id, UIO_FW_ID, sizeof(cfg.device_id));
  strncpy(cfg.manufacturer, "github.com/nvitya/univio", sizeof(cfg.manufacturer));
  strncpy(cfg.serial_number, "1", sizeof(cfg.serial_number));

  if (!InitDevice())
  {
    return false;
  }

  initialized = true;
  return true;
}

bool TUioGenDevBase::InitDevice()
{
  unsigned n;

  mpram = &g_mpram[0];

  g_nvstorage.Init();

  blp_bit_clocks = (SystemCoreClock >> 4);  // 1/16 s

  spi = &g_spi;
  i2c = &g_i2c;
  uart = &g_uart;

  i2ctra.completed = true;

  // prepare g_pins

  for (n = 0; n < UIO_PIN_COUNT; ++n)
  {
    uint8_t   portnum = (n / UIO_PINS_PER_PORT);
    uint8_t   pinnum  = (n & (UIO_PINS_PER_PORT - 1));

    g_pins[n].Assign(portnum, pinnum, false);
  }

  if (!InitBoard())
  {
    return false;
  }

  g_nvdata.Init();

  ResetConfig();

  return true;
}

uint16_t TUioGenDevBase::PinSetup(uint8_t pinid, uint32_t pincfg, bool active)
{
  if (pinid >= UIO_PIN_COUNT)
  {
    return UIOERR_UNITSEL;
  }

  uint8_t pintype = (pincfg & 0xFF);
  uint8_t unitnum = ((pincfg >> 8) & 0xFF);

  TPinCfg   pcf;
  pcf.pinid = pinid;
  pcf.pincfg = pincfg;
  pcf.unitnum = unitnum;
  pcf.flags = ((pincfg >> 16) & 0xFFFF);
  pcf.hwpinflags = PINCFG_INPUT | PINCFG_PULLUP;

  pcf.pintype = 0; // check for reserved pins first
  if (!PinFuncAvailable(&pcf))  // do not touch reserved pins !
  {
    if (0 == pintype)
    {
      return 0;
    }
    else
    {
      __NOP();
      return UIOERR_FUNC_NOT_AVAIL;
    }
  }

  TGpioPin * ppin = &g_pins[pinid];
  if (!active)
  {
    ppin->Setup(PINCFG_INPUT | PINCFG_PULLUP);
  }

  pcf.pintype = pintype;
  if (!PinFuncAvailable(&pcf))
  {
    __NOP();
    return UIOERR_FUNC_NOT_AVAIL;
  }

  if (UIO_PINTYPE_PASSIVE == pintype) // passive = input with pullup
  {

  }

  // INPUTS

  else if (UIO_PINTYPE_DIG_IN == pintype) // digital input
  {
    if (unitnum >= UIO_DIN_COUNT)
    {
      return UIOERR_UNITSEL;
    }

    dig_in[unitnum] = ppin;

    pcf.hwpinflags = PINCFG_INPUT;
    if (0x0001 & pcf.flags)
    {
      pcf.hwpinflags |= PINCFG_PULLDOWN;
    }
    else if (0x0002 & pcf.flags)
    {
      // nothing = floating
    }
    else
    {
      pcf.hwpinflags |= PINCFG_PULLUP;
    }

    cfginfo[UIO_INFOIDX_DIN] |= (1 << unitnum);
  }

  else if (UIO_PINTYPE_ADC_IN == pintype)
  {
    if (unitnum >= UIO_ADC_COUNT)
    {
      return UIOERR_UNITSEL;
    }

    SetupAdc(&pcf);

    cfginfo[UIO_INFOIDX_ADC] |= (1 << unitnum);
  }

  // OUTPUTS

  else if ((UIO_PINTYPE_DIG_OUT == pintype) || (UIO_PINTYPE_LEDBLP == pintype))
  {
    bool initial_1;

    if (UIO_PINTYPE_LEDBLP == pintype)
    {
      if (unitnum >= UIO_LEDBLP_COUNT)
      {
        return UIOERR_UNITSEL;
      }

      ledblp[unitnum] = ppin;
      initial_1 = (0 != (ledblp_value[unitnum] & 1));

      cfginfo[UIO_INFOIDX_LEDBLP] |= (1 << unitnum);
    }
    else
    {
      if (unitnum >= UIO_DOUT_COUNT)
      {
        return UIOERR_UNITSEL;
      }

      dig_out[unitnum] = ppin;
      initial_1 = (0 != (dout_value & (1 << unitnum)));

      cfginfo[UIO_INFOIDX_DOUT] |= (1 << unitnum);
    }

    pcf.hwpinflags = PINCFG_OUTPUT | PINCFG_GPIO_INIT_0;

    if (0x0002 & pcf.flags)  // open-drain flag
    {
      pcf.hwpinflags |= PINCFG_OPENDRAIN;
    }

    if (0x0001 & pcf.flags) // inverted ?
    {
      ppin->Assign(ppin->portnum, ppin->pinnum, true);   // reassign required because of the inverted
      if (!initial_1)
      {
        pcf.hwpinflags |= PINCFG_GPIO_INIT_1;
      }
    }
    else
    {
      ppin->Assign(ppin->portnum, ppin->pinnum, false);  // reassign required because of the inverted
      if (initial_1)
      {
        pcf.hwpinflags |= PINCFG_GPIO_INIT_1;
      }
    }
  }

  else if (UIO_PINTYPE_DAC_OUT == pintype)
  {
    if (unitnum >= UIO_DAC_COUNT)
    {
      return UIOERR_UNITSEL;
    }

    if (active)
    {
      SetupDac(&pcf);
    }

    cfginfo[UIO_INFOIDX_DAC] |= (1 << unitnum);
  }
  else if (UIO_PINTYPE_PWM_OUT == pintype)
  {
    if (unitnum >= UIO_PWM_COUNT)
    {
      return UIOERR_UNITSEL;
    }

    THwPwmChannel * pwm = &g_pwm[unitnum];
    pwmch[unitnum] = pwm;

    if (active)
    {
      SetupPwm(&pcf);

      pwm->SetFrequency(cfg.pwm_freq[unitnum]);
      SetPwmDuty(unitnum, pwm_value[unitnum]);
      pwm->Enable();
    }

    cfginfo[UIO_INFOIDX_PWM] |= (1 << unitnum);
  }

  else if (UIO_PINTYPE_SPI == pintype)
  {
    if (active)
    {
      SetupSpi(&pcf);
    }
    cfginfo[UIO_INFOIDX_CBITS] |= UIO_INFOCBIT_SPI;
  }

  else if (UIO_PINTYPE_I2C == pintype)
  {
    if (active)
    {
      SetupI2c(&pcf);
    }
    cfginfo[UIO_INFOIDX_CBITS] |= UIO_INFOCBIT_I2C;
  }

  else if (UIO_PINTYPE_UART == pintype)
  {
		#if USB_UART_ENABLE
			if (active)
			{
				uart_active = true;  // will create the USB-UART port
				SetupUart(&pcf);
			}
	    cfginfo[UIO_INFOIDX_CBITS] |= UIO_INFOCBIT_UART;
		#endif
  }

  else if (UIO_PINTYPE_CLKOUT == pintype)
  {
    if (active)
    {
      SetupClockOut(&pcf);
    }
    cfginfo[UIO_INFOIDX_CBITS] |= UIO_INFOCBIT_CLKOUT;
  }

  else // unhandled config
  {
    return UIOERR_PINTYPE;
  }

  if (active)
  {
    ppin->Setup(pcf.hwpinflags);
  }
  else
  {
    ppin->Setup(PINCFG_INPUT | PINCFG_PULLUP);
  }


  //cfg.pinsetup[pinid] = pincfg;

  return 0;
}

uint16_t TUioGenDevBase::GetAdcValue(uint8_t adc_idx, uint16_t * rvalue)
{
  if (adc_idx >= UIO_ADC_COUNT)
  {
    return UIOERR_UNITSEL;
  }

  uint8_t adcinfo = adc_channel[adc_idx];
  if (0 == (0x80 & adcinfo))
  {
    return UIOERR_UNITSEL;
  }

  uint8_t adcch = (adcinfo & 0x1F);
  uint8_t adcnum = ((adcinfo >> 5) & 3);

  *rvalue = g_adc[adcnum].ChValue(adcch);
  return 0;
}

void TUioGenDevBase::SaveSetup()
{
  TRACE("Saving setup...\r\n");

  TUioCfgStb * pstb = &g_cfgstb;  // use the ram storage

  *pstb = cfg; // copy the settings

  pstb->signature = UIOCFG_V2_SIGNATURE;
  pstb->length = sizeof(TUioCfgStb);
  pstb->checksum = 0;
  pstb->checksum = uio_content_checksum(pstb, sizeof(*pstb));

  // save to flash
#if 1
  g_nvstorage.CopyTo(nvsaddr_setup, pstb, sizeof(*pstb));
#else
  g_nvstorage.Erase(nvsaddr_setup, sizeof(*pstb));
  g_nvstorage.Write(nvsaddr_setup, pstb, sizeof(*pstb));
#endif

  TRACE("Setup save completed.\r\n");
}

#if HAS_SPI_FLASH

  TUioCfgStb spifl_stb __attribute__((aligned(4)));  // local buffer for flash loading

#endif

void TUioGenDevBase::LoadSetup()
{
  #if HAS_SPI_FLASH
    g_nvstorage.Read(nvsaddr_setup, &spifl_stb, sizeof(spifl_stb));
    TUioCfgStb * pstb = &spifl_stb;
  #else
    TUioCfgStb * pstb = (TUioCfgStb *)nvsaddr_setup;
  #endif

  TRACE("Setup size = %u\r\n", sizeof(TUioCfgStb));

  TRACE("Loading Configuration...\r\n");

  if (pstb->signature != UIOCFG_V2_SIGNATURE)
  {
    TRACE("  signature error: %08X\r\n", pstb->signature);
    return;
  }

  if (pstb->length != sizeof(TUioCfgStb))
  {
    TRACE("  config length difference\r\n");
    return;
  }

  if (0 != uio_content_checksum(pstb, sizeof(TUioCfgStb)))
  {
    TRACE("  config checksum error\r\n");
    return;
  }

  TRACE("Saved setup ok, activating.\r\n");

  cfg = *pstb; // copy to the active configuration

  // TODO: load runmode
  runmode = 1;
  ConfigurePins(true);
}

void TUioGenDevBase::ClearConfig()
{
  unsigned n;

  uart_active = false;

  for (n = 0; n < UIO_PIN_COUNT; ++n)
  {
    cfg.pinsetup[n] = 0;
  }

  // set output defaults
  cfg.dv_douts = 0;

  for (n = 0; n < UIO_DAC_COUNT; ++n)
  {
    cfg.dv_dac[n] = 0;
  }

  for (n = 0; n < UIO_PWM_COUNT; ++n)
  {
    cfg.dv_pwm[n] = 0;
    cfg.pwm_freq[n] = 1000;
  }

  for (n = 0; n < UIO_LEDBLP_COUNT; ++n)
  {
    cfg.dv_ledblp[n] = 0x00000000;
  }
}

void TUioGenDevBase::ResetConfig()
{
  unsigned n;

  ConfigurePins(false); // set all pins passive


}

void TUioGenDevBase::ConfigurePins(bool active)
{
	unsigned n;

  // clear config info
	for (n = 0; n < UIO_INFO_COUNT; ++n)
	{
		cfginfo[n] = 0;
	}

  // 1. Clear all assignments

  // inputs

  for (n = 0; n < UIO_DIN_COUNT; ++n)
  {
    dig_in[n] = nullptr;
  }

  for (n = 0; n < UIO_ADC_COUNT; ++n)
  {
    adc_channel[n] = 0x00;
  }

  // outputs

  dout_value = cfg.dv_douts;
  for (n = 0; n < UIO_DOUT_COUNT; ++n)
  {
    dig_out[n] = nullptr;
  }

  for (n = 0; n < UIO_DAC_COUNT; ++n)
  {
    dac_value[n] = cfg.dv_dac[n];
  }

  for (n = 0; n < UIO_PWM_COUNT; ++n)
  {
    pwmch[n] = nullptr;
    pwm_value[n] = cfg.dv_pwm[n];
  }

  for (n = 0; n < UIO_LEDBLP_COUNT; ++n)
  {
    ledblp[n] = nullptr;
    ledblp_value[n] = cfg.dv_ledblp[n];
  }

  // 2. setup the pins and assignments

  for (n = 0; n < UIO_PIN_COUNT; ++n)
  {
    if (active)
    {
      PinSetup(n, cfg.pinsetup[n], true);
    }
    else
    {
      PinSetup(n, 0, false); // set passive
    }
  }
}

void TUioGenDevBase::SetPwmDuty(uint8_t apwmnum, uint16_t aduty)
{
  if (apwmnum >= UIO_PWM_COUNT)
  {
    return;
  }

  THwPwmChannel * pwm = pwmch[apwmnum];
  if (!pwm)
  {
    return;
  }

  uint16_t onclocks = ((pwm->periodclocks * aduty) >> 16);

  pwm->SetOnClocks(onclocks);
}

void TUioGenDevBase::SetDacOutput(uint8_t adacnum, uint16_t avalue)
{
  //TODO: implement
}

uint16_t TUioGenDevBase::SpiStart()
{
  if (spi_status)
  {
    return UDOERR_BUSY;
  }

  if (!spi)
  {
    return UIOERR_UNITSEL;
  }

  if ((0 == spi_speed) || (spi_trlen == 0)
       || (spi_trlen > UIO_MPRAM_SIZE - spi_rx_offs) || (spi_trlen > UIO_MPRAM_SIZE - spi_tx_offs))
  {
    return UIOERR_UNIT_PARAMS;
  }

  if (spi->speed != spi_speed)
  {
    spi->speed = spi_speed;
    spi->Init(spi->devnum); // re-init the device
  }

  spi->StartTransfer(0, 0, 0, spi_trlen, &mpram[spi_tx_offs], &mpram[spi_rx_offs]);

  spi_status = 1;

  return 0;
}

uint16_t TUioGenDevBase::I2cStart()
{
  if (0xFFFF == i2c_result)
  {
    return UDOERR_BUSY;
  }

  if (!i2c)
  {
    return UIOERR_UNITSEL;
  }

  if (!i2ctra.completed)
  {
    return UDOERR_BUSY;
  }

  i2c_trlen = (i2c_cmd >> 16);
  uint32_t edata_len = ((i2c_cmd >> 12) & 3);

  if ((0 == i2c_speed) || (i2c_trlen == 0) || (i2c_trlen > UIO_MPRAM_SIZE - i2c_data_offs))
  {
    return UIOERR_UNIT_PARAMS;
  }

  if (i2c->speed != i2c_speed)
  {
    i2c->speed = i2c_speed;
    i2c->Init(i2c->devnum); // re-init the device
  }

  if (i2c_cmd & UIO_I2C_CMD_WRITE)
  {
    i2c->StartWrite(&i2ctra, i2c_cmd & 0x7F, i2c_eaddr | (edata_len << 24), &mpram[i2c_data_offs], i2c_trlen);
  }
  else
  {
    i2c->StartRead(&i2ctra, i2c_cmd & 0x7F, i2c_eaddr | (edata_len << 24), &mpram[i2c_data_offs], i2c_trlen);
  }

  i2c_result = 0xFFFF;

  return 0;
}

void TUioGenDevBase::SetRunMode(uint8_t arunmode)
{
  // TODO: Save Config

  if ((runmode != arunmode) && (1 == arunmode))
  {
    SaveSetup();
  }

  runmode = arunmode;

  ConfigurePins(1 == runmode);

  // TODO: Save RunMode
}

void TUioGenDevBase::Run() // handle led blink patterns
{
  unsigned n;
  unsigned t0 = CLOCKCNT;

  if (t0 - last_blp_time >= blp_bit_clocks)
  {
    blp_idx = ((blp_idx + 1) & 0x1F);
    blp_mask = (1 << blp_idx);

    for (n = 0; n < UIO_LEDBLP_COUNT; ++n)
    {
      TGpioPin * ppin = ledblp[n];
      if (ppin)
      {
        if (ledblp_value[n] & blp_mask)
        {
          ppin->Set1();
        }
        else
        {
          ppin->Set0();
        }
      }
    }

    last_blp_time = t0;
  }

  // check SPI
  if (spi_status)
  {
    spi->Run();
    if (spi->finished)
    {
      spi_status = 0;
    }
  }

  // check I2C
  if (0xFFFF == i2c_result)
  {
    i2c->Run();
    if (i2ctra.completed)
    {
      i2c_result = i2ctra.error;
    }
  }

}

uint32_t uio_content_checksum(void * adataptr, uint32_t adatalen)
{
  int32_t remaining = adatalen;
  uint32_t * dp = (uint32_t *)adataptr;
  uint32_t csum = 0;

  while (remaining > 3)
  {
    csum += *dp++;
    remaining -= 4;
  }

  if (1 == remaining)
  {
    csum += (*dp & 0x000000FF);
  }
  else if (2 == remaining)
  {
    csum += (*dp & 0x0000FFFF);
  }
  else if (3 == remaining)
  {
    csum += (*dp & 0x00FFFFFF);
  }

  return(0 - csum);
}
