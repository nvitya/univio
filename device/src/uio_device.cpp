/* -----------------------------------------------------------------------------
 * This file is a part of the UNIVIO project: https://github.com/nvitya/univio
 * Copyright (c) 2023 Viktor Nagy, nvitya
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
 *  file:     uio_device.cpp
 *  brief:    UNIVIO device final instance, UDO request handling
 *  created:  2023-05-13
 *  authors:  nvitya
*/

#include "uio_device.h"
#include "uio_gendev_version.h"
#include "uio_nvdata.h"
#include "board_pins.h"

TUioDevice g_uiodev;

bool TUioDevice::prfn_0100_DevId(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint16_t index = rq->index;

  switch (index)
  {
    case 0x0100:  return udo_response_cstring(rq, UIO_DEVICE_TYPE_ID);
    case 0x0101:  return udo_response_cstring(rq, UIO_FW_ID);
    case 0x0102:  return udo_ro_uint(rq, UIO_VERSION_INTEGER, 4);

    case 0x0110:  return udo_ro_uint(rq, UIO_PIN_COUNT, 1);
    case 0x0111:  return udo_ro_uint(rq, UIO_PINS_PER_PORT, 1);
    case 0x0112:  return udo_ro_uint(rq, UIO_MPRAM_SIZE, 4);
  }

  return udo_response_error(rq, UDOERR_INDEX);
}

bool TUioDevice::prfn_0180_DevConf(TUdoRequest * rq, TParamRangeDef * prdef)
{
  switch (rq->index)
  {
    case 0x0180: // set config / run mode
    {
      if (!rq->iswrite)
      {
        return udo_ro_uint(rq, runmode, 1);
      }

      uint8_t rmv = udorq_uintvalue(rq);
      if (rmv > 1) // special command
      {
        if (2 == rmv)
        {
          // TODO: restart
          return udo_response_error(rq, UDOERR_NOT_IMPLEMENTED);
        }
        return udo_response_error(rq, UDOERR_WRITE_VALUE);
      }

      SetRunMode(rmv);

      return udo_response_ok(rq);
    }
    case 0x0181:   return udo_rw_data_zp(rq, &cfg.device_id[0],     sizeof(cfg.device_id));
    case 0x0182:   return udo_rw_data(rq, &cfg.usb_vendor_id,    sizeof(cfg.usb_vendor_id));
    case 0x0183:   return udo_rw_data(rq, &cfg.usb_product_id,   sizeof(cfg.usb_product_id));
    case 0x0184:   return udo_rw_data_zp(rq, &cfg.manufacturer[0],  sizeof(cfg.manufacturer));
    case 0x0185:   return udo_rw_data_zp(rq, &cfg.serial_number[0], sizeof(cfg.serial_number));
  }

  return udo_response_error(rq, UDOERR_INDEX);
}

bool TUioDevice::prfn_PinCfgReset(TUdoRequest * rq, TParamRangeDef * prdef)
{
  if (!rq->iswrite)  return udo_response_error(rq, UDOERR_WRITE_ONLY);

  if (1 == udorq_uintvalue(rq))
  {
    ResetConfig();
  }
  return udo_response_ok(rq);
}

bool TUioDevice::prfn_PinConfig(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint16_t pinid = (rq->index & 0xFF);
  if (pinid < UIO_PIN_COUNT)
  {
    if (rq->iswrite)
    {
      if (runmode)
      {
        return udo_response_error(rq, UIOERR_RUN_MODE);
      }

      uint32_t pcf = udorq_uintvalue(rq);
      uint16_t err = PinSetup(pinid, pcf, false);
      if (err)
      {
        return udo_response_error(rq, err);
      }

      cfg.pinsetup[pinid] = pcf;
      return udo_response_ok(rq);
    }
    else
    {
      return udo_ro_uint(rq, cfg.pinsetup[pinid], 4);
    }
  }

  return udo_response_error(rq, UIOERR_UNITSEL);
}

bool TUioDevice::prfn_DefValue_DigOut(TUdoRequest * rq, TParamRangeDef * prdef)
{
  return udo_rw_data(rq, &cfg.dv_douts, sizeof(cfg.dv_douts));
}

