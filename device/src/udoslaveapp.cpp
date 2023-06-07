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
 *  file:     udoslaveapp.cpp
 *  brief:    UDO Slave Application Implementation for the UnivIO devices
 *  created:  2023-05-13
 *  authors:  nvitya
*/

#include <udoslaveapp.h>
#include "paramtable.h"

// the udoslave_app_read_write() is called from the communication system (Serial or IP) to
// handle the actual UDO requests
bool udoslave_app_read_write(TUdoRequest * udorq)
{
  if (udorq->index < 0x0100)  // handle the standard UDO indexes (0x0000 - 0x0100)
  {
    return udoslave_handle_base_objects(udorq);
  }


  if (param_read_write(udorq)) // try the parameter table first
  {
    return true;
  }


  return false;
}
