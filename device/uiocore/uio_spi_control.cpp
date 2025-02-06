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
 *  file:     uio_spi_control.cpp
 *  brief:    UNIVIO SPI controller interface
 *  date:     2024-09-10
 *  authors:  nvitya
*/

#include "uio_spi_control.h"
#include "uio_dev_base.h"

THwSpi        g_spi[UIO_SPI_COUNT];
TUioSpiCtrl   g_spictrl[UIO_SPI_COUNT];

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
    	if (1 == spi_status)  // the SPI Flash accelerator also controls the SPI with spi_status=8
    	{
    		spi_status = 0;
    	}
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

  return udo_response_error(rq, UDOERR_INDEX);
}

