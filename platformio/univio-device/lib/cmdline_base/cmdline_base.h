/* -----------------------------------------------------------------------------
 * This file is a part of the UDOTESTS project: https://github.com/nvitya/udotests
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
 *  file:     cmdline_base.h
 *  brief:    base class for interactive command line (Arduino version)
 *  created:  2023-05-31
 *  authors:  nvitya
*/

#ifndef SRC_CMDLINE_BASE_H_
#define SRC_CMDLINE_BASE_H_

#include "Arduino.h"
#include "strparse.h"

#define CMDLINE_MAX_RX_MSG_LEN  256  // maximal length of a parsed message

class TCmdLineBase
{
public:
  uint16_t          cmdlen = 0;
  bool              initialized = false;

  TStrParseObj      sp;

  const char *      prompt = "cmd> ";
  uint8_t           cmdbuf[CMDLINE_MAX_RX_MSG_LEN];  // parsed message buffer

  virtual           ~TCmdLineBase() { }

  void              Init();
  virtual bool      InitHw();

  bool              ExecCmd();
  virtual bool      ParseCmd();

  void              WritePrompt();

  void              Run();  // processes Rx

};

#endif /* SRC_CMDLINE_BASE_H_ */
