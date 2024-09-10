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
 *  file:     uio_spiflash_control.cpp
 *  brief:    UNIVIO SPI FLASH Accelerator Interface
 *  date:     2024-09-10
 *  authors:  nvitya
*/

#include "board.h"

#if UIO_SPIFLASH_COUNT

#include "uio_spiflash_control.h"
#include "uio_dev_base.h"
#include "board_pins.h"
#include "spiflash.h"

TSpiFlash         extflash;
TUioSpiFlashCtrl  g_spiflash_ctrl;

void TUioSpiFlashCtrl::Init(TUioDevBase * adevbase)
{
  devbase = adevbase;
  spictrl = &g_spictrl[0];

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
}

void TUioSpiFlashCtrl::Run()
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

  pslot = flws_first;
  if (!pslot)
  {
    spifl_state = 0;
    spictrl->spi_status = 0;
    return;
  }

  spictrl->spi_status = 8;  // SPI flash command is running

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

bool TUioSpiFlashCtrl::SpiFlashCmdPrepare()
{
  uint8_t cmd = (spifl_cmd[0] & 0xFF);

  extflash.spi = spictrl->spi;

  if (1 == cmd) // Initialize the flash
  {
    if (0 != spictrl->spi_status)
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
    if (sidx >= flws_cnt)
    {
      return false;
    }

    TUioFlwSlot * pslot = &flwslot[sidx];
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
    if (flws_last)
    {
      flws_last->next = pslot;
      flws_last = pslot;
    }
    else
    {
      flws_last  = pslot;
      flws_first = pslot;
    }
  }
  else
  {
    return false;
  }

  spictrl->spi_status = 8;  // SPI flash command is running
  return true;
}

void TUioSpiFlashCtrl::SpiFlashSlotFinish()
{
  spifl_state = 0;

  TUioFlwSlot * pslot = flws_first;
  if (!pslot)
  {
    return;
  }

  pslot->busy = 0;
  flws_first = pslot->next;
  if (!flws_first)
  {
    flws_last = nullptr;
  }
}

bool TUioSpiFlashCtrl::prfn_SpiFlashControl(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint8_t idx  = (rq->index & 0x0F);

  if (0x0 == idx)  // free slot count
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
  else if (0x1 == idx)  // flash command
  {
    if (!udo_rw_data(rq, &spifl_cmd[0], 8))
    {
      return false;
    }

    if (rq->iswrite)
    {
      if ((spifl_cmd[0] & (1 << 8)) && (UIO_SPI_COUNT > 1))
      {
        spictrl = &g_spictrl[1];
      }
      else
      {
        spictrl = &g_spictrl[0];
      }

      if (1 == spictrl->spi_status) // normal SPI is runing ?
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
  else if (0x2 == idx)  // flash size
  {
    return udo_ro_data(rq, &extflash.bytesize, 4);
  }
  else if (0x3 == idx)  // flash size
  {
    return udo_ro_data(rq, &extflash.idcode, 4);
  }

  return udo_response_error(rq, UDOERR_INDEX);
}

#endif
