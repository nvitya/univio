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
#include "board_pins.h"

THwSpi           g_spi[UIO_SPI_COUNT];
THwI2c           g_i2c[UIO_I2C_COUNT];
THwUart          g_uart[UIO_UART_COUNT];
THwAdc           g_adc[UIOMCU_ADC_COUNT];
THwPwmChannel    g_pwm[UIO_PWM_COUNT];
TGpioPin         g_pins[UIO_PIN_COUNT];

THwDmaChannel    g_dma_spi_tx[UIO_SPI_COUNT];
THwDmaChannel    g_dma_spi_rx[UIO_SPI_COUNT];

THwDmaChannel    g_dma_i2c_tx[UIO_I2C_COUNT];
THwDmaChannel    g_dma_i2c_rx[UIO_I2C_COUNT];

THwDmaChannel    g_dma_uart_tx[UIO_UART_COUNT];
THwDmaChannel    g_dma_uart_rx[UIO_UART_COUNT];

uint8_t          g_mpram[UIO_MPRAM_SIZE];

TUioCfgStb       g_cfgstb;

void TUioSpiCtrl::Init(TUioDevBase * adevbase, THwSpi * aspi)
{
	devbase = adevbase;
	spi = aspi;
}

void TUioSpiCtrl::UpdateSettings()
{
	bool ichigh = (0 != (spi_mode & (1 << 1)));
	bool dslate = (0 != (spi_mode & (1 << 0)));
  if ((spi->speed != spi_speed) || (ichigh != spi->idleclk_high) || (dslate != spi->datasample_late))
  {
  	spi->idleclk_high    = ichigh;
  	spi->datasample_late = dslate;
    spi->speed = spi_speed;
    spi->Init(spi->devnum); // re-init the device
  }
}

uint16_t TUioSpiCtrl::SpiStart()
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

  UpdateSettings();

  spi->StartTransfer(0, 0, 0, spi_trlen, &devbase->mpram[spi_tx_offs], &devbase->mpram[spi_rx_offs]);

  spi_status = 1;

  return 0;
}

void TUioSpiCtrl::Run()
{
  if (spi_status)
  {
    spi->Run();
    if (spi->finished)
    {
      spi_status = 0;
    }
  }
}

bool TUioSpiCtrl::prfn_SpiControl(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint8_t idx  = (rq->index & 0x1F);

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
  	for (unsigned n = 0; n < devbase->flws_cnt; ++n)
  	{
  		if (!devbase->flwslot[n].busy)
  		{
  			fsc |= (1 << n);
  		}
  	}
  	return udo_ro_uint(rq, fsc, 1);
  }
  else if (0x11 == idx)  // flash command
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
  else if (0x12 == idx)  // flash size
  {
  	return udo_ro_data(rq, &extflash.bytesize, 4);
  }
  else if (0x13 == idx)  // flash size
  {
  	return udo_ro_data(rq, &extflash.idcode, 4);
  }

  return udo_response_error(rq, UDOERR_INDEX);
}


bool TUioSpiCtrl::SpiFlashCmdPrepare()
{
	uint8_t cmd = (spifl_cmd[0] & 0xFF);

	if (0 == spi_status)
	{
		UpdateSettings();
	}

  extflash.spi = spi;

	if (1 == cmd) // Initialize the flash
	{
  	if (0 != spi_status)
  	{
  		return false;
  	}
    extflash.has4kerase = true;
  	extflash.Init();
  	return true;  // finish here
	}
	else if ((2 == cmd) || (3 == cmd))
	{
		uint8_t sidx = ((spifl_cmd[0] >> 16) & 0x7);
		if (sidx >= devbase->flws_cnt)
		{
			return false;
		}

		TUioFlwSlot * pslot = &devbase->flwslot[sidx];
		if (pslot->busy)
		{
			return false;
		}

		pslot->busy = 1;
		pslot->cmd = cmd;
		pslot->slotidx = sidx;
		pslot->fladdr = spifl_cmd[1];
		pslot->next = nullptr;

		// add to queue
		if (devbase->flws_last)
		{
			devbase->flws_last->next = pslot;
			devbase->flws_last = pslot;
		}
		else
		{
			devbase->flws_last  = pslot;
			devbase->flws_first = pslot;
		}
	}
	else
	{
		return false;
	}

	spi_status = 8;  // SPI flash command is running
  return true;
}

