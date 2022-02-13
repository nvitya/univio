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
 *  file:     usb_cdc_echo.cpp
 *  brief:    CDC Echo USB Device definition
 *  version:  1.00
 *  date:     2020-06-19
 *  authors:  nvitya
*/

#include "string.h"
#include <usb_univio.h>
#include "traces.h"
#include "hwpins.h"
#include "usbdevice.h"
#include "hwusbctrl.h"

#include "uio_device.h"

TUsbDevUio       usbdev;

TUsbFuncUio      usb_func_uio;
TUsbFuncCdcUart  usb_func_uart;

void usb_device_init()
{
	TRACE("Initializing UnivIO USB Device\r\n");

	if (!usbdev.Init()) // calls InitDevice first which sets up the device
	{
		TRACE("Error initializing USB device!\r\n");
		return;
	}

	//TRACE("IF input endpoint: %02X\r\n", uio_cdc_data.ep_input.index);
	//TRACE("IF output endpoint: %02X\r\n", uio_cdc_data.ep_output.index);
}

void usb_device_run()
{
	usbdev.HandleIrq();

	usbdev.Run();
}

//----------------------------------------------------

bool TUifCdcControl::InitInterface()
{
	intfdesc.interface_class = 2; // CDC
	intfdesc.interface_subclass = 2; // Abstract Control Model
	intfdesc.interface_protocol = 0; // 0 = no class specitic control

	interface_name = "UnivIO Control";

	desc_cdc_callmgmt.data_interface = index + 1;  // the following interface must be the data interface
	desc_cdc_union.master_interface = index;
	desc_cdc_union.slave_interface = index + 1;

	// some other descriptors are required

	AddConfigDesc((void *)&cdc_desc_header_func[0],     true);
  AddConfigDesc((void *)&desc_cdc_callmgmt,           true);
	AddConfigDesc((void *)&cdc_desc_call_acm_func[0],   true);
  AddConfigDesc((void *)&desc_cdc_union,              true);

	// endpoints

	ep_manage.Init(HWUSB_EP_TYPE_INTERRUPT, false, 8);
	ep_manage.interval = 10; // polling interval
	AddEndpoint(&ep_manage);

	return true;
}

void TUifCdcControl::OnConfigured()
{
	//TRACE("SPEC Device Configured.\r\n");

	//ep_manage.EnableRecv();
}

bool TUifCdcControl::HandleTransferEvent(TUsbEndpoint * aep, bool htod)
{
	return false;
}

bool TUifCdcControl::HandleSetupRequest(TUsbSetupRequest * psrq)
{
	if (0x20 == psrq->request) // set line coding, data stage follows !
	{
		//TRACE("UIO Ctrl Set line coding (SETUP)\r\n");
		device->StartSetupData();  // start receiving the data part, which will be handled at the HandleSetupData()
		return true;
	}
	else if (0x21 == psrq->request) // Get line coding
	{
		//TRACE("VCP Get line coding\r\n");
		device->StartSendControlData(&linecoding, sizeof(linecoding));
		return true;
	}
	else if (0x22 == psrq->request) // Set Control Line State
	{
		//TRACE("VCP Set Control Line State: %04X\r\n", psrq->value);
		device->SendControlStatus(true);
		return true;
	}

	return false;
}

// the setup request's data part comes in a separate phase and so it has a separate callback:
bool TUifCdcControl::HandleSetupData(TUsbSetupRequest * psrq, void * adata, unsigned adatalen)
{
	if (0x20 == psrq->request) // set line coding
	{
		memcpy(&linecoding, adata, sizeof(linecoding));

	  //TRACE("%u ", CLOCKCNT / (SystemCoreClock / 1000));
		//TRACE("UIO Ctrl Line Coding data:\r\n  baud=%i, format=%i, parity=%i, bits=%i\r\n",
		//		linecoding.baudrate, linecoding.charformat, linecoding.paritytype, linecoding.databits
		//);

		device->SendControlStatus(true);
		return true;
	}

	return false;
}

//------------------------------------------------

bool TUsbFuncUio::InitFunction()
{
  funcdesc.function_class = 2;
  funcdesc.function_sub_class = 2;
  funcdesc.function_protocol = 1;

  func_name = "UnivIO Control";

  AddInterface(&uif_control);
  AddInterface(&uif_data);

  return true;
}

void TUsbFuncUio::Run()
{
  uif_data.Run();
}

//------------------------------------------------

