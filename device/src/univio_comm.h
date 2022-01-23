/*
 * univio_comm.h
 *
 *  Created on: Nov 20, 2021
 *      Author: vitya
 */

#ifndef SRC_UNIVIO_COMM_H_
#define SRC_UNIVIO_COMM_H_

#include "platform.h"

#if UART_CTRL_ENABLE

#include "stdint.h"
#include "univio.h"
#include "hwuart.h"
#include "hwdma.h"

#define UARTCOMM_RXBUF_SIZE      1024   // circular buffer
#define UARTCOMM_TXBUF_SIZE      (UIO_MAX_DATA_LEN + 64)  // one tx response must fit into it

#define UARTCOMM_RX_TIMEOUT_US    10000  // 10 ms


class TUartComm
{
public:
  bool              initialized = false;

  uint8_t           rxcrc = 0;
  uint8_t           txcrc = 0;

  unsigned          lastrecvtime = 0;
  unsigned          rxstate = 0;
  unsigned          rxcnt = 0;
  unsigned          error_count_crc = 0;
  unsigned          error_count_timeout = 0;

  uint16_t          rxmsglen = 0;
  uint16_t          rxdmapos = 0;

  uint16_t          txbufwr = 0;
  uint16_t          txlen = 0;

  THwUart           uart;
  THwDmaChannel     dma_rx;
  THwDmaChannel     dma_tx;

  THwDmaTransfer    dmaxfer_tx;
  THwDmaTransfer    dmaxfer_rx;

  TUnivioRequest    rq;

  uint8_t           rxdmabuf[UARTCOMM_RXBUF_SIZE];  // circular buffer, might contain more messages

  uint8_t           txbuf[2][UARTCOMM_TXBUF_SIZE];

  bool              Init();
  void              Run();  // processes Rx and Tx

  void              SendAnswer(); // the answer is prepared in the rq

  unsigned          AddTx(void * asrc, unsigned len); // returns the amount actually written
  void              StartSendTxBuffer();
  inline unsigned   TxAvailable() { return sizeof(txbuf[0]) - txlen; }

};

extern TUartComm  g_uartctrl;

#endif

#endif /* SRC_UNIVIO_COMM_H_ */
