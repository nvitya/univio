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
 *  file:     uio_spiflash_control.h
 *  brief:    UNIVIO SPI FLASH Accelerator Interface
 *  date:     2024-09-10
 *  authors:  nvitya
*/

#ifndef UIOCORE_UIO_SPIFLASH_CONTROL_H_
#define UIOCORE_UIO_SPIFLASH_CONTROL_H_

#include "uio_common.h"

#if UIO_SPIFLASH_COUNT

#include "tclass.h"
#include "udo.h"
#include "udoslave.h"
#include "simple_partable.h"
#include "uio_spi_control.h"

#include "hwspi.h"
#include "hwdma.h"

#define UIO_FLW_SLOT_MAX      4
#define UIO_FLW_SECTOR_SIZE   4096

class TUioDevBase;

typedef struct TUioFlwSlot
{
  uint8_t           busy;
  uint8_t           cmd;          // 3 = copy, 1 = flash 4k sector only
  uint8_t           slotidx;
  uint8_t           _reserved[1];
  uint32_t          fladdr;
  TUioFlwSlot *     next;
//
} TUioFlwSlot;

class TUioSpiFlashCtrl : public TClass
{
public:
  TUioDevBase *     devbase = nullptr;
  TUioSpiCtrl *     spictrl = nullptr;

  uint8_t           flws_cnt = 0;
  TUioFlwSlot       flwslot[UIO_FLW_SLOT_MAX];
  TUioFlwSlot *     flws_first = nullptr;
  TUioFlwSlot *     flws_last  = nullptr;

  uint8_t           spifl_state = 0;
  uint32_t          spifl_cmd[2] = {0, 0};
  uint8_t *         spifl_wrkbuf = nullptr;
  uint8_t *         spifl_srcbuf = nullptr;

  void              Init(TUioDevBase * adevbase);
  void              Run();
  uint16_t          SpiStart();
  void              UpdateSettings();
  bool              prfn_SpiFlashControl(TUdoRequest * rq, TParamRangeDef * prdef);

  bool              SpiFlashCmdPrepare();
  void              SpiFlashSlotFinish();
};

extern TUioSpiFlashCtrl  g_spiflash_ctrl;

#endif

#endif /* UIOCORE_UIO_SPIFLASH_CONTROL_H_ */