void TUioSpiCtrl::SpiFlashRun()
{
	if (!extflash.completed)
	{
		extflash.Run();
		return;
	}

	TUioFlwSlot * pslot;

	#if 0
		// check un-linked busy slots !
		volatile uint8_t fsbm = 0;
		volatile uint8_t fscnt = 0;
		for (unsigned n = 0; n < flws_cnt; ++n)
		{
			if (flwslot[n].busy)
			{
				fsbm |= (1 << n);
				++fscnt;
			}
		}
		volatile uint8_t lcnt = 0;
		pslot = flws_first;
		while (pslot)
		{
			++lcnt;
			pslot = pslot->next;
		}

		if (fscnt != lcnt)
		{
			__NOP();
			__NOP();  // set breakpoint here
			__NOP();
		}
	#endif

	pslot = devbase->flws_first;
	if (!pslot)
	{
		spifl_state = 0;
		spi_status = 0;
		return;
	}

	spi_status = 8;  // SPI flash command is running

	if (0 == spifl_state)  // start the command
	{
    spifl_srcbuf = &devbase->mpram[pslot->slotidx * 4096];
    spifl_wrkbuf = &devbase->mpram[UIO_MPRAM_SIZE - 4096];

    if (3 == pslot->cmd)
    {
    	extflash.StartReadMem(pslot->fladdr, spifl_wrkbuf, 4096);
    	spifl_state = 11;
    }
    else // 2 == cmd, start the write only
    {
  		extflash.StartWriteMem(pslot->fladdr, spifl_srcbuf, 4096);
  		spifl_state = 13;
    }
	}
	else if (11 == spifl_state) // sector read finished, compare the sectors
	{
		// compare memory
		bool   erased = true;
		bool   match = true;
		uint32_t * sp32  = (uint32_t *)(spifl_srcbuf);
		uint32_t * wp32  = (uint32_t *)(spifl_wrkbuf);
		uint32_t * wendptr = (uint32_t *)(spifl_wrkbuf + 4096);

		while (wp32 < wendptr)
		{
			if (*wp32 != 0xFFFFFFFF)  erased = false;
			if (*wp32 != *sp32)
			{
				match = false; // do not break for complete the erased check!
			}

			++wp32;
			++sp32;
		}

		if (match)
		{
			// nothing to do
			SpiFlashSlotFinish();
			return;
		}

		// must be rewritten

		if (!erased)
		{
			extflash.StartEraseMem(pslot->fladdr, 4096);
			spifl_state = 12; // wait until the erease finished.
			return;
		}

		extflash.StartWriteMem(pslot->fladdr, spifl_srcbuf, 4096);
		spifl_state = 13;
	}
	else if (12 == spifl_state) // erase finished, start write
	{
		extflash.StartWriteMem(pslot->fladdr, spifl_srcbuf, 4096);
		spifl_state = 13;
	}
	else if (13 == spifl_state) // write finished.
	{
		SpiFlashSlotFinish();
	}
	else // unhandled state
	{
		SpiFlashSlotFinish();
	}
}

void TUioSpiCtrl::SpiFlashSlotFinish()
{
	spifl_state = 0;

	TUioFlwSlot * pslot = devbase->flws_first;
	if (!pslot)
	{
		return;
	}

	pslot->busy = 0;
	devbase->flws_first = pslot->next;
	if (!devbase->flws_first)
	{
		devbase->flws_last = nullptr;
	}
}


//----------------------------------------------------------------------------------------------------------

void TUioI2cCtrl::Init(TUioDevBase * adevbase, THwI2c * ai2c)
{
	devbase = adevbase;
	i2c = ai2c;
	i2ctra.completed = true;
}