bool TUioDevice::prfn_DefValue_AnaOut(TUdoRequest * rq, TParamRangeDef * prdef)
{
  unsigned unitid = (rq->index & 0x1F);
  if (unitid >= UIO_DAC_COUNT)
  {
    return udo_response_error(rq, UIOERR_UNITSEL);
  }

  return udo_rw_data(rq, &cfg.dv_dac[unitid], sizeof(cfg.dv_dac[0]));
}

bool TUioDevice::prfn_DefValue_PwmDuty(TUdoRequest * rq, TParamRangeDef * prdef)
{
  unsigned unitid = (rq->index & 0x1F);
  if (unitid >= UIO_PWM_COUNT)
  {
    return udo_response_error(rq, UIOERR_UNITSEL);
  }

  return udo_rw_data(rq, &cfg.dv_pwm[unitid], sizeof(cfg.dv_pwm[0]));
}

bool TUioDevice::prfn_DefValue_PwmFreq(TUdoRequest * rq, TParamRangeDef * prdef)
{
  unsigned unitid = (rq->index & 0x1F);
  if (unitid >= UIO_PWM_COUNT)
  {
    return udo_response_error(rq, UIOERR_UNITSEL);
  }

  udo_rw_data(rq, &cfg.pwm_freq[unitid], sizeof(cfg.pwm_freq[0]));  // save to the shadow
  if (!rq->iswrite)
  {
    return true; // already handled
  }

  THwPwmChannel * pwm = pwmch[unitid];
  if (pwm && pwm->initialized)
  {
    pwm->SetFrequency(cfg.pwm_freq[unitid]);  // update at the actual unit too
  }
  return udo_response_ok(rq);
}

bool TUioDevice::prfn_DefValue_LedBlp(TUdoRequest * rq, TParamRangeDef * prdef)
{
  unsigned unitid = (rq->index & 0x1F);
  if (unitid >= UIO_LEDBLP_COUNT)
  {
    return udo_response_error(rq, UIOERR_UNITSEL);
  }

  return udo_rw_data(rq, &cfg.dv_ledblp[unitid], sizeof(cfg.dv_ledblp[0]));
}

bool TUioDevice::prfn_ConfigInfo(TUdoRequest * rq, TParamRangeDef * prdef)
{
  unsigned idx = (rq->index & 0xFF);

  if (idx >= UIO_INFO_COUNT)
  {
    return udo_response_error(rq, UDOERR_INDEX);
  }

  return udo_ro_data(rq, &cfginfo[idx], sizeof(cfginfo[idx]));
}

bool TUioDevice::prfn_NvData(TUdoRequest * rq, TParamRangeDef * prdef)
{
  unsigned idx = (rq->index & 0xFF);

  if (0x80 == idx) // NVDATA LOCK
  {
    return udo_rw_data(rq, &g_nvdata.lock, sizeof(g_nvdata.lock));
  }

  if (idx >= UIO_NVDATA_COUNT)
  {
    return udo_response_error(rq, UDOERR_INDEX);
  }

  if (rq->iswrite)
  {
    uint32_t rv32 = udorq_uintvalue(rq);
    return udo_response_error(rq, g_nvdata.SaveValue(idx, rv32));
  }

  return udo_ro_uint(rq, g_nvdata.value[idx], 4);
}

bool TUioDevice::prfn_DigOutSetClr(TUdoRequest * rq, TParamRangeDef * prdef)
{
  if (!rq->iswrite)
  {
    return udo_response_error(rq, UDOERR_WRITE_ONLY);
  }

  unsigned idx = (rq->index & 0x01);
  uint32_t rv32 = udorq_uintvalue(rq);

  //TRACE("DOUT(%04X) <- %08X\r\n", rq->index, rv32);

  for (unsigned n = 0; n < 16; ++n)
  {
    unsigned pnum = n + (16 * idx);
    if (pnum < UIO_DOUT_COUNT)
    {
      TGpioPin *  ppin = dig_out[pnum];
      if (ppin)
      {
        uint32_t smask = (1 << n);
        uint32_t cmask = (1 << (n + 16));
        if (rv32 & smask)
        {
          ppin->Set1();
          dout_value |= (1 << pnum);
        }
        else if (rv32 & cmask)
        {
          ppin->Set0();
          dout_value &= ~(1 << pnum);
        }
      }
    }
  }

  return udo_response_ok(rq);
}

