/*
 *  file:     udoslaveapp.cpp
 *  brief:    UDO Slave Application Implementation (udo request handling)
 *  created:  2023-05-13
 *  authors:  nvitya
 *  license:  public domain
*/

#include <udoslaveapp.h>
#include "paramtable.h"

// the udoslave_app_read_write() is called from the communication system (Serial or IP) to
// handle the actual UDO requests
bool udoslave_app_read_write(TUdoRequest * udorq)
{
  if (udorq->index < 0x0100)  // handle the standard UDO indexes (0x0000 - 0x00FF)
  {
    return udoslave_handle_base_objects(udorq);
  }

  // then handle the request with the parameter table
  return param_read_write(udorq);
}
   