/* -----------------------------------------------------------------------------
 * This file is a part of the UNIVIO project: https://github.com/nvitya/univio
 * Copyright (c) 2023 Viktor Nagy, nvitya
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
 *  file:     uio_device.cpp
 *  brief:    UNIVIO device final instance, UDO request handling
 *  created:  2023-05-13
 *  authors:  nvitya
*/

#ifndef _UIO_DEVICE_H
#define _UIO_DEVICE_H

#include "uio_gendev_impl.h"
#include "paramtable.h"

class TUioDevice : public TUioGenDevImpl
{
public:
  bool       prfn_0100_DevId(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_0180_DevConf(TUdoRequest * rq, TParamRangeDef * prdef);

  bool       prfn_PinCfgReset(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_PinConfig(TUdoRequest * rq, TParamRangeDef * prdef);

  bool       prfn_DefValue_DigOut(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_DefValue_AnaOut(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_DefValue_PwmDuty(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_DefValue_PwmFreq(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_DefValue_LedBlp(TUdoRequest * rq, TParamRangeDef * prdef);

  bool       prfn_ConfigInfo(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_NvData(TUdoRequest * rq, TParamRangeDef * prdef);

  bool       prfn_DigOutSetClr(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_DigOutDirect(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_DigInValues(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_AnaInValues(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_AnaOutCtrl(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_PwmControl(TUdoRequest * rq, TParamRangeDef * prdef);
  bool       prfn_LedBlpCtrl(TUdoRequest * rq, TParamRangeDef * prdef);

  bool       prfn_Mpram(TUdoRequest * rq, TParamRangeDef * prdef);

};

extern TUioDevice   g_uiodev;

#endif
