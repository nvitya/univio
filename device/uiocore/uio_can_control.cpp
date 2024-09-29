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
 *  file:     uio_can_control.cpp
 *  brief:    UNIVIO CAN controller interface
 *  date:     2024-09-10
 *  authors:  nvitya
*/


#include "uio_can_control.h"
#include "uio_dev_base.h"

THwCan       g_can[UIO_CAN_COUNT];
TUioCanCtrl  g_canctrl[UIO_CAN_COUNT];

void TUioCanCtrl::Init(TUioDevBase * adevbase, THwCan * acan)
{
  devbase = adevbase;
  can = acan;
  mstatus.rx_cnt = 0;
  mstatus.tx_cnt = 0;
  rxbuf_filled = false;
}


void TUioCanCtrl::Run()
{
  if (can->Enabled())
  {
    can->HandleRx();
    unsigned rxidx = (mstatus.rx_cnt % UIO_CAN_RXBUF_SIZE);
    while (can->TryRecvMessage(&rxbuf[rxidx]))
    {
      ++mstatus.rx_cnt;
      ++rxidx;
      if (rxidx >= UIO_CAN_RXBUF_SIZE)
      {
        rxidx = 0;
      }
    }

    if (mstatus.rx_cnt >= UIO_CAN_RXBUF_SIZE)
    {
      rxbuf_filled = true;
    }

    can->HandleTx();
    can->UpdateErrorCounters();  // to count the CAN bus errors
  }
}

bool TUioCanCtrl::prfn_CanControl(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint8_t idx  = (rq->index & 0x1F);

  if (0x00 == idx) // CAN Speed
  {
    if (rq->iswrite)
    {
      can->SetSpeed(udorq_intvalue(rq));
      return udo_response_ok(rq);
    }
    else
    {
      return udo_ro_int(rq, can->speed, 4);
    }
  }
  else if (0x01 == idx) // CAN Control
  {
    bool recvown = can->receive_own;
    bool active  = can->Enabled();
    bool silent_monitor = can->silent_monitor_mode;
    uint32_t ctrl = 0;

    if (!rq->iswrite)
    {
      if (active)   ctrl |= (1 << 0);
      if (recvown)  ctrl |= (1 << 1);
      if (silent_monitor)  ctrl |= (1 << 2);
      return udo_ro_uint(rq, ctrl, 4);
    }

    ctrl = udorq_uintvalue(rq);

    if (ctrl & UIO_CAN_CTRL_ACTIVE)
    {
      active = true;
    }
    else if (ctrl & (UIO_CAN_CTRL_ACTIVE << 16))
    {
      active = false;
    }

    if (ctrl & UIO_CAN_CTRL_RECVOWN)
    {
      recvown = true;
    }
    else if (ctrl & (UIO_CAN_CTRL_RECVOWN << 16))
    {
      recvown = false;
    }

    if (ctrl & UIO_CAN_CTRL_SILENTM)
    {
      silent_monitor = true;
    }
    else if (ctrl & (UIO_CAN_CTRL_SILENTM << 16))
    {
      silent_monitor = false;
    }

    can->receive_own = recvown;
    can->silent_monitor_mode = silent_monitor;
    if (active && !can->Enabled())
    {
      SetFilters();
      can->Enable();
    }
    else if (!active && can->Enabled())
    {
      can->Disable();
    }

    return udo_response_ok(rq);
  }
  else if (0x02 == idx) // CAN Filters
  {
    if (!udo_rw_data(rq, &filters[0], sizeof(filters)))
    {
      return false;
    }

    if (rq->iswrite && can->Enabled())
    {
      can->Disable();
      SetFilters();
      can->Enable();
    }

    return true;
  }
  else if (0x03 == idx) // CAN Status
  {
    UpdateStatus();
    return udo_ro_data(rq, &mstatus, sizeof(mstatus));
  }
  else if (0x04 == idx) // CAN Reset counters
  {
    if (rq->iswrite)
    {
      can->errcnt_ack = 0;
      can->errcnt_crc = 0;
      can->errcnt_form = 0;
      can->errcnt_stuff = 0;
      can->errcnt_bit0 = 0;
      can->errcnt_bit1 = 0;
      return udo_response_ok(rq);
    }
    else
    {
      return udo_response_error(rq, UDOERR_WRITE_ONLY);
    }
  }
  else if (0x05 == idx) // CAN Send Message
  {
    if (!rq->iswrite)
    {
      return udo_response_error(rq, UDOERR_WRITE_ONLY);
    }

    unsigned remaining = rq->rqlen;
    TCanMsg * pmsg = (TCanMsg *)rq->dataptr;
    TCanMsg * pmsg_end = (TCanMsg *)(rq->dataptr + rq->rqlen - sizeof(TCanMsg));
    while (pmsg <= pmsg_end)
    {
      can->StartSendMessage(pmsg);
      ++pmsg;
    }

    return udo_response_ok(rq);
  }
  else if (0x06 == idx) // CAN Rx Buffer Size
  {
    return udo_ro_uint(rq, UIO_CAN_RXBUF_SIZE,  4);
  }
  else if (0x07 == idx) // CAN Rx Status
  {
    return udo_ro_uint(rq, mstatus.rx_cnt,  4);
  }
  else if (0x08 == idx) // CAN Read Received Messages
  {
    if (rq->iswrite)
    {
      return udo_response_error(rq, UDOERR_READ_ONLY);
    }

    if (rq->maxanslen < 4 + sizeof(TCanMsg))
    {
      return udo_response_error(rq, UDOERR_DATA_TOO_BIG);
    }

    // Answer format:
    //   u32       rxcnt;
    //   u32       start_rxcnt;
    //   TCanMsg   msg[];

    uint32_t * pact_rxcnt   = (uint32_t *)(rq->dataptr + 0);
    uint32_t * pstart_rxcnt = (uint32_t *)(rq->dataptr + 4);
    *pact_rxcnt = mstatus.rx_cnt;
    rq->anslen = 8;

    TCanMsg *  pmsg = (TCanMsg *)(rq->dataptr + 8);

    int msgcnt = mstatus.rx_cnt - rq->offset;  // count of new messages to send
    if (msgcnt <= 0)
    {
      // the provided offset was higher than the actual message count, report the last
      // rxcnt with no messages
      *pstart_rxcnt = mstatus.rx_cnt;
      return udo_response_ok(rq);
    }

    // else msgcnt > 0

    if (msgcnt > UIO_CAN_RXBUF_SIZE)
    {
      msgcnt = UIO_CAN_RXBUF_SIZE; // some messages are lost in this case
    }

    uint32_t startcnt = mstatus.rx_cnt - msgcnt;
    uint32_t maxmsg = (rq->maxanslen - rq->anslen) / sizeof(TCanMsg);
    if (msgcnt > maxmsg)
    {
      msgcnt = maxmsg;  // not all the new messages fit into the answer buffer
    }

    unsigned midx = (startcnt % UIO_CAN_RXBUF_SIZE);
    while (msgcnt)
    {
      *pmsg = rxbuf[midx];
      ++pmsg;
      ++midx;
      if (midx >= UIO_CAN_RXBUF_SIZE)
      {
        midx = 0;
      }

      rq->anslen += sizeof(TCanMsg);
      --msgcnt;
    }

    return udo_response_ok(rq);
  }

  return udo_response_error(rq, UDOERR_INDEX);
}

