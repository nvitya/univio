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
 *  file:     uio_gendev_base.cpp (Arduino)
 *  brief:    UNIVIO GENDEV target independent parts
 *  created:  2022-02-13
 *  authors:  nvitya
*/

#include "string.h"

#include "Arduino.h"
#include "uio_gendev_base.h"
#include "uio_nvdata.h"
#include "traces.h"
#include "SPIFFS.h"

THwAdc           g_adc[UIOMCU_ADC_COUNT];
THwPwmChannel    g_pwm[UIO_PWM_COUNT];
TGpioPin         g_pins[UIO_PIN_COUNT];

uint8_t          g_mpram[UIO_MPRAM_SIZE];

bool TUioGenDevBase::Init()
{
  initialized = false;

  // initialize configuration defaults
  cfg.usb_vendor_id = 0xDEAD;
  cfg.usb_product_id = 0xBEEF;

  strncpy(cfg.device_id, UIO_HW_ID, sizeof(cfg.device_id));
  strncpy(cfg.manufacturer, "github.com/nvitya/univio", sizeof(cfg.manufacturer));
  strncpy(cfg.serial_number, "1", sizeof(cfg.serial_number));

  cfg.ip_address.Set(192, 168, 2, 20);
  cfg.net_mask.Set(255, 255, 255, 0);
  cfg.gw_address.Set(0, 0, 0, 0);
  cfg.dns.Set(0, 0, 0, 0);
  cfg.dns2.Set(0, 0, 0, 0);

  cfg.wifi_ssid[0] = 0;
  cfg.wifi_password[0] = 0;

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

  // prepare g_pins

  for (n = 0; n < UIO_PIN_COUNT; ++n)
  {
    uint8_t   portnum = (n / UIO_PINS_PER_PORT);
    uint8_t   pinnum  = (n & (UIO_PINS_PER_PORT - 1));

    g_pins[n].Assign(pinnum, false);
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
  pcf.hwpinflags = PINCFG_INPUT | PINCFG_PULLUP;  // for passive pins

  pcf.pintype = 0; // check for reserved pins first
  if (!PinFuncAvailable(&pcf))  // do not touch reserved pins !
  {
    if (0 == pintype)
    {
      return 0;
    }
    else
    {
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
    return UIOERR_FUNC_NOT_AVAIL;
  }

  if (UIO_PINTYPE_PASSIVE == pintype) // passive = input with pullup
  {
    // keep the previously set passive value
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
      ppin->Assign(ppin->pinnum, true);   // reassign required because of the inverted
    }
    else
    {
      ppin->Assign(ppin->pinnum, false);  // reassign required because of the inverted
    }

    if (initial_1)
    {
      pcf.hwpinflags |= PINCFG_GPIO_INIT_1;
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
      spi_active = true;
      SetupSpi(&pcf);
    }
    cfginfo[UIO_INFOIDX_CBITS] |= UIO_INFOCBIT_SPI;
  }

  else if (UIO_PINTYPE_I2C == pintype)
  {
    if (active)
    {
      i2c_active = true;
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
    if (IGNORE_PINFLAGS != pcf.hwpinflags)
    {
      ppin->Setup(pcf.hwpinflags);
    }
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

  // prepare signature, checksum
  cfg.signature = UIOCFG_V2_SIGNATURE;
  cfg.length = sizeof(TUioCfgStb);
  cfg.checksum = 0;
  cfg.checksum = uio_content_checksum(&cfg, sizeof(cfg));

  File file = SPIFFS.open("/univio-config.bin", FILE_WRITE);
  if (!file)
  {
    TRACE("Error writing config file!\r\n");
    return;
  }

  if (file.write((const uint8_t *)&cfg, sizeof(cfg)))
  {
    TRACE("Setup save completed.\r\n");
  }
  else
  {
    TRACE("Setup save failed!\r\n");
  }

  file.close();
}

TUioCfgStb  loaded_cfg __attribute__((aligned(4)));  // local buffer for config loading

void TUioGenDevBase::LoadSetup()
{
  File file = SPIFFS.open("/univio-config.bin", FILE_READ);
  if (!file)
  {
    TRACE("Error reading config file!\r\n");
    return;
  }

  if (!file.read((uint8_t *)&loaded_cfg, sizeof(loaded_cfg)))
  {
    file.close();
    TRACE("Setup load failed!\r\n");
    return;
  }

  file.close();

  TRACE("Setup size = %u\r\n", sizeof(TUioCfgStb));

  if (loaded_cfg.signature != UIOCFG_V2_SIGNATURE)
  {
    TRACE("  signature error: %08X\r\n", loaded_cfg.signature);
    return;
  }

  if (loaded_cfg.length != sizeof(TUioCfgStb))
  {
    TRACE("  config length difference\r\n");
    return;
  }

  if (0 != uio_content_checksum(&loaded_cfg, sizeof(TUioCfgStb)))
  {
    TRACE("  config checksum error\r\n");
    return;
  }

  TRACE("Saved setup ok, activating.\r\n");

  cfg = loaded_cfg; // copy to the active configuration

  // TODO: load runmode
  runmode = 1;
  ConfigurePins(true);
}

void TUioGenDevBase::ClearConfig()
{
  unsigned n;

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

  // clear SPI pin configurations
  spi_active = false;
  spi_devcfg.spics_io_num    = -1;  //PIN_SPI_CS;  // CS pin
  spi_buscfg.mosi_io_num     = -1; // PIN_SPI_MOSI;
  spi_buscfg.miso_io_num     = -1; // PIN_SPI_MISO;
  spi_buscfg.sclk_io_num     = -1; // PIN_SPI_CLK;

  i2c_active = false;
  i2c_conf.scl_io_num = -1;
  i2c_conf.sda_io_num = -1;

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

  // 3. setup special peripherals, like SPI, I2C, as they have own pin routing
  SetupSpecialPeripherals(active);
}

uint16_t TUioGenDevBase::SpiStart()
{
  if (spi_status)
  {
    return UDOERR_BUSY;
  }

  if (!spi_active)
  {
    return UIOERR_UNITSEL;
  }

  if ((0 == spi_speed) || (spi_trlen == 0)
       || (spi_trlen > UIO_MPRAM_SIZE - spi_rx_offs) || (spi_trlen > UIO_MPRAM_SIZE - spi_tx_offs))
  {
    return UIOERR_UNIT_PARAMS;
  }

  if ((0 == spi_devcfg.clock_speed_hz) || (spi_devcfg.clock_speed_hz != spi_speed))
  {
    /* remove is not necessary 
    if (spi_devcfg.clock_speed_hz)
    {
      spi_bus_remove_device(spih);
    }
    */

    spi_devcfg.mode = 0;                      // SPI mode 0
    spi_devcfg.clock_speed_hz = spi_speed;  
    //spi_devcfg.spics_io_num = ...;          // was already set before
    spi_devcfg.queue_size = 4;              

    spi_bus_add_device(SPI2_HOST, &spi_devcfg, &spih);
  }

  memset(&spi_trans, 0, sizeof(spi_trans));
  spi_trans.user = (void*)0;
  spi_trans.flags = 0;
  spi_trans.tx_buffer = &mpram[spi_tx_offs];
  spi_trans.rx_buffer = &mpram[spi_rx_offs];
  spi_trans.length = spi_trlen * 8;

  spi_device_queue_trans(spih, &spi_trans, portMAX_DELAY);  // will run in the background
  spi_status = 1;  // activates the completition polling

  //spi_device_transmit(spih, &spi_trans); // this blocks until finishes
  //spi_status = 0;

  return 0;
}

/*

    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.scl_io_num = (gpio_num_t)PIN_I2C_SCL;
    i2c_conf.sda_io_num = (gpio_num_t)PIN_I2C_SDA;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = 100000;
    i2c_conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL; //Any one clock source that is available for the specified frequency may be choosen

    ret = i2c_driver_install((i2c_port_t)I2C_DEV_NUM, conf.mode, 0, 0, 0);
    Serial.printf("I2C driver install result=%i\r\n", ret);

*/

void TUioGenDevBase::I2cWorkerThread()
{
  i2c_conf.master.clk_speed = i2c_speed;
  i2c_param_config(i2c_port, &i2c_conf);

  i2c_cmdh = i2c_cmd_link_create();

  unsigned  ealen = ((i2c_transpar >> 12) & 3);
  unsigned  trlen = (i2c_transpar >> 16);
  bool      iswrite = ((i2c_transpar & 1) != 0);
  uint8_t   devaddr_x2 = (i2c_transpar & 0xFE);

  i2c_master_start(i2c_cmdh);

  if (!iswrite && (0 == ealen))
  {
    // issue the read only
    i2c_master_write_byte(i2c_cmdh, devaddr_x2 | I2C_MASTER_READ, true);
    i2c_master_read(i2c_cmdh, &mpram[i2c_data_offs], trlen, I2C_MASTER_LAST_NACK);
  }
  else // write first
  {
    i2c_master_write_byte(i2c_cmdh, devaddr_x2 | I2C_MASTER_WRITE, true);
    if (ealen)
    {
      i2c_master_write(i2c_cmdh, (const uint8_t *)&i2c_eaddr, ealen, true);
    }
    if (!iswrite)
    {
      i2c_master_start(i2c_cmdh);
      i2c_master_write_byte(i2c_cmdh, devaddr_x2 | I2C_MASTER_READ, true);
      i2c_master_read(i2c_cmdh, &mpram[i2c_data_offs], trlen, I2C_MASTER_LAST_NACK);      
    }
    else
    {
      i2c_master_write(i2c_cmdh, &mpram[i2c_data_offs], trlen, true);
    }
  }

  i2c_master_stop(i2c_cmdh);

  esp_err_t ret = i2c_master_cmd_begin(i2c_port, i2c_cmdh, 1000 / portTICK_RATE_MS);  

  // cleanup
  i2c_cmd_link_delete(i2c_cmdh);

  if (ret != 0)
  {
    i2c_status = 0x1001;  // some ack or other error happened
  }
  else
  {
    i2c_status = 0;  // finished successfully
  }
  
  i2c_running = false;
  vTaskDelete(nullptr); // deletes this task
}

void thead_i2c_proc(void * arg)
{
  ((TUioGenDevBase *)arg)->I2cWorkerThread();
}

uint16_t TUioGenDevBase::I2cStart()
{
  if (i2c_running)
  {
    return UDOERR_BUSY;
  }

  unsigned i2c_trlen = (i2c_transpar >> 16);

  if ((0 == i2c_speed) || (i2c_trlen == 0) || (i2c_trlen > UIO_MPRAM_SIZE - i2c_data_offs))
  {
    return UIOERR_UNIT_PARAMS;
  }

  i2c_running = true;
  i2c_status = 0xFFFF;  // signalize running status

  xTaskCreate(thead_i2c_proc, "I2C-Worker", 4096, this, 10, nullptr);

  return 0;
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
  unsigned t0 = micros();

  if (t0 - last_blp_us >= blp_bit_us)
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

    last_blp_us = t0;
  }

  if (spi_status)
  {
    spi_transaction_t * rtrans;
    if (ESP_OK == spi_device_get_trans_result(spih, &rtrans, 0))
    {
      spi_status = 0;      
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
