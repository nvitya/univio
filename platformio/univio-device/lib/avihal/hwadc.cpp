/*
 *  file:     hwadc.cpp
 *  brief:    
 *  date:     2023-06-26
 *  authors:  nvitya
*/

#include "hwadc.h"

bool THwAdc::Init(int adevnum, uint32_t achannel_map)
{
  devnum = adevnum;

  initialized = true;
  return true;
}

uint16_t THwAdc::ChValue(uint8_t ach)
{
  return 0;
}
