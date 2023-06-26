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
 *  file:     uio_nvdata.cpp
 *  brief:    UNIVIO GENDEV non-volatile data handling
 *  date:     2022-02-13
 *  authors:  nvitya
*/

#include "string.h"
#include "udoslave.h"
#include "uio_device.h"
#include <uio_nvdata.h>

TUioNvData g_nvdata;

void TUioNvData::Init()
{
  unsigned n;
}

uint16_t TUioNvData::SaveValue(uint8_t aid, uint32_t avalue)
{
  if (lock != UIO_NVDATA_UNLOCK)
  {
    return UDOERR_READ_ONLY;
  }

  aid = (aid & 0x1F);  // ensure safe index

  if (value[aid] == avalue)
  {
    return 0;
  }

  value[aid] = avalue; // update locally first

  return 0;
}
