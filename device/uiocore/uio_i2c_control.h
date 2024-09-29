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
 *  file:     uio_i2c_control.h
 *  brief:    UNIVIO I2C controller interface
 *  date:     2024-09-10
 *  authors:  nvitya
*/

#ifndef UIOCORE_UIO_I2C_CONTROL_H_
#define UIOCORE_UIO_I2C_CONTROL_H_

#include "tclass.h"
#include "udo.h"
#include "udoslave.h"
#include "simple_partable.h"

#include "hwi2c.h"
#include "hwdma.h"

#include "uio_common.h"

class TUioDevBase;

class TUioI2cCtrl : public TClass
{
public:
  TUioDevBase *     devbase = nullptr;

  THwI2c *          i2c = nullptr;
  TI2cTransaction   i2ctra;
  uint16_t          i2c_data_offs = 0;
  uint32_t          i2c_speed = 100000;
  uint32_t          i2c_eaddr = 0;
  uint32_t          i2c_cmd = 0;
  uint16_t          i2c_trlen = 0;
  uint16_t          i2c_result = 0;

  void              Init(TUioDevBase * adevbase, THwI2c * ai2c);
  void              Run();
  uint16_t          I2cStart();
  bool              prfn_I2cControl(TUdoRequest * rq, TParamRangeDef * prdef);

};

extern THwI2c       g_i2c[UIO_I2C_COUNT];
extern TUioI2cCtrl  g_i2cctrl[UIO_I2C_COUNT];

#endif /* UIOCORE_UIO_I2C_CONTROL_H_ */
