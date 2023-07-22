/*
 *  file:     hwadc.cpp
 *  brief:    
 *  date:     2023-06-26
 *  authors:  nvitya
*/

#include "hwadc.h"

bool THwAdc::Init(uint32_t apin_map)
{
  pin_map = apin_map;

  for (unsigned n = 0; n < 31; ++n)
  {
    if (pin_map & (1 << n))
    {
      adcAttachPin(n);
    }
  }

  initialized = true;
  return true;
}

uint16_t THwAdc::ChValue(uint8_t apin)
{
  return (analogRead(apin) << 4);
}
