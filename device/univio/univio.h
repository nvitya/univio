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
 *  file:     univio.h
 *  brief:    UNIVIO basic definitions / utilities
 *  version:  1.00
 *  date:     2022-02-13
 *  authors:  nvitya
*/

#pragma once

#include "stdint.h"
#include "platform.h"

#define UNIVIO_UART_BAUDRATE  1000000

#ifndef UIO_MAX_DATA_LEN
  #define UIO_MAX_DATA_LEN     256
#endif

#define UIOERR_CONNECTION       0x1001  // not connected, send / receive error
#define UIOERR_CRC              0x1002
#define UIOERR_TIMEOUT          0x1003
#define UIOERR_DATA_TOO_BIG     0x1004

#define UIOERR_WRONG_ADDR       0x2001  // address not existing
#define UIOERR_WRONG_ACCESS     0x2002
#define UIOERR_READ_ONLY        0x2003
#define UIOERR_WRITE_ONLY       0x2004
#define UIOERR_VALUE            0x2005  // invalid value
#define UIOERR_RUN_MODE         0x2006  // config mode required
#define UIOERR_UNITSEL          0x2007  // the referenced unit is not existing
#define UIOERR_BUSY             0x2008

#define UIOERR_NOT_IMPLEMENTED  0x9001
#define UIOERR_INTERNAL         0x9002  // internal implementation error

typedef struct
{
  uint8_t      iswrite;
  uint8_t      metalen;
  uint16_t     result;
  uint16_t     address;
  uint16_t     length;

  uint8_t      metadata[8];

  uint8_t      data[UIO_MAX_DATA_LEN];
//
} TUnivioRequest;

uint8_t univio_calc_crc(uint8_t acrc, uint8_t adata);

