/*
 * mscounter.cpp
 *
 *  Created on: May 21, 2023
 *      Author: vitya
 */

#include "mscounter.h"
#include "clockcnt.h"

bool     mscounter_initialized = false;

uint32_t mscounter_last_clocks = 0;
uint32_t mscounter_clocks_per_ms = 0;
uint32_t mscounter_counter = 0;

uint32_t mscounter()
{
  if (!mscounter_initialized)
  {
    mscounter_clocks_per_ms = SystemCoreClock / 1000;
    mscounter_initialized = true;
    mscounter_last_clocks = CLOCKCNT;
  }

  unsigned elapsed_clk = CLOCKCNT - mscounter_last_clocks;
  unsigned elapsed_ms = elapsed_clk / mscounter_clocks_per_ms;
  mscounter_counter += elapsed_ms;
  mscounter_last_clocks += elapsed_ms * mscounter_clocks_per_ms;
  return mscounter_counter;
}
