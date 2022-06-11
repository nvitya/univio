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
 *  file:     uio_dev_base.h
 *  brief:    UNIVIO basic device definitions and utilities
 *  version:  1.00
 *  date:     2022-02-13
 *  authors:  nvitya
*/

#pragma once

#include "univio.h"


typedef struct
{
  uint16_t          usb_vendor_id;
  uint16_t          usb_product_id;
  char              manufacturer[32];
  char              device_id[32];
  char              serial_number[32];
//
} TUioDevBaseCfg;

class TUioDevBase
{
public:
  virtual           ~TUioDevBase() { }

  uint8_t           runmode = 0;  // 0 = CONFIG mode, 1 = RUN mode

  bool              initialized = false;

  TUioDevBaseCfg    basecfg __attribute__((aligned(4)));

  bool              Init();

  bool              HandleRequest(TUnivioRequest * rq);

public:

  virtual bool      InitDevice() { return false; }
  virtual bool      HandleDeviceRequest(TUnivioRequest * rq)  { return false; }
  virtual void      SaveSetup()  { }
  virtual void      LoadSetup()  { }
  virtual void      SetRunMode(uint8_t arunmode) { }

public: // utility functions

  bool              ResponseError(TUnivioRequest * rq, uint16_t aerror);
  bool              ResponseOk(TUnivioRequest * rq);

  bool              ResponseBin(TUnivioRequest * rq, void * asrc, unsigned alen);
  bool              ResponseStr(TUnivioRequest * rq, const char * astr);

  bool              ResponseI32(TUnivioRequest * rq, int32_t adata);
  bool              ResponseI16(TUnivioRequest * rq, int16_t adata);

  bool              ResponseU32(TUnivioRequest * rq, uint32_t adata);
  bool              ResponseU16(TUnivioRequest * rq, uint16_t adata);
  bool              ResponseU8(TUnivioRequest * rq, uint8_t adata);

  uint32_t          RqValueU32(TUnivioRequest * rq);
  uint16_t          RqValueU16(TUnivioRequest * rq);
  uint8_t           RqValueU8(TUnivioRequest * rq);

  bool              HandleRw(TUnivioRequest * rq, void * avarptr, unsigned alen);

};


