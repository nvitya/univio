/*
 * univio_conn.h
 *
 *  Created on: Nov 21, 2021
 *      Author: vitya
 */

#ifndef SRC_UNIVIO_CONN_H_
#define SRC_UNIVIO_CONN_H_

#include "sercomm.h"
#include "univio.h"
#include "nstime.h"

class TUnivioConn
{
public:
	TSerComm        comm;
	char            serdevname[64];

	TUnivioRequest  rq;

	uint8_t         crc = 0;
	bool            iserror = false;
	uint8_t         buf[UNIVIO_MAX_DATA_LEN * 2];
	unsigned        bufcnt = 0;
	unsigned        rxcnt = 0;
	unsigned        rxreadpos = 0;
	unsigned        rxstate = 0;

	unsigned        receive_timeout_us = 100000;  // 100 ms
  nstime_t        lastrecvtime = 0;

	TUnivioConn();
	virtual ~TUnivioConn();

	bool Open(const char * aserdevname);
	void Close();

	uint16_t          Read(uint16_t aaddr, void * pdst, uint16_t alen, uint16_t * rlen);
	uint16_t          Write(uint16_t aaddr, void * psrc, uint16_t len);

	uint16_t          ReadUint32(uint16_t aaddr, uint32_t * rdata);
	uint16_t          WriteUint(uint16_t aaddr, uint32_t adata, unsigned alen);

	void              ExecRequest();
	unsigned          AddTx(void * asrc, unsigned len);
  inline unsigned   TxAvailable() { return sizeof(buf) - bufcnt; }

};

#endif /* SRC_UNIVIO_CONN_H_ */