bool TUsbDevUio::InitDevice()
{
  devdesc.vendor_id = g_uiodev.basecfg.usb_vendor_id;
  if (0 == devdesc.vendor_id)
  {
    devdesc.vendor_id = UIO_DEFAULT_VENDOR_ID;
  }

  devdesc.product_id = g_uiodev.basecfg.usb_product_id;
  if (0 == devdesc.product_id)
  {
    devdesc.product_id = UIO_DEFAULT_PRODUCT_ID;
  }

  devdesc.device_class = 0x02; // Communications and CDC Control
  manufacturer_name = &g_uiodev.basecfg.manufacturer[0];
  device_name = &g_uiodev.basecfg.device_id[0];
  device_serial_number = &g_uiodev.basecfg.serial_number[0];
  if (strlen(manufacturer_name) == 0)
  {
    manufacturer_name = "UnivIO";
  }
  if (strlen(device_name) == 0)
  {
    device_name = UIO_FW_ID;
  }
  if (strlen(device_serial_number) == 0)
  {
    device_serial_number = "1";
  }

  AddFunction(&usb_func_uio);

  if (g_uiodev.uart_active)
  {
    // Add the bridged real USB to UART
    TRACE("Activating USB-UART\r\n");

    usb_func_uart.AssignUart(&g_uart);
    AddFunction(&usb_func_uart);
  }

  return true;
}

//------------------------------------------------

bool TUioCdcData::InitInterface()
{
	intfdesc.interface_class = 0x0A; // CDC Data
	intfdesc.interface_subclass = 0;
	intfdesc.interface_protocol = 0; // no specific protocol

	interface_name = "UnivIO Data";

	// endpoints

	ep_input.Init(HWUSB_EP_TYPE_BULK, true, 64);
	AddEndpoint(&ep_input);
	ep_output.Init(HWUSB_EP_TYPE_BULK, false, 64);
	AddEndpoint(&ep_output);

  Reset();

	return true;
}

void TUioCdcData::OnConfigured()
{
	TRACE("UnivIO Data Interface ready.\r\n");

  Reset();

	ep_input.EnableRecv();
}

bool TUioCdcData::HandleTransferEvent(TUsbEndpoint * aep, bool htod)
{
	int r;
	if (htod)
	{
	  // data chunk arrived
	  rxenabled = false;
		r = ep_input.ReadRecvData(&usb_rxbuf[0], sizeof(usb_rxbuf));
		//TRACE("%i byte VCP data arrived\r\n", r);
		if (r > 0)
		{
		  usb_rxlen = r;
		  usb_rpos = 0;
		  lastrecvtime = CLOCKCNT;

      #if 0
        TRACE("USB RxDATA: ");
        for (r = 0; r < usb_rxlen; ++r)  TRACE(" %02X", usb_rxbuf[r]);
        TRACE("\r\n");
      #endif

		  HandleRx();
		}
		else
		{
		  rxenabled = true;
      ep_input.EnableRecv();
		}
	}
	else
	{
	  usb_ready_to_send = true;
	  HandleTx();
	}

	return true;
}

void TUioCdcData::Reset()
{
  usb_txlen = 0;
  rxenabled = true;
  tx_remaining = 0;
  txstate = 0;
}

void TUioCdcData::Run()
{
  HandleTx();
  HandleRx();
}

