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
 *  file:     uio_nvdata.h
 *  brief:    UNIVIO GENDEV non-volatile data handling
 *  version:  1.00
 *  date:     2022-02-13
 *  authors:  nvitya
*/

#pragma once

#include "stdint.h"

#define UIO_NVDATA_SIGNATURE  0x55AAAA55
#define UIO_NVDATA_SECTORS             2
#define UIO_NVDATA_COUNT              32
#define UIO_NVDATA_UNLOCK     0x5ADEC0DE

typedef struct
{
  uint8_t     id;
  uint8_t     id_not;
  uint8_t     pad[2];
  uint32_t    value;
//
} TUioNvDataChRec;

typedef struct
{
  uint32_t         signature;
  uint32_t         serial;
  uint32_t         value[UIO_NVDATA_COUNT];
//
} TUioNvDataHead;

class TUioNvData
{
public:
  uint32_t     lock = 0;
  uint32_t     sector_size = 1024;  // will be adjusted to the erase size
  uint32_t     nvsaddr_base = 0;

  uint8_t      sector_index = 0;
  uint16_t     chrec_count = 0;

  uint32_t     value[UIO_NVDATA_COUNT] = {0};

  void         Init();
  uint16_t     SaveValue(uint8_t aid, uint32_t avalue);
};

extern TUioNvData g_nvdata;