bool TUioDevice::prfn_DigOutDirect(TUdoRequest * rq, TParamRangeDef * prdef)
{
  if (!rq->iswrite)
  {
    return udo_ro_uint(rq, dout_value, 4);
  }

  dout_value = udorq_uintvalue(rq);

  for (unsigned n = 0; n < UIO_DOUT_COUNT; ++n)
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

  return udo_response_ok(rq);
}

bool TUioDevice::prfn_DigInValues(TUdoRequest * rq, TParamRangeDef * prdef)
{
  if (rq->iswrite)
  {
    return udo_response_error(rq, UDOERR_READ_ONLY);
  }

  uint32_t rv32 = 0;
  for (unsigned n = 0; n < UIO_DIN_COUNT; ++n)
  {
    TGpioPin *  ppin = dig_in[n];
    uint32_t    pmask = (1 << n);
    if (ppin && ppin->Value())
    {
      rv32 |= pmask;
    }
  }
  return udo_ro_uint(rq, rv32, 4);
}

bool TUioDevice::prfn_AnaInValues(TUdoRequest * rq, TParamRangeDef * prdef)
{
  if (rq->iswrite)
  {
    return udo_response_error(rq, UDOERR_READ_ONLY);
  }

  uint8_t idx = (rq->index & 0x1F);
  uint16_t rv16;
  uint16_t err;

  err = GetAdcValue(idx, &rv16); // handles non-existing unit too
  if (err)
  {
    return udo_response_error(rq, err);
  }
  else
  {
    return udo_ro_uint(rq, rv16, 2);
  }
}

bool TUioDevice::prfn_AnaOutCtrl(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint8_t idx  = (rq->index & 0x1F);
  uint8_t func = (rq->index & 0xE0);

  if (idx >= UIO_DAC_COUNT)
  {
    return udo_response_error(rq, UIOERR_UNITSEL);
  }

  if (0x00 == func)
  {
    if (!udo_rw_data(rq, &dac_value[idx], sizeof(dac_value[0]))) // save to the shadow
    {
      return false;
    }

    if (rq->iswrite)
    {
      SetDacOutput(idx,  dac_value[idx]);
    }

    return udo_response_ok(rq);
  }
  else if (0x20 == func) // Waveform type
  {
    // not handled yet
  }
  else if (0x40 == func) // Amplitude
  {
    // not handled yet
  }
  else if (0x60 == func) // Frequency
  {
    // not handled yet
  }

  return udo_response_error(rq, UDOERR_INDEX);
}

bool TUioDevice::prfn_PwmControl(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint8_t idx  = (rq->index & 0x1F);
  uint8_t func = (rq->index & 0xE0);

  if (idx >= UIO_PWM_COUNT)
  {
    return udo_response_error(rq, UIOERR_UNITSEL);
  }

  if (0x00 == func) // periodic mode
  {
    if (!udo_rw_data(rq, &pwm_value[idx], sizeof(pwm_value[0]))) // save to the shadow
    {
      return false;
    }

    if (rq->iswrite)
    {
      SetPwmDuty(idx, pwm_value[idx]);
    }

    return udo_response_ok(rq);
  }
  else if (0x20 == func) // single pulse mode
  {
    // not handled yet
  }

  return udo_response_error(rq, UDOERR_INDEX);
}

bool TUioDevice::prfn_LedBlpCtrl(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint8_t idx  = (rq->index & 0x1F);
  if (idx >= UIO_LEDBLP_COUNT)
  {
    return udo_response_error(rq, UIOERR_UNITSEL);
  }

  return udo_rw_data(rq, &ledblp_value[idx], sizeof(ledblp_value[0]));
}

