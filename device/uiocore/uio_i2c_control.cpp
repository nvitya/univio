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
 *  file:     uio_i2c_control.cpp
 *  brief:    UNIVIO I2C controller interface
 *  date:     2024-09-10
 *  authors:  nvitya
*/

#include "uio_i2c_control.h"
#include "uio_dev_base.h"

THwI2c       g_i2c[UIO_I2C_COUNT];
TUioI2cCtrl  g_i2cctrl[UIO_I2C_COUNT];

void TUioI2cCtrl::Init(TUioDevBase * adevbase, THwI2c * ai2c)
{
  devbase = adevbase;
  i2c = ai2c;
  i2ctra.completed = true;
}

uint16_t TUioI2cCtrl::I2cStart()
{
  if (0xFFFF == i2c_result)
  {
    return UDOERR_BUSY;
  }

  if (!i2c)
  {
    return UIOERR_UNITSEL;
  }

  if (!i2ctra.completed)
  {
    return UDOERR_BUSY;
  }

  i2c_trlen = (i2c_cmd >> 16);
  uint32_t edata_len = ((i2c_cmd >> 12) & 3);

  if ((0 == i2c_speed) || (i2c_trlen == 0) || (i2c_trlen > UIO_MPRAM_SIZE - i2c_data_offs))
  {
    return UIOERR_UNIT_PARAMS;
  }

  if (i2c->speed != i2c_speed)
  {
    i2c->speed = i2c_speed;
  }

  // RP2040 bug: re-init (? reset) required sometimes
  i2c->Init(i2c->devnum); // re-init the device

  uint8_t   addr   = (i2c_cmd >> 1) & 0x7F;
  uint32_t  extra  = (i2c_eaddr & 0xFFFFFF) | (edata_len << 24);

  if (i2c_cmd & UIO_I2C_CMD_WRITE)
  {
    i2c->StartWrite(&i2ctra, addr, extra, &devbase->mpram[i2c_data_offs], i2c_trlen);
  }
  else
  {
    i2c->StartRead(&i2ctra,  addr, extra, &devbase->mpram[i2c_data_offs], i2c_trlen);
  }

  i2c_result = 0xFFFF;

  return 0;
}

void TUioI2cCtrl::Run()
{
  if (0xFFFF == i2c_result)
  {
    i2c->Run();
    if (i2ctra.completed)
    {
      i2c_result = i2ctra.error;
    }
  }
}

bool TUioI2cCtrl::prfn_I2cControl(TUdoRequest * rq, TParamRangeDef * prdef)
{
  uint8_t idx  = (rq->index & 0x1F);

  if (0x00 == idx) // I2C Speed
  {
    return udo_rw_data(rq, &i2c_speed, sizeof(i2c_speed));
  }
  else if (0x01 == idx) // I2C EADDR (Extra Address)
  {
    return udo_rw_data(rq, &i2c_eaddr, sizeof(i2c_eaddr));
  }
  else if (0x02 == idx) // I2C Transaction Start
  {
    if (rq->iswrite)
    {
      i2c_cmd = udorq_uintvalue(rq);
      uint16_t err = I2cStart();
      return udo_response_error(rq, err); // will be response ok with err=0
    }
    else
    {
      return udo_ro_uint(rq, i2c_cmd, 1);
    }
  }
  else if (0x03 == idx) // I2C Transaction Status / Result
  {
    return udo_rw_data(rq, &i2c_result, sizeof(i2c_result));
  }
  else if (0x04 == idx) // I2C data MPRAM offset
  {
    return udo_rw_data(rq, &i2c_data_offs, sizeof(i2c_data_offs));
  }

  return udo_response_error(rq, UDOERR_INDEX);
}


