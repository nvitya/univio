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
 *  file:     usb_univio.h
 *  brief:    UnivIO USB Device definition
 *  created:  2021-12-05
 *  authors:  nvitya
*/

#ifndef SRC_USB_UNIVIO_H_
#define SRC_USB_UNIVIO_H_

#include "univio.h"
#include "usbdevice.h"
#include "usbif_cdc.h"
#include "usbfunc_cdc_uart.h"

#define UIO_DEFAULT_VENDOR_ID   0xDEAD
#define UIO_DEFAULT_PRODUCT_ID  0xBEEF

class TUifCdcControl : public TUsbInterface
{
private:
	typedef TUsbInterface super;

public:

  TUsbCdcDescCallUnionFunc  desc_cdc_union =
  {
    .length = 5,
    .descriptor_type = 0x24,  // 0x24 = CS_INTERFACE
    .descriptor_subtype = 6,  // 6 = union func
    .master_interface = 0,    // will be set automatically
    .slave_interface = 1      // will be set automatically
  };

  TUsbCdcDescCallManagement  desc_cdc_callmgmt =
  {
    .length = 5,
    .descriptor_type = 0x24,  // 0x24 = CS_INTERFACE
    .descriptor_subtype = 6,  // 6 = union func
    .capabilities = 0,        // 0 = no call management
    .data_interface = 0       // will be set automatically
  };

	TCdcLineCoding  linecoding;

	TUsbEndpoint    ep_manage;
	uint8_t         databuf[64] __attribute__((aligned(4)));

public: // mandatory functions
	virtual bool    InitInterface();
	virtual void    OnConfigured();
	virtual bool    HandleTransferEvent(TUsbEndpoint * aep, bool htod);
	virtual bool    HandleSetupRequest(TUsbSetupRequest * psrq);
	virtual bool    HandleSetupData(TUsbSetupRequest * psrq, void * adata, unsigned adatalen);

};

class TUioCdcData : public TUsbInterface
{
private:
	typedef TUsbInterface super;

public:

  bool            rxenabled = true;
  bool            usb_ready_to_send = true;
  unsigned        txstate = 0;
  unsigned        txcnt = 0;
  unsigned        tx_remaining = 0;
  unsigned        tx_dataidx = 0;

  uint8_t         rxcrc = 0;
  uint8_t         txcrc = 0;

  unsigned        lastrecvtime = 0;
  unsigned        rxstate = 0;
  unsigned        rxcnt = 0;
  unsigned        error_count_crc = 0;
  unsigned        error_count_timeout = 0;


  TUnivioRequest  rq;

  TUsbEndpoint    ep_input;
  TUsbEndpoint    ep_output;

  uint8_t         usb_txlen = 0;
  uint16_t        usb_rxlen = 0;
  uint16_t        usb_rpos = 0;

  uint8_t         usb_txbuf[64] __attribute__((aligned(4)));
  uint8_t         usb_rxbuf[64] __attribute__((aligned(4)));

public: // mandatory functions
	virtual bool    InitInterface();
	virtual void    OnConfigured();
	virtual bool    HandleTransferEvent(TUsbEndpoint * aep, bool htod);

public: // help functions
  void            Reset();

  void            Run();

  void            HandleRx();  // more commands might come in one packet
  void            HandleTx();

  bool            AddTx(uint8_t b);

};

class TUsbFuncUio : public TUsbFunction
{
private:
  typedef TUsbFunction super;

public:

  TUifCdcControl       uif_control;
  TUioCdcData          uif_data;

  virtual bool         InitFunction();
  virtual void         Run();

};

class TUsbDevUio : public TUsbDevice
{
private:
	typedef TUsbDevice super;

public: // mandatory functions

	virtual bool    InitDevice();

};

extern TUsbFuncUio      usb_func_uio;
extern TUsbFuncCdcUart  usb_func_uart;

extern TUsbDevUio       usbdev;

void usb_device_init();
void usb_device_run();


#endif /* SRC_USB_UNIVIO_H_ */
