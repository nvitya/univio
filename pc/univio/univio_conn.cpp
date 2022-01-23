/*
 * univio_conn.cpp
 *
 *  Created on: Nov 21, 2021
 *      Author: vitya
 */

#include "string.h"
#include "univio_conn.h"

#define TRACE_COMM 0

TUnivioConn::TUnivioConn()
{
  // nothing yet
}

TUnivioConn::~TUnivioConn()
{
  Close();
}

bool TUnivioConn::Open(const char * aserdevname)
{
	strncpy(&serdevname[0], aserdevname, sizeof(serdevname));

	comm.Close();
  comm.baudrate = 1000000;
	if (!comm.Open(aserdevname))
	{
		return false;
	}

	return true;
}

void TUnivioConn::Close()
{
	comm.Close();
}

uint16_t TUnivioConn::Read(uint16_t aaddr, void * pdst, uint16_t alen, uint16_t * rlen)
{
	rq.address = aaddr;
	rq.metalen = 0;
	rq.length = alen;
	rq.iswrite = 0;

	ExecRequest();

	if (0 == rq.result)
	{
		if (rq.length > alen)
		{
			rq.result = UIOERR_DATA_TOO_BIG;
			return rq.result;
		}

		if (rq.length < alen)
		{
			memset(pdst, 0, alen);
		}

		memcpy(pdst, &rq.data[0], rq.length);

		if (rlen)
		{
			*rlen = rq.length;
		}
	}

	return rq.result;
}

uint16_t TUnivioConn::Write(uint16_t aaddr, void * psrc, uint16_t len)
{
	rq.address = aaddr;
	rq.metalen = 0;
	rq.length = len;
	rq.iswrite = 1;
	if (len > sizeof(rq.data))
	{
		return UIOERR_DATA_TOO_BIG;
	}
	memcpy(&rq.data[0], psrc, len);

	ExecRequest();

	return rq.result;
}

uint16_t TUnivioConn::ReadUint32(uint16_t aaddr, uint32_t * rdata)
{
	rq.address = aaddr;
	rq.metalen = 0;
	rq.length = 4;
	rq.iswrite = 0;

	ExecRequest();

	if (0 == rq.result)
	{
		if (4 <= rq.length)
		{
			*rdata = *(uint32_t *)&rq.data[0];
		}
		else if (2 == rq.length)
		{
			*rdata = *(uint16_t *)&rq.data[0];
		}
		else
		{
			*rdata = *(uint8_t *)&rq.data[0];
		}
	}

	return rq.result;
}

uint16_t TUnivioConn::WriteUint(uint16_t aaddr, uint32_t adata, unsigned alen)
{
	rq.address = aaddr;
	rq.metalen = 0;
	rq.length = alen;
	rq.iswrite = 1;
	*(uint32_t *)&rq.data[0] = adata;

	ExecRequest();

	return rq.result;
}

void TUnivioConn::ExecRequest()
{
	int r;
	uint8_t  b;

	bufcnt = 0;
	crc = 0;

	comm.Flush();

	// prepare the request

	b = 0x55;
	AddTx(&b, 1);
	b = (rq.iswrite ? 1 : 0);
	if      (2 == rq.metalen)  b |= (1 << 2);
	else if (4 == rq.metalen)  b |= (2 << 2);
	else if (8 == rq.metalen)  b |= (3 << 2);

	if (rq.length >= 15)
	{
		b |= 0xF0;
	}
	else
	{
		b |= (rq.length << 4);
	}
	AddTx(&b, 1);

	if (rq.length >= 15)
	{
		AddTx(&rq.length, 2);
	}
	AddTx(&rq.address, 2);

	if (rq.metalen)
	{
		AddTx(&rq.metadata[0], rq.metalen);
	}

	if (rq.iswrite && rq.length)
	{
		AddTx(&rq.data[0], rq.length);
	}

	AddTx(&crc, 1);

	// send the request

#if TRACE_COMM
  	printf(">> ");
  	for (unsigned bi = 0; bi < bufcnt; ++bi)  printf(" %02X", buf[bi]);
  	printf("\n");
#endif

	r = comm.Write(&buf[0], bufcnt);
	if (r <= 0)
	{
		rq.result = UIOERR_CONNECTION;
		return;
	}

	if (unsigned(r) != bufcnt)
	{
		rq.result = UIOERR_CONNECTION;
		return;
	}

	// receive the response

	bufcnt = 0;
	rxstate = 0;
	rxcnt = 0;
	rxreadpos = 0;
	crc = 0;

	lastrecvtime = nstime();

  while (true)
  {
  	r = comm.Read(&buf[bufcnt], sizeof(buf) - bufcnt);
  	if (r <= 0)
  	{
  		if (r == -EAGAIN)
  		{
  			if (nstime() - lastrecvtime > 1000000000) //receive_timeout_us * 1000)
  			{
  				rq.result = UIOERR_CONNECTION;
  				return;
  			}
  			continue;
  		}
  		rq.result = UIOERR_CONNECTION;
    	return;
  	}

  	lastrecvtime = nstime();

#if TRACE_COMM
  	printf("<< ");
  	for (unsigned bi = 0; bi < r; ++bi)  printf(" %02X", buf[bufcnt + bi]);
  	printf("\n");
#endif

  	bufcnt += r;

  	while (rxreadpos < bufcnt)
  	{
			uint8_t b = buf[rxreadpos];

			if ((rxstate > 0) && (rxstate < 10))
			{
				crc = univio_calc_crc(crc, b);
			}

			if (0 == rxstate)  // waiting for the sync byte
			{
				if (0x55 == b)
				{
					crc = univio_calc_crc(0, b); // start the CRC from zero
					rxstate = 1;
				}
			}
			else if (1 == rxstate) // command and length
			{
				rq.iswrite = (b & 1); // bit0: 0 = read, 1 = write
				iserror = (0 != (b & 2));

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
						if (!rq.iswrite || iserror)
						{
							rxstate = 5;  // read data or error code
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
					if (!rq.iswrite || iserror)
					{
						rxcnt = 0;
						rxstate = 5; // read data or error code follows
					}
					else
					{
						rxstate = 10;  // crc check
					}
				}
			}
			else if (5 == rxstate) // read data or error code
			{
				if (iserror && (rq.length != 2))
				{
					rq.length = 2;
				}
				rq.data[rxcnt] = b;
				++rxcnt;
				if (rxcnt >= rq.length)
				{
					rxstate = 10;
				}
			}
			else if (10 == rxstate) // crc check
			{
				if (b != crc)
				{
					rq.result = UIOERR_CRC;
					return;
				}
				else
				{
					if (iserror)
					{
						rq.result = *(uint16_t *)&rq.data[0];
					}
					else
					{
						rq.result = 0;
					}
					return;
				}
			}

			++rxreadpos;
  	}
  }
}

unsigned TUnivioConn::AddTx(void * asrc, unsigned len) // returns the amount actually written
{
  unsigned available = TxAvailable();
  if (0 == available)
  {
    return 0;
  }

  if (len > available)  len = available;

  uint8_t * srcp = (uint8_t *)asrc;
  uint8_t * dstp = &buf[bufcnt];
  uint8_t * endp = dstp + len;
  while (dstp < endp)
  {
    uint8_t b = *srcp++;
    *dstp++ = b;
    crc = univio_calc_crc(crc, b);
  }

  bufcnt += len;

  return len;
}
