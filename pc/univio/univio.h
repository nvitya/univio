/*
 * univio.h
 *
 *  Created on: Nov 21, 2021
 *      Author: vitya
 */

#ifndef SRC_UNIVIO_H_
#define SRC_UNIVIO_H_

#include "stdint.h"

#define UNIVIO_UART_BAUDRATE  1000000

#define UNIVIO_MAX_DATA_LEN     256

#define UIOERR_CONNECTION       0x1001  // not connected, send / receive error
#define UIOERR_CRC              0x1002
#define UIOERR_TIMEOUT          0x1003
#define UIOERR_DATA_TOO_BIG     0x1004  // the provided buffer was too small to store the read response
                                        // the provided buffer for write was too big for the request

#define UIOERR_WRONG_ADDR       0x2001  // address not existing
#define UIOERR_WRONG_ACCESS     0x2002
#define UIOERR_READ_ONLY        0x2003
#define UIOERR_WRITE_ONLY       0x2004
#define UIOERR_VALUE            0x2005  // invalid value
#define UIOERR_RUN_MODE         0x2006  // config mode required
#define UIOERR_UNITSEL          0x2007  // the referenced unit is not existing


typedef struct
{
  uint8_t      iswrite;
  uint8_t      metalen;
  uint16_t     result;
  uint16_t     address;
  uint16_t     length;

  uint8_t      metadata[8];

  uint8_t      data[UNIVIO_MAX_DATA_LEN];
//
} TUnivioRequest;

uint8_t univio_calc_crc(uint8_t acrc, uint8_t adata);

#endif /* SRC_UNIVIO_H_ */
