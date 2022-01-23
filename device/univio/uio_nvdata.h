/*
 * uio_nvdata.h
 *
 *  Created on: Dec 16, 2021
 *      Author: vitya
 */

#ifndef UNIVIO_UIO_NVDATA_H_
#define UNIVIO_UIO_NVDATA_H_

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

#endif /* UNIVIO_UIO_NVDATA_H_ */
