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
 *  file:     uio_dev_base.cpp
 *  brief:    UNIVIO basic device definitions and utilities
 *  version:  1.00
 *  date:     2022-02-13
 *  authors:  nvitya
*/

#include "string.h"
#include <uio_dev_base.h>

bool TUioDevBase::Init()
{
  initialized = false;

  strncpy(basecfg.device_id, UIO_FW_ID, sizeof(basecfg.device_id));

  if (!InitDevice())
  {
    return false;
  }

  initialized = true;
  return true;
}

bool TUioDevBase::HandleRequest(TUnivioRequest * rq)
{
  if (!initialized)
  {
    return ResponseError(rq, UIOERR_INTERNAL);
  }

  uint16_t addr = rq->address;

  if (addr >= 0x0100) // these are not handled here
  {
    if (HandleDeviceRequest(rq))
    {
      return true;
    }
    else
    {
      return ResponseError(rq, UIOERR_WRONG_ADDR);
    }
  }

  if (addr < 0x0010) // read-only system data
  {
    switch (addr)
    {
      case 0x0000:  return ResponseU32(rq, 0x66CCAA55);
      case 0x0001:  return ResponseU32(rq, UIO_MAX_DATA_LEN);
      case 0x0002:  return ResponseU32(rq, UIO_MEM_SIZE);
      case 0x0003:  return ResponseStr(rq, UIO_FW_ID);
      case 0x0004:  return ResponseU32(rq, UIO_FW_VER);
    }

    return ResponseError(rq, UIOERR_WRONG_ADDR);
  }

  if (0x0010 == addr)  // RUN / CONFIG mode
  {
    if (!rq->iswrite)
    {
      return ResponseU8(rq, runmode);
    }

    uint8_t rmv = RqValueU8(rq);
    if (rmv > 1) // special command
    {
      if (2 == rmv)
      {
        // TODO: restart
        return ResponseError(rq, UIOERR_NOT_IMPLEMENTED);
      }
      return ResponseError(rq, UIOERR_VALUE);
    }

    SetRunMode(rmv);
    return ResponseOk(rq);
  }

  // else R/W system data

  if (rq->iswrite && runmode)
  {
    return ResponseError(rq, UIOERR_RUN_MODE);
  }

  switch (addr)
  {
    case 0x0011:   return HandleRw(rq, &basecfg.device_id[0], sizeof(basecfg.device_id));
    case 0x0012:   return HandleRw(rq, &basecfg.usb_vendor_id, sizeof(basecfg.usb_vendor_id));
    case 0x0013:   return HandleRw(rq, &basecfg.usb_product_id, sizeof(basecfg.usb_product_id));
    case 0x0014:   return HandleRw(rq, &basecfg.manufacturer[0], sizeof(basecfg.manufacturer));
    case 0x0015:   return HandleRw(rq, &basecfg.serial_number[0], sizeof(basecfg.serial_number));
  }

  return ResponseError(rq, UIOERR_WRONG_ADDR);
}

bool TUioDevBase::ResponseError(TUnivioRequest * rq, uint16_t aerror)
{
  if (aerror)
  {
    rq->result = aerror;
    rq->length = 2;
  }
  else
  {
    return ResponseOk(rq);
  }
  return true;
}

bool TUioDevBase::ResponseOk(TUnivioRequest * rq)
{
  if (!rq->iswrite)
  {
    return ResponseError(rq, UIOERR_INTERNAL);
  }
  rq->result = 0;
  rq->length = 0;
  return true;
}

bool TUioDevBase::HandleRw(TUnivioRequest * rq, void * avarptr, unsigned alen)
{
  if (rq->iswrite)
  {
    if (rq->length < alen)
    {
      memset(avarptr, 0, alen);  // fill up with zeroes first
    }
    else
    {
      rq->length = alen;
    }

    memcpy(avarptr, &rq->data[0], rq->length);
    rq->result = 0;
    return true;
  }
  else
  {
    return ResponseBin(rq, avarptr, alen);
  }
}

bool TUioDevBase::ResponseU32(TUnivioRequest * rq, uint32_t adata)
{
  if (rq->iswrite)  return ResponseError(rq, UIOERR_READ_ONLY);

  *(uint32_t *)&rq->data[0] = adata;
  rq->length = sizeof(adata);
  rq->result = 0;
  return true;
}

bool TUioDevBase::ResponseU16(TUnivioRequest * rq, uint16_t adata)
{
  if (rq->iswrite)  return ResponseError(rq, UIOERR_READ_ONLY);

  *(uint16_t *)&rq->data[0] = adata;
  rq->length = sizeof(adata);
  rq->result = 0;
  return true;
}

bool TUioDevBase::ResponseU8(TUnivioRequest * rq, uint8_t adata)
{
  if (rq->iswrite)  return ResponseError(rq, UIOERR_READ_ONLY);

  rq->data[0] = adata;
  rq->length = 1;
  rq->result = 0;
  return true;
}

bool TUioDevBase::ResponseBin(TUnivioRequest * rq, void * asrc, unsigned alen)
{
  if (rq->iswrite)  return ResponseError(rq, UIOERR_READ_ONLY);

  if (alen > sizeof(rq->data))  alen = sizeof(rq->data);

  rq->length = alen;
  memcpy(&rq->data[0], asrc, alen);
  rq->result = 0;
  return true;
}

bool TUioDevBase::ResponseStr(TUnivioRequest * rq, const char * astr)
{
  if (rq->iswrite)  return ResponseError(rq, UIOERR_READ_ONLY);

  unsigned len = strlen(astr);
  if (len > sizeof(rq->data))  len = sizeof(rq->data);

  rq->length = len;
  memcpy(&rq->data[0], astr, len);
  rq->result = 0;
  return true;
}

uint32_t TUioDevBase::RqValueU32(TUnivioRequest * rq)
{
  if      (1 == rq->length)  return rq->data[0];
  else if (2 == rq->length)  return *(uint16_t *)&rq->data[0];
  else                       return *(uint32_t *)&rq->data[0];
}

uint16_t TUioDevBase::RqValueU16(TUnivioRequest * rq)
{
  if      (1 == rq->length)  return rq->data[0];
  else                       return *(uint16_t *)&rq->data[0];
}

uint8_t TUioDevBase::RqValueU8(TUnivioRequest * rq)
{
  return rq->data[0];
}