uint16_t TUioI2cCtrl::I2cStart()
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
  }

  // RP2040 bug: re-init (? reset) required sometimes
  i2c->Init(i2c->devnum); // re-init the device

  uint8_t   addr   = (i2c_cmd >> 1) & 0x7F;
  uint32_t  extra  = (i2c_eaddr & 0xFFFFFF) | (edata_len << 24);

  if (i2c_cmd & UIO_I2C_CMD_WRITE)
  {
    i2c->StartWrite(&i2ctra, addr, extra, &devbase->mpram[i2c_data_offs], i2c_trlen);
  }
  else
  {
    i2c->StartRead(&i2ctra,  addr, extra, &devbase->mpram[i2c_data_offs], i2c_trlen);
  }

  i2c_result = 0xFFFF;

  return 0;
}

void TUioI2cCtrl::Run()
{
  if (0xFFFF == i2c_result)
  {
    i2c->Run();
    if (i2ctra.completed)
    {
      i2c_result = i2ctra.error;
    }
  }
}

bool TUioI2cCtrl::prfn_I2cControl(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint8_t idx  = (rq->index & 0x1F);

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
  else if (0x04 == idx) // I2C data MPRAM offset
  {
    return udo_rw_data(rq, &i2c_data_offs, sizeof(i2c_data_offs));
  }

  return udo_response_error(rq, UDOERR_INDEX);
}

//-------------------------------------------------------------------------------------------------

bool TUioDevBase::Init()
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

bool TUioDevBase::InitDevice()
{
  unsigned n;

  mpram = &g_mpram[0];

  g_nvstorage.Init();

  blp_bit_clocks = (SystemCoreClock >> 4);  // 1/16 s

  for (n = 0; n < UIO_SPI_COUNT;  ++n)  spictrl[n].Init(this, &g_spi[n]);
  for (n = 0; n < UIO_I2C_COUNT;  ++n)  i2cctrl[n].Init(this, &g_i2c[n]);
  for (n = 0; n < UIO_UART_COUNT; ++n)  uart[n] = &g_uart[n];

  int pcnt = UIO_MPRAM_SIZE / UIO_FLW_SECTOR_SIZE;
  if (pcnt >= 2)
  {
  	flws_cnt = pcnt - 1;
  }
  else
  {
  	flws_cnt = 0;
  }

  if (flws_cnt > UIO_FLW_SLOT_MAX)  flws_cnt = UIO_FLW_SLOT_MAX;

  for (pcnt = 0; pcnt < flws_cnt; ++pcnt)
  {
  	flwslot[pcnt].busy = 0;
  	flwslot[pcnt].slotidx = pcnt;
  }
  flws_first = nullptr;
  flws_last  = nullptr;

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

uint16_t TUioDevBase::PinSetup(uint8_t pinid, uint32_t pincfg, bool active)
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
				uart_active[0] = true;  // will create the USB-UART port
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

uint16_t TUioDevBase::GetAdcValue(uint8_t adc_idx, uint16_t * rvalue)
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

void TUioDevBase::SaveSetup()
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

void TUioDevBase::LoadSetup()
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

void TUioDevBase::ClearConfig()
{
  unsigned n;

  for (n = 0; n < UIO_UART_COUNT; ++n)
  {
  	uart_active[n] = false;
  }

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

void TUioDevBase::ResetConfig()
{
  unsigned n;

  ConfigurePins(false); // set all pins passive

}

void TUioDevBase::ConfigurePins(bool active)
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

void TUioDevBase::SetPwmDuty(uint8_t apwmnum, uint16_t aduty)
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

void TUioDevBase::SetDacOutput(uint8_t adacnum, uint16_t avalue)
{
  //TODO: implement
}

void TUioDevBase::SetRunMode(uint8_t arunmode)
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

void TUioDevBase::Run() // handle led blink patterns
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

  for (n = 0; n < UIO_SPI_COUNT; ++n)
  {
  	spictrl[n].Run();
  }

  for (n = 0; n < UIO_I2C_COUNT; ++n)
  {
  	i2cctrl[n].Run();
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