void TUioCanCtrl::SetFilters()
{
  can->AcceptListClear();
  unsigned fnum = filters[0];
  if (0 == fnum)
  {
    can->AcceptAdd(0x000, 0x000);
  }
  else
  {
    if (fnum > 7) fnum = 7;
    for (unsigned n = 1; n <= fnum; ++n)
    {
      can->AcceptAdd(filters[n] & 0xFFFF, filters[n] >> 16);
    }
  }
}

void TUioCanCtrl::UpdateStatus()
{
  unsigned st = 0;
  if (can->IsBusOff())   st |= UIO_CAN_ST_ERR_BUSOFF;
  if (can->IsWarning())  st |= UIO_CAN_ST_ERR_PASSIVE;
  mstatus.status = st;

  unsigned ctrl = 0;
  if (can->Enabled())    ctrl |= UIO_CAN_CTRL_ACTIVE;
  if (can->receive_own)  ctrl |= UIO_CAN_CTRL_RECVOWN;
  if (can->silent_monitor_mode)  ctrl |= UIO_CAN_CTRL_SILENTM;
  mstatus.ctrl = ctrl;

  mstatus.acterr_rx = can->acterr_rx;
  mstatus.acterr_tx = can->acterr_tx;
  mstatus.timestamp = can->TimeStampCounter();

  mstatus.errcnt_ack   = can->errcnt_ack;
  mstatus.errcnt_crc   = can->errcnt_crc;
  mstatus.errcnt_form  = can->errcnt_form;
  mstatus.errcnt_stuff = can->errcnt_stuff;
  mstatus.errcnt_bit0  = can->errcnt_bit0;
  mstatus.errcnt_bit1  = can->errcnt_bit1;
}
