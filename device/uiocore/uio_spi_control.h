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
 *  file:     uio_spi_control.h
 *  brief:    UNIVIO SPI controller interface
 *  date:     2024-09-10
 *  authors:  nvitya
*/

#ifndef UIOCORE_UIO_SPI_CONTROL_H_
#define UIOCORE_UIO_SPI_CONTROL_H_

#include "tclass.h"
#include "udo.h"
#include "udoslave.h"
#include "simple_partable.h"

#include "hwspi.h"
#include "hwdma.h"

class TUioDevBase;

class TUioSpiCtrl : public TClass
{
public:
  TUioDevBase *     devbase = nullptr;
  THwSpi *          spi = nullptr;
  uint16_t          spi_rx_offs = 0;
  uint16_t          spi_tx_offs = 0;
  uint32_t          spi_speed = 1000000;
  uint16_t          spi_trlen = 0;
  uint8_t           spi_status = 0;
  uint8_t           spi_mode = 0;

  void              Init(TUioDevBase * adevbase, THwSpi * aspi);
  void              Run();
  uint16_t          SpiStart();
  void              UpdateSettings();
  bool              prfn_SpiControl(TUdoRequest * rq, TParamRangeDef * prdef);
};

extern THwSpi       g_spi[UIO_SPI_COUNT];
extern TUioSpiCtrl  g_spictrl[UIO_SPI_COUNT];

#endif /* UIOCORE_UIO_SPI_CONTROL_H_ */
