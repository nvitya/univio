/*
 * uio_gendev_base.cpp
 *
 *  Created on: Nov 24, 2021
 *      Author: vitya
 */

#include "string.h"
#include "hwspi.h"
#include "hwi2c.h"
#include "hwintflash.h"
#include <uio_gendev_base.h>
#include "uio_nvdata.h"
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

bool TUioGenDevBase::InitDevice()
{
  unsigned n;

  mpram = &g_mpram[0];

  hwintflash.Init();

  blp_bit_clocks = (SystemCoreClock >> 4);  // 1/16 s

  spi = &g_spi;
  i2c = &g_i2c;
  uart = &g_uart;

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
  }

  else if (UIO_PINTYPE_ADC_IN == pintype)
  {
    if (unitnum >= UIO_ADC_COUNT)
    {
      return UIOERR_UNITSEL;
    }

    SetupAdc(&pcf);
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
    }
    else
    {
      if (unitnum >= UIO_DOUT_COUNT)
      {
        return UIOERR_UNITSEL;
      }

      dig_out[unitnum] = ppin;
      initial_1 = (0 != (dout_value & (1 << unitnum)));
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
  }

  else if (UIO_PINTYPE_SPI == pintype)
  {
    if (active)
    {
      SetupSpi(&pcf);
    }
  }

  else if (UIO_PINTYPE_I2C == pintype)
  {
    if (active)
    {
      SetupI2c(&pcf);
    }
  }

  else if (UIO_PINTYPE_UART == pintype)
  {
		#if USB_UART_ENABLE
			if (active)
			{
				uart_active = true;  // will create the USB-UART port
				SetupUart(&pcf);
			}
		#endif
  }

  else if (UIO_PINTYPE_CLKOUT == pintype)
  {
    if (active)
    {
      SetupClockOut(&pcf);
    }
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

bool TUioGenDevBase::HandleDeviceRequest(TUnivioRequest * rq)
{
  unsigned    n;
  uint32_t    rv32;
  uint16_t    rv16;
  uint16_t    err;

  uint16_t addr = rq->address;

  if (addr < 0x0200) // some read-only info
  {
    switch (addr)
    {
      case 0x0100:  return ResponseU8(rq,  UIO_PIN_COUNT);
      case 0x0101:  return ResponseU16(rq, UIO_PINS_PER_PORT);
      case 0x0102:  return ResponseU16(rq, UIO_MPRAM_SIZE);

      case 0x0110:
      {
        if (!rq->iswrite)  return ResponseError(rq, UIOERR_WRITE_ONLY);

        if (1 == RqValueU32(rq))
        {
          ResetConfig();
        }
        return ResponseOk(rq);
      }
    }

    return ResponseError(rq, UIOERR_WRONG_ADDR);
  }

  if (addr < 0x0300) // pin configuration
  {
    uint8_t pinid = addr - 0x0200;
    if (pinid >= UIO_PIN_COUNT)
    {
      return ResponseError(rq, UIOERR_WRONG_ADDR);
    }

    if (rq->iswrite)
    {
      if (runmode)
      {
        return ResponseError(rq, UIOERR_RUN_MODE);
      }

      uint32_t pcf = RqValueU32(rq);
      uint16_t err = PinSetup(pinid, pcf, false);
      if (err)
      {
        return ResponseError(rq, err);
      }

      cfg.pinsetup[pinid] = pcf;
      return ResponseOk(rq);
    }
    else
    {
      return ResponseU32(rq, cfg.pinsetup[pinid]);
    }
  }

  if (0x0300 == addr)  // DOUT
  {
    return HandleRw(rq, &cfg.dv_douts, sizeof(cfg.dv_douts));
  }

  if ((0x0320 <= addr) && (addr < 0x0320 + UIO_DAC_COUNT))  // ANA_OUT / DAC
  {
    return HandleRw(rq, &cfg.dv_dac[addr - 0x320], sizeof(cfg.dv_dac[0]));
  }

  if ((0x0340 <= addr) && (addr < 0x0340 + UIO_PWM_COUNT))  // PWM Duty
  {
    return HandleRw(rq, &cfg.dv_pwm[addr - 0x340], sizeof(cfg.dv_pwm[0]));
  }

  if ((0x0360 <= addr) && (addr < 0x0360 + UIO_LEDBLP_COUNT))  // LEDBLP
  {
    return HandleRw(rq, &cfg.dv_ledblp[addr - 0x360], sizeof(cfg.dv_ledblp[0]));
  }

  if ((0x0700 <= addr) && (addr < 0x0700 + UIO_PWM_COUNT)) // PWM frequency setup
  {
    uint8_t idx = addr - 0x0700;

    HandleRw(rq, &cfg.pwm_freq[idx], sizeof(cfg.pwm_freq[0]));  // save to the shadow

    if (!rq->iswrite)
    {
      return true; // already handled
    }

    THwPwmChannel * pwm = pwmch[idx];
    if (pwm && pwm->initialized)
    {
      pwm->SetFrequency(cfg.pwm_freq[idx]);  // update at the actual unit too
    }
    return ResponseOk(rq);
  }

  // Non-Volatile Data
  if ((0x0F00 <= addr) && (addr < 0x0F00 + UIO_NVDATA_COUNT))  // NVDATA Value
  {
    uint8_t idx = (addr - 0x0F00);
    if (rq->iswrite)
    {
      rv32 = RqValueU32(rq);
      return ResponseError(rq, g_nvdata.SaveValue(idx, rv32));
    }
    else
    {
      return ResponseU32(rq, g_nvdata.value[idx]);
    }
  }
  if (0x0F80 == addr) // NVDATA LOCK
  {
    return HandleRw(rq, &g_nvdata.lock, sizeof(g_nvdata.lock));
  }

  // IO Control

  if ((0x1000 <= addr) && (addr <= 0x1001))  // DOUT Set/Clear
  {
    if (!rq->iswrite)
    {
      return ResponseError(rq, UIOERR_WRITE_ONLY);
    }

    rv32 = RqValueU32(rq);

    for (n = 0; n < 16; ++n)
    {
      uint8_t idx = n + 16 * (addr - 0x1000);
      if (idx < UIO_DOUT_COUNT)
      {
        TGpioPin *  ppin = dig_out[idx];
        if (ppin)
        {
          uint32_t smask = (1 << n);
          uint32_t cmask = (1 << (n + 16));
          if (rv32 & smask)
          {
            ppin->Set1();
            dout_value |= (1 << idx);
          }
          else if (rv32 & cmask)
          {
            ppin->Set0();
            dout_value &= ~(1 << idx);
          }
        }
      }
    }
    return ResponseOk(rq);
  }
  else if (0x1010 == addr) // DOUT direct setup
  {
    if (!rq->iswrite)
    {
      return ResponseU32(rq, dout_value);
    }

    dout_value = RqValueU32(rq);

    for (n = 0; n < UIO_DOUT_COUNT; ++n)
    {
      TGpioPin *  ppin = dig_out[n];
      if (ppin)
      {
        uint32_t smask = (1 << n);
        if (dout_value & smask)
        {
          ppin->Set1();
        }
        else
        {
          ppin->Set0();
        }
      }
    }
    return ResponseOk(rq);
  }
  else if (0x1100 == addr) // Get DIN
  {
    if (rq->iswrite)
    {
      return ResponseError(rq, UIOERR_READ_ONLY);
    }

    rv32 = 0;
    for (n = 0; n < UIO_DIN_COUNT; ++n)
    {
      TGpioPin *  ppin = dig_in[n];
      uint32_t    pmask = (1 << n);
      if (ppin && ppin->Value())
      {
        rv32 |= pmask;
      }
    }
    return ResponseU32(rq, rv32);
  }
  else if ((0x1200 <= addr) && (addr < 0x1200 + UIO_ADC_COUNT)) // Get ANA_IN
  {
    if (rq->iswrite)
    {
      return ResponseError(rq, UIOERR_READ_ONLY);
    }

    uint8_t idx = (addr - 0x1200);
    err = GetAdcValue(idx, &rv16);
    if (err)
    {
      return ResponseError(rq, err);
    }
    else
    {
      return ResponseU16(rq, rv16);
    }
  }
  else if ((0x1300 <= addr) && (addr < 0x1300 + UIO_ADC_COUNT)) // Set ANA_OUT
  {
    uint8_t idx = addr - 0x0700;

    HandleRw(rq, &dac_value[idx], sizeof(dac_value[0]));  // save to the shadow
    if (!rq->iswrite)
    {
      return true; // already handled
    }

    SetDacOutput(idx,  dac_value[idx]);

    return ResponseOk(rq);
  }

  else if ((0x1400 <= addr) && (addr < 0x0700 + UIO_PWM_COUNT)) // Set PWM Duty Cycle
  {
    uint8_t idx = addr - 0x0700;

    HandleRw(rq, &pwm_value[idx], sizeof(pwm_value[0]));  // save to the shadow
    if (!rq->iswrite)
    {
      return true; // already handled
    }

    SetPwmDuty(idx, pwm_value[idx]);
    return ResponseOk(rq);
  }

  else if ((0x1500 <= addr) && (addr < 0x1500 + UIO_LEDBLP_COUNT))
  {
    uint8_t idx = addr - 0x1500;
    return HandleRw(rq, &ledblp_value[idx], sizeof(ledblp_value[0]));
  }

  // SPI Control
  else if (0x1600 == addr) // SPI Speed
  {
    return HandleRw(rq, &spi_speed, sizeof(spi_speed));
  }
  else if (0x1601 == addr) // SPI transaction length
  {
    return HandleRw(rq, &spi_trlen, sizeof(spi_trlen));
  }
  else if (0x1602 == addr) // SPI status
  {
    if (rq->iswrite)
    {
      rv32 = RqValueU32(rq);
      if (1 == rv32)
      {
        // start the SPI transaction
        err = SpiStart();
        return ResponseError(rq, err); // will be response ok with err=0
      }
      else
      {
        return ResponseError(rq, UIOERR_VALUE);
      }
    }
    else
    {
      return ResponseU8(rq, spi_status);
    }
  }
  else if (0x1604 == addr) // SPI write data MPRAM offset
  {
    return HandleRw(rq, &spi_tx_offs, sizeof(spi_tx_offs));
  }
  else if (0x1605 == addr) // SPI read data MPRAM offset
  {
    return HandleRw(rq, &spi_rx_offs, sizeof(spi_rx_offs));
  }

  // I2C Conrol
  else if (0x1620 == addr) // I2C Speed
  {
    return HandleRw(rq, &i2c_speed, sizeof(i2c_speed));
  }
  else if (0x1621 == addr) // I2C EADDR (Extra Address)
  {
    return HandleRw(rq, &i2c_eaddr, sizeof(i2c_eaddr));
  }
  else if (0x1622 == addr) // I2C Transaction Start
  {
    if (rq->iswrite)
    {
      i2c_cmd = RqValueU32(rq);
      err = I2cStart();
      return ResponseError(rq, err); // will be response ok with err=0
    }
    else
    {
      return ResponseU32(rq, i2c_cmd);
    }
  }
  else if (0x1623 == addr) // I2C Transaction Status / Result
  {
    return ResponseU16(rq, i2c_result);
  }
  else if (0x1624 == addr) // I2C data MPRAM offset
  {
    return HandleRw(rq, &i2c_data_offs, sizeof(i2c_data_offs));
  }

  // MEMORY RANGE

  else if ((0x8000 <= addr) && (addr + rq->length <= 0x8000 + 2 * UIO_ADC_COUNT)) // ADC
  {
    if (rq->iswrite)
    {
      return ResponseError(rq, UIOERR_READ_ONLY);
    }

    if ((addr & 1) || (rq->length & 1))  // odd addresses or length are not allowed
    {
      return ResponseError(rq, UIOERR_WRONG_ACCESS);
    }

    uint8_t idx = ((addr - 0x8000) >> 1);
    uint8_t endidx = idx + (rq->length >> 1);
    uint16_t * dp16 = (uint16_t *)&rq->data[0];
    while (idx < endidx)
    {
      if (0 != GetAdcValue(idx, dp16))
      {
        *dp16 = UIO_ADC_ERROR_VALUE;
      }

      ++dp16;
      ++idx;
    }
    return ResponseOk(rq);
  }

  else if ((0x8100 <= addr) && (addr + rq->length <= 0x8100 + 2 * UIO_DAC_COUNT)) // DAC
  {
    if ((addr & 1) || (rq->length & 1))  // odd addresses or length are not allowed
    {
      return ResponseError(rq, UIOERR_WRONG_ACCESS);
    }

    uint8_t idx = ((addr - 0x8100) >> 1);

    HandleRw(rq, &dac_value[idx], rq->length);
    if (!rq->iswrite)
    {
      return true; // already handled
    }

    // update the dac outputs
    uint8_t endidx = idx + (rq->length >> 1);
    while (idx < endidx)
    {
      SetDacOutput(idx, dac_value[idx]);
      ++idx;
    }

    return ResponseOk(rq);
  }

  else if ((0x8200 <= addr) && (addr + rq->length <= 0x8200 + 2 * UIO_PWM_COUNT)) // PWM
  {
    if ((addr & 1) || (rq->length & 1))  // odd addresses or length are not allowed
    {
      return ResponseError(rq, UIOERR_WRONG_ACCESS);
    }

    uint8_t idx = ((addr - 0x8200) >> 1);

    HandleRw(rq, &pwm_value[idx], rq->length);
    if (!rq->iswrite)
    {
      return true; // already handled
    }

    // update the pwm outputs
    uint8_t endidx = idx + (rq->length >> 1);
    while (idx < endidx)
    {
      SetPwmDuty(idx, pwm_value[idx]);
      ++idx;
    }

    return ResponseOk(rq);
  }

  else if ((0xC000 <= addr) && (addr + rq->length <= 0xC000 + UIO_MPRAM_SIZE)) // MPRAM
  {
    return HandleRw(rq, mpram + (addr - 0xC000), rq->length);
  }

  return false;
}

void TUioGenDevBase::SaveSetup()
{
  TRACE("Saving setup...\r\n");

  TUioCfgStb * pstb = &g_cfgstb;  // use the ram storage

  pstb->signature = UIOCFG_SIGNATURE;

  // copy configs
  pstb->basecfg = basecfg;
  pstb->base_length = sizeof(basecfg);
  pstb->base_csum = uio_content_checksum(&basecfg, sizeof(basecfg));

  pstb->cfg = cfg;
  pstb->cfg_length = sizeof(cfg);
  pstb->cfg_csum = uio_content_checksum(&cfg, sizeof(cfg));

  pstb->_tail_pad = 0x1111111111111111;

  // save to flash
#if 1
  hwintflash.StartCopyMem(nvsaddr_setup, pstb, sizeof(*pstb));
  hwintflash.WaitForComplete();
#else
  hwintflash.StartEraseMem(nvsaddr_setup, sizeof(*pstb));
  hwintflash.WaitForComplete();
  hwintflash.StartWriteMem(nvsaddr_setup, pstb, sizeof(*pstb));
  hwintflash.WaitForComplete();
#endif

  TRACE("Setup save completed.\r\n");
}

void TUioGenDevBase::LoadSetup()
{
  TUioCfgStb * pstb = (TUioCfgStb *)nvsaddr_setup;

  TRACE("Setup size = %u\r\n", sizeof(TUioCfgStb));

  TRACE("Loading Configuration...\r\n");

  if (pstb->signature != UIOCFG_SIGNATURE)
  {
    TRACE("  signature error: %08X\r\n", pstb->signature);
    return;
  }

  if ( (pstb->base_length != sizeof(basecfg)) || (pstb->cfg_length != sizeof(cfg)) )
  {
    TRACE("  config length difference\r\n");
    return;
  }

  if ( pstb->base_csum != uio_content_checksum(&pstb->basecfg, sizeof(pstb->basecfg)) )
  {
    TRACE("  base config checksum error\r\n");
    return;
  }

  if ( pstb->cfg_csum != uio_content_checksum(&pstb->cfg, sizeof(pstb->cfg)) )
  {
    TRACE("  config checksum error\r\n");
    return;
  }

  TRACE("Saved setup ok, activating.\r\n");

  basecfg = pstb->basecfg;
  cfg = pstb->cfg;


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
    return UIOERR_BUSY;
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
    return UIOERR_BUSY;
  }

  if (!i2c)
  {
    return UIOERR_UNITSEL;
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
    i2c->StartWriteData(i2c_cmd & 0x7F, i2c_eaddr | (edata_len << 24), &mpram[i2c_data_offs], i2c_trlen);
  }
  else
  {
    i2c->StartReadData(i2c_cmd & 0x7F, i2c_eaddr | (edata_len << 24), &mpram[i2c_data_offs], i2c_trlen);
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
    if (!i2c->busy)
    {
      i2c_result = i2c->error;
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
