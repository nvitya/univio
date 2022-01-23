/*
 * uio_nvdata.cpp
 *
 *  Created on: Dec 16, 2021
 *      Author: vitya
 */

#include "string.h"
#include "hwintflash.h"
#include "univio.h"
#include "uio_gendev.h"
#include <uio_nvdata.h>

TUioNvData g_nvdata;

void TUioNvData::Init()
{
  unsigned n;

  if (!hwintflash.initialized)
  {
    hwintflash.Init();
  }

  sector_size = g_uiodev.nvs_sector_size;
  nvsaddr_base = g_uiodev.nvsaddr_nvdata;

  chrec_count = (sector_size - sizeof(TUioNvDataHead)) / sizeof(TUioNvDataChRec);

  // two sectors, select the newer one

  TUioNvDataHead *  phead1 = (TUioNvDataHead *)(nvsaddr_base + 0 * sector_size);
  TUioNvDataHead *  phead2 = (TUioNvDataHead *)(nvsaddr_base + 1 * sector_size);

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

    hwintflash.StartEraseMem(nvsaddr_base, sector_size * 2);
    hwintflash.WaitForComplete();

    ldatahead.signature = UIO_NVDATA_SIGNATURE;
    ldatahead.serial = 1;
    memcpy(&ldatahead.value[0], &value[0], sizeof(value));

    hwintflash.StartWriteMem(nvsaddr_base, &ldatahead,  sizeof(ldatahead));
    hwintflash.WaitForComplete();

    sector_index = 0;
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

  TUioNvDataHead *  phead = (TUioNvDataHead *)(nvsaddr_base + sector_size * sector_index);

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

      hwintflash.StartWriteMem(unsigned(prec), &lrec,  sizeof(lrec));
      hwintflash.WaitForComplete();
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

  hwintflash.StartEraseMem(nvsaddr_base + sector_size * sector_index, sector_size);
  hwintflash.WaitForComplete();

  hwintflash.StartWriteMem(nvsaddr_base + sector_size * sector_index, &ldatahead,  sizeof(ldatahead));
  hwintflash.WaitForComplete();

  return 0;
}
