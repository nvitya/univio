/*
 * uio_dev_base.h
 *
 *  Created on: Nov 24, 2021
 *      Author: vitya
 */

#ifndef UNIVIO_UIO_DEV_BASE_H_
#define UNIVIO_UIO_DEV_BASE_H_

#include "univio.h"


typedef struct
{
  uint16_t          usb_vendor_id; //  = 0xDEAD;
  uint16_t          usb_product_id; // = 0x6E10;
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

  TUioDevBaseCfg    basecfg;

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

  bool              ResponseU32(TUnivioRequest * rq, uint32_t adata);
  bool              ResponseU16(TUnivioRequest * rq, uint16_t adata);
  bool              ResponseU8(TUnivioRequest * rq, uint8_t adata);

  uint32_t          RqValueU32(TUnivioRequest * rq);
  uint16_t          RqValueU16(TUnivioRequest * rq);
  uint8_t           RqValueU8(TUnivioRequest * rq);

  bool              HandleRw(TUnivioRequest * rq, void * avarptr, unsigned alen);

};

#endif /* UNIVIO_UIO_DEV_BASE_H_ */