void TUioCdcData::HandleRx()
{
  while ((rxstate != 50) && (usb_rpos < usb_rxlen))
  {
    uint8_t b = usb_rxbuf[usb_rpos];

    //TRACE("< %02X, rxstate=%u\r\n", b, rxstate);

    if ((rxstate > 0) && (rxstate < 10))
    {
      rxcrc = univio_calc_crc(rxcrc, b);
    }

    if (0 == rxstate)  // waiting for the sync byte
    {
      if (0x55 == b)
      {
        rxcrc = univio_calc_crc(0, b); // start the CRC from zero
        rxstate = 1;
      }
    }
    else if (1 == rxstate) // command and length
    {
      if (b & 1) // bit0: 0 = read, 1 = write
      {
        rq.iswrite = 1;
      }
      else
      {
        rq.iswrite = 0;
      }
      // calculate the metadata length
      rq.metalen = ((0x8420 >> (b & 0xC)) & 0xF);
      rq.length = (b >> 4);
      rxcnt = 0;
      if (15 == rq.length)
      {
        rxstate = 2;  // extended length follows
      }
      else
      {
        rxstate = 3;  // address follows
      }
    }
    else if (2 == rxstate) // extended length
    {
      if (0 == rxcnt)
      {
        rq.length = b; // low byte
        rxcnt = 1;
      }
      else
      {
        rq.length |= (b << 8); // high byte
        rxcnt = 0;
        rxstate = 3; // address follows
      }
    }
    else if (3 == rxstate) // address
    {
      if (0 == rxcnt)
      {
        rq.address = b;  // address low
        rxcnt = 1;
      }
      else
      {
        rq.address |= (b << 8);  // address high
        rxcnt = 0;
        if (rq.metalen)
        {
          rxstate = 4;  // meta follows
        }
        else
        {
          if (rq.iswrite)
          {
            rxstate = 5;  // data follows when write
          }
          else
          {
            rxstate = 10;  // then crc check
          }
        }
      }
    }
    else if (4 == rxstate) // metadata
    {
      rq.metadata[rxcnt] = b;
      ++rxcnt;
      if (rxcnt >= rq.metalen)
      {
        if (rq.iswrite)
        {
          rxcnt = 0;
          rxstate = 5; // write data follows
        }
        else
        {
          rxstate = 10;  // crc check
        }
      }
    }
    else if (5 == rxstate) // write data
    {
      rq.data[rxcnt] = b;
      ++rxcnt;
      if (rxcnt >= rq.length)
      {
        rxstate = 10;
      }
    }
    else if (10 == rxstate) // crc check
    {
      if (b != rxcrc)
      {
        TRACE("CRC error: expected: %02X\r\n", rxcrc);
        // crc error, no answer
        ++error_count_crc;

        rxstate = 0; // go to the next request
      }
      else
      {
        // execute the request, prepares the answer
        g_uiodev.HandleRequest(&rq);

        // the answer is prepared in the rq, send it

        txstate = 1; // start the answering
        rxstate = 50; // wait until the response is sent, the HandleTX() might change this

        HandleTx();
      }
    }

    ++usb_rpos;
  }

  if (0 == txstate)  // if answering is finished
  {
    if (50 == rxstate)
    {
      rxstate = 0;  // start the receiving
    }

    if (!rxenabled)
    {
      rxenabled = true;
      ep_input.EnableRecv();
    }
  }
}

void TUioCdcData::HandleTx()
{
  if (0 == txstate)
  {
    return;
  }

  uint8_t   b;

  if (1 == txstate)  // prepare the head
  {
    usb_txlen = 0;
    rq.metalen = 0; // no metadata by anwers yet
    txcrc = 0;
    b = 0x55; // sync
    AddTx(b);

    b = (rq.iswrite ? 1 : 0);
    if (rq.result)
    {
      // error response
      b |= (1 << 1) | (2 << 4);  // error flag with fix 2 byte data
      AddTx(b);
      AddTx(rq.address & 0xFF); // echo the address back
      AddTx((rq.address >> 8) & 0xFF);
      AddTx(rq.result & 0xFF); // send the result
      AddTx((rq.result >> 8) & 0xFF); // send the result

      txstate = 10; // crc
    }
    else
    {
      // normal response
      if (rq.iswrite)
      {
        rq.length = 0;
      }

      if (rq.length > 14)
      {
        b |= 0xF0;
        AddTx(b); // send the command / length
        AddTx(rq.length & 0xFF); // add the extended length
        AddTx((rq.length >> 8) & 0xFF);
      }
      else
      {
        b |= (rq.length << 4);
        AddTx(b); // send the command / length
      }

      AddTx(rq.address & 0xFF); // echo the address back
      AddTx((rq.address >> 8) & 0xFF);

      // no metadata

      if (rq.length)
      {
        tx_dataidx = 0;
        txstate = 5; // send data body
      }
      else
      {
        txstate = 10; // crc
      }
    }
  }

  if (5 == txstate) // sending data body, might be chunked
  {
    while (true)
    {
      if (tx_dataidx < rq.length)
      {
        if (!AddTx(rq.data[tx_dataidx]))  // returns false when no more space in the buffer
        {
          break;
        }

        ++tx_dataidx;
      }
      else
      {
        txstate = 10; // CRC
        break;
      }
    }
  }

  if (10 == txstate) // sending crc
  {
    if (AddTx(txcrc))
    {
      txstate = 50;
    }
  }

  // sending buffered data
  if ((usb_txlen > 0) && usb_ready_to_send)
  {
    #if 0
      TRACE("USB Tx: ");
      for (unsigned n = 0; n < usb_txlen; ++n)  TRACE(" %02X", usb_txbuf[n]);
      TRACE("\r\n");
    #endif

    ep_output.StartSendData(&usb_txbuf[0], usb_txlen);
    usb_ready_to_send = false;
    usb_txlen = 0;
  }

  if (50 == txstate) // wait for last chunk send
  {
    if (0 == usb_txlen)
    {
      txstate = 0;  // go back to idle
    }
  }
}

bool TUioCdcData::AddTx(uint8_t b)
{
  if (usb_txlen >= sizeof(usb_txbuf))
  {
    return false;
  }

  usb_txbuf[usb_txlen] = b;
  txcrc = univio_calc_crc(txcrc, b);
  ++usb_txlen;

  return true;
}
