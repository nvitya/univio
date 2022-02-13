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
 *  file:     univio_comm.cpp
 *  brief:    UNIVIO UART interface
 *  created:  2021-12-05
 *  authors:  nvitya
*/

#include "platform.h"

#if UART_CTRL_ENABLE

#include "common.h"
#include "univio.h"
#include "univio_comm.h"
#include "uio_gendev.h"
#include "traces.h"

TUartComm g_uartctrl;

// the TUartComm::InitHw() are moved to the board_pins.cpp

bool TUartComm::Init()
{
  initialized = false;

  if (!uart.initialized)
  {
    return false;
  }

  uart.DmaAssign(true,  &dma_tx);
  uart.DmaAssign(false, &dma_rx);

  // start the DMA receive with circular DMA buffer
  rxdmapos = 0;
  dmaxfer_rx.bytewidth = 1;
  dmaxfer_rx.count = sizeof(rxdmabuf);
  dmaxfer_rx.dstaddr = &rxdmabuf[0];
  dmaxfer_rx.flags = DMATR_CIRCULAR;
  uart.DmaStartRecv(&dmaxfer_rx);

  rxmsglen = 0;
  txlen = 0;

/*
  // Send some wellcome message
  AddTxMessage("\r\n-------------------------------\r\n");
  AddTxMessage("VIHAL UART DMA DEMO\r\n");
  AddTxMessage("Enter some text (not echoed as you type) and press enter!\r\n");
  StartSendTxBuffer(); // send it with DMA now
*/

  initialized = true;
  return true;
}

unsigned TUartComm::AddTx(void * asrc, unsigned len) // returns the amount actually written
{
  unsigned available = TxAvailable();
  if (0 == available)
  {
    return 0;
  }

  if (len > available)  len = available;

  uint8_t * srcp = (uint8_t *)asrc;
  uint8_t * dstp = &txbuf[txbufwr][txlen];
  uint8_t * endp = dstp + len;
  while (dstp < endp)
  {
    uint8_t b = *srcp++;
    *dstp++ = b;
    txcrc = univio_calc_crc(txcrc, b);
  }

  txlen += len;

  return len;
}


void TUartComm::StartSendTxBuffer()
{
  if (txlen && !dma_tx.Active())
  {
    // setup the TX DMA and flip the buffer

    dmaxfer_tx.flags = 0;
    dmaxfer_tx.bytewidth = 1;
    dmaxfer_tx.count = txlen;
    dmaxfer_tx.srcaddr = &txbuf[txbufwr][0];

    uart.DmaStartSend(&dmaxfer_tx);

    // change the buffer
    txbufwr ^= 1;
    txlen = 0;
  }
}

void TUartComm::Run()
{
  // RX processing
  uint16_t newrxdmapos = sizeof(rxdmabuf) - dma_rx.Remaining();
  if (newrxdmapos >= sizeof(rxdmabuf))
  {
    // fix for Remaining() == 0 (sometimes the linked list operation is processed only later)
    newrxdmapos = 0;
  }

  unsigned t0 = CLOCKCNT;

  while (rxdmapos != newrxdmapos)
  {
    uint8_t b = rxdmabuf[rxdmapos];
    lastrecvtime = t0;

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
      }
      else
      {
        // execute the request, prepares the answer
        g_uiodev.HandleRequest(&rq);

        SendAnswer(); // the answer is prepared in the rq
      }

      rxstate = 0; // go to the next request
    }

    ++rxdmapos;
    if (rxdmapos >= sizeof(rxdmabuf))  rxdmapos = 0;
  }

#if 0
  if (lastrecvtime - t0 > US_TO_CLOCKS(UARTCOMM_RX_TIMEOUT_US))
  {
    ++error_count_timeout;
    rxstate = 0;
  }
#endif

  // Sending buffered tx messages
  StartSendTxBuffer();
}

void TUartComm::SendAnswer()
{
  uint8_t   b;
  uint16_t  w;
  uint32_t  d;

  rq.metalen = 0; // no metadata by anwers yet
  txcrc = 0;
  b = 0x55; // sync
  AddTx(&b, 1);
  b = (rq.iswrite ? 1 : 0);
  if (rq.result)
  {
    // error response
    b |= (1 << 1) | (2 << 4);  // error flag with fix 2 byte data
    AddTx(&b, 1);
    AddTx(&rq.address, 2); // echo the address back
    AddTx(&rq.result, 2); // send the result
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
      AddTx(&b, 1); // send the command / length
      AddTx(&rq.length, 2); // add the extended length
    }
    else
    {
      b |= (rq.length << 4);
      AddTx(&b, 1); // send the command / length
    }

    AddTx(&rq.address, 2); // echo back the address

    // no metadata

    if (rq.length)
    {
      AddTx(&rq.data[0], rq.length);
    }
  }

  b = txcrc;
  AddTx(&b, 1); // then send the crc
}

#endif
