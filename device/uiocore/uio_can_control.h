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
 *  file:     uio_can_control.h
 *  brief:    UNIVIO CAN controller interface
 *  date:     2024-09-10
 *  authors:  nvitya
*/

#ifndef UIOCORE_UIO_CAN_CONTROL_H_
#define UIOCORE_UIO_CAN_CONTROL_H_

#include "tclass.h"
#include "udo.h"
#include "udoslave.h"
#include "simple_partable.h"

#include "hwcan.h"
#include "hwdma.h"

#include "uio_common.h"

#define UIO_CAN_CTRL_ACTIVE     (1 << 0)
#define UIO_CAN_CTRL_RECVOWN    (1 << 1)
#define UIO_CAN_CTRL_SILENTM    (1 << 2)   // silent monitor mode

#define UIO_CAN_ST_ERR_PASSIVE  (1 << 0)
#define UIO_CAN_ST_ERR_BUSOFF   (1 << 1)

#ifndef UIO_CAN_RXBUF_SIZE
  #define UIO_CAN_RXBUF_SIZE      64
#endif

class TUioDevBase;

typedef struct
{
  uint8_t   status;       // actual status flags
  uint8_t   ctrl;         // actual conrol flags
  uint8_t   acterr_rx;    // CAN std actual RX error counter (0-255)
  uint8_t   acterr_tx;    // CAN std actual TX error counter (0-255)
  uint16_t  timestamp;    // Actual CAN bit time counter, which is used for timestamping
  uint16_t  _reserved;

  uint32_t  errcnt_ack;   // Count of CAN ACK Errors
  uint32_t  errcnt_crc;   // Count of CAN CRC Errors
  uint32_t  errcnt_form;  // Count of CAN Frame Errors
  uint32_t  errcnt_stuff; // Count of CAN Stuffing errors
  uint32_t  errcnt_bit0;  // Count of CAN bit 0 sending error (bit 1 received)
  uint32_t  errcnt_bit1;  // Count of CAN bit 1 sending error (bit 0 received)

  uint32_t  rx_cnt;       // CAN messages received since the start
  uint32_t  tx_cnt;       // CAN messages transmitted (queued) since the start
//
} TCanCtrlStatus; // 40 bytes


class TUioCanCtrl : public TClass
{
public:
  TUioDevBase *     devbase = nullptr;
  THwCan *          can = nullptr;

  bool              rxbuf_filled = false;
  TCanCtrlStatus    mstatus;

  uint32_t          filters[8] = {0};

  void              Init(TUioDevBase * adevbase, THwCan * acan);
  void              Run();
  bool              prfn_CanControl(TUdoRequest * rq, TParamRangeDef * prdef);

  void              SetFilters();
  void              UpdateStatus();

public:

  TCanMsg           rxbuf[UIO_CAN_RXBUF_SIZE];
};

extern THwCan       g_can[UIO_CAN_COUNT];
extern TUioCanCtrl  g_canctrl[UIO_CAN_COUNT];

#endif /* UIOCORE_UIO_CAN_CONTROL_H_ */