bool TUioDevice::prfn_SpiControl(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint8_t idx  = (rq->index & 0xFF);

  if (0x00 == idx) // SPI Settings
  {
  	if (1 == rq->offset)
  	{
  		rq->offset = 0; // override the offset !
  		return udo_rw_data(rq, &spi_mode, sizeof(spi_mode));
  	}
  	else // SPI Speed
  	{
      return udo_rw_data(rq, &spi_speed, sizeof(spi_speed));
  	}
  }
  else if (0x01 == idx) // SPI transaction length
  {
    return udo_rw_data(rq, &spi_trlen, sizeof(spi_trlen));
  }
  else if (0x02 == idx) // SPI status
  {
    if (rq->iswrite)
    {
      uint32_t rv32 = udorq_uintvalue(rq);
      if (1 == rv32)
      {
        // start the SPI transaction
        uint16_t err = SpiStart();
        return udo_response_error(rq, err); // will be response ok with err=0
      }
      else
      {
        return udo_response_error(rq, UDOERR_WRITE_VALUE);
      }
    }
    else
    {
      return udo_ro_uint(rq, spi_status, 1);
    }
  }
  else if (0x04 == idx) // SPI write data MPRAM offset
  {
    return udo_rw_data(rq, &spi_tx_offs, sizeof(spi_tx_offs));
  }
  else if (0x05 == idx) // SPI read data MPRAM offset
  {
    return udo_rw_data(rq, &spi_rx_offs, sizeof(spi_rx_offs));
  }

  // SPI Flash Write Accelerator
  else if (0x10 == idx)  // free slot count
  {
  	uint8_t fsc = 0;
  	for (unsigned n = 0; n < flws_cnt; ++n)
  	{
  		if (!flwslot[n].busy)
  		{
  			fsc |= (1 << n);
  		}
  	}
  	return udo_ro_uint(rq, fsc, 1);
  }
  else if (0x11 == idx)  // flash size
  {
  	return udo_ro_data(rq, &extflash.bytesize, 4);
  }
  else if (0x12 == idx)  // flash command
  {
  	if (!udo_rw_data(rq, &spifl_cmd[0], 8))
  	{
  		return false;
  	}

    if (rq->iswrite)
    {
    	if (1 == spi_status) // normal SPI is runing ?
    	{
    		return udo_response_error(rq, UDOERR_BUSY);
    	}

    	if (!SpiFlashCmdPrepare())  // sets the spi_status and spifl_state !
    	{
    		return udo_response_error(rq, UDOERR_WRITE_VALUE);
    	}

      return udo_response_ok(rq);
    }
  }

  return udo_response_error(rq, UDOERR_INDEX);
}

bool TUioDevice::prfn_I2cControl(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint8_t idx  = (rq->index & 0xFF);

  if (0x00 == idx) // I2C Speed
  {
    return udo_rw_data(rq, &i2c_speed, sizeof(i2c_speed));
  }
  else if (0x01 == idx) // I2C EADDR (Extra Address)
  {
    return udo_rw_data(rq, &i2c_eaddr, sizeof(i2c_eaddr));
  }
  else if (0x02 == idx) // I2C Transaction Start
  {
    if (rq->iswrite)
    {
      i2c_cmd = udorq_uintvalue(rq);
      uint16_t err = I2cStart();
      return udo_response_error(rq, err); // will be response ok with err=0
    }
    else
    {
      return udo_ro_uint(rq, i2c_cmd, 1);
    }
  }
  else if (0x03 == idx) // I2C Transaction Status / Result
  {
    return udo_rw_data(rq, &i2c_result, sizeof(i2c_result));
  }
  else if (0x04 == idx) // SPI read data MPRAM offset
  {
    return udo_rw_data(rq, &i2c_data_offs, sizeof(i2c_data_offs));
  }

  return udo_response_error(rq, UDOERR_INDEX);
}

bool TUioDevice::prfn_Mpram(TUdoRequest * rq, TParamRangeDef * prdef)
{
  return udo_rw_data(rq, mpram, UIO_MPRAM_SIZE);
}

#if 0

bool TUioGenDevBase::HandleDeviceRequest(TUnivioRequest * rq)
{
  unsigned    n;
  uint32_t    rv32;
  uint16_t    rv16;
  uint16_t    err;

  uint16_t addr = rq->address;

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

  return false;
}

#endif
