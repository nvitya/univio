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
 *  file:     uio_nvdata.cpp
 *  brief:    UNIVIO GENDEV non-volatile data handling
 *  version:  1.00
 *  date:     2022-02-13
 *  authors:  nvitya
*/

#include "string.h"
#include "univio.h"
#include "uio_device.h"
#include <uio_nvdata.h>
#include "uio_nvstorage.h"

TUioNvData g_nvdata;

#if HAS_SPI_FLASH

  // with SPI Flash storage the CPU must have enough RAM to have this relative big buffer

  // this two sector buffer mirrors the SPI Flash content:
  uint8_t nvdata_spi_buf[8192] __attribute__((aligned(8)));
#endif

void TUioNvData::Init()
{
  unsigned n;

  if (!g_nvstorage.initialized)
  {
    g_nvstorage.Init();
  }

  sector_size = g_uiodev.nvs_sector_size;
  nvsaddr_base = g_uiodev.nvsaddr_nvdata;

  chrec_count = (sector_size - sizeof(TUioNvDataHead)) / sizeof(TUioNvDataChRec);

  #if HAS_SPI_FLASH
    // load the two sectors into the memory first, the sector size must be 4096 here

    g_nvstorage.Read(nvsaddr_base, &nvdata_spi_buf[0], sizeof(nvdata_spi_buf));

    TUioNvDataHead *  phead1 = (TUioNvDataHead *)&nvdata_spi_buf[0 * sector_size];
    TUioNvDataHead *  phead2 = (TUioNvDataHead *)&nvdata_spi_buf[1 * sector_size];
  #else
    // two sectors, select the newer one

    TUioNvDataHead *  phead1 = (TUioNvDataHead *)(nvsaddr_base + 0 * sector_size);
    TUioNvDataHead *  phead2 = (TUioNvDataHead *)(nvsaddr_base + 1 * sector_size);
  #endif

  TUioNvDataHead *   phead = nullptr;

  if (UIO_NVDATA_SIGNATURE == phead1->signature)
  {
    phead = phead1;
    sector_index = 0;
  }

  if (UIO_NVDATA_SIGNATURE == phead2->signature)
  {
    if (!phead || (phead->serial < phead2->serial))
    {
      phead = phead2;
      sector_index = 1;
    }
  }

  TUioNvDataHead  ldatahead;

  if (!phead)
  {
    // not initialized yet

    g_nvstorage.Erase(nvsaddr_base, sector_size * 2);

    ldatahead.signature = UIO_NVDATA_SIGNATURE;
    ldatahead.serial = 1;
    memcpy(&ldatahead.value[0], &value[0], sizeof(value));

    g_nvstorage.Write(nvsaddr_base, &ldatahead,  sizeof(ldatahead));

    sector_index = 0;

    #if HAS_SPI_FLASH
      // update the local buffer
      memset(&nvdata_spi_buf[0], 0xFF, sizeof(nvdata_spi_buf));
      memcpy(&nvdata_spi_buf[0], &ldatahead,  sizeof(ldatahead));
    #endif
  }
  else
  {
    // load the initial data
    memcpy(&value[0], &phead->value[0], sizeof(value));

    // process the change recs
    TUioNvDataChRec *  prec = (TUioNvDataChRec *)(phead + 1);  // the first changerec follows the header
    n = 0;
    while (n < chrec_count)
    {
      if ((prec->id == (prec->id_not ^ 0xFF)) && (prec->id >= 0) && (prec->id < UIO_NVDATA_COUNT))
      {
        value[prec->id] = prec->value;
      }
      else
      {
        break;
      }
      ++prec;
      ++n;
    }
  }
}

uint16_t TUioNvData::SaveValue(uint8_t aid, uint32_t avalue)
{
  if (lock != UIO_NVDATA_UNLOCK)
  {
    return UIOERR_READ_ONLY;
  }

  aid = (aid & 0x1F);  // ensure safe index

  if (value[aid] == avalue)
  {
    return 0;
  }

  value[aid] = avalue; // update locally first

  #if HAS_SPI_FLASH
    TUioNvDataHead *  phead = (TUioNvDataHead *)&nvdata_spi_buf[sector_size * sector_index];
    uint8_t * pbase = (uint8_t *)&nvdata_spi_buf[0];
  #else
    TUioNvDataHead *  phead = (TUioNvDataHead *)(nvsaddr_base + sector_size * sector_index);
    uint8_t * pbase = (uint8_t *)nvsaddr_base;
  #endif

  // search an empty (0xFF) record

  TUioNvDataChRec *  prec = (TUioNvDataChRec *)(phead + 1);  // the address after the header
  unsigned n = 0;
  while (n < chrec_count)
  {
    if ((0xFF == prec->id) && (0xFF == prec->id_not) && (0xFFFFFFFF == prec->value))
    {
      // found an empty record, write there

      TUioNvDataChRec lrec;
      memset(&lrec, 0xFF, sizeof(lrec));
      lrec.id = aid;
      lrec.id_not = (aid ^ 0xFF);
      lrec.value = avalue;

      g_nvstorage.Write(nvsaddr_base + unsigned(prec) - unsigned(pbase), &lrec,  sizeof(lrec));
      #if HAS_SPI_FLASH
        // update the local buffer too, the prec points there
        memcpy(prec, &lrec,  sizeof(lrec));
      #endif

      return 0;
    }
    ++prec;
    ++n;
  }

  // no empty record was found, erase the other sector and write there

  TUioNvDataHead  ldatahead;
  ldatahead.signature = UIO_NVDATA_SIGNATURE;
  ldatahead.serial = phead->serial + 1;
  memcpy(&ldatahead.value[0], &value[0], sizeof(value));

  sector_index = (sector_index ^ 1); // change to the other sector

  g_nvstorage.Erase(nvsaddr_base + sector_size * sector_index, sector_size);
  g_nvstorage.Write(nvsaddr_base + sector_size * sector_index, &ldatahead,  sizeof(ldatahead));

  #if HAS_SPI_FLASH
    // update the local buffer too
    memset(&nvdata_spi_buf[sector_size * sector_index], 0xFF, sector_size);
    memcpy(&nvdata_spi_buf[sector_size * sector_index], &ldatahead,  sizeof(ldatahead));
  #endif

  return 0;
}
