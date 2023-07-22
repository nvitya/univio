/*
 * hwpins.cpp
 *
 *  Created on: Sep 3, 2022
 *      Author: vitya
 */

#include "hwpins.h"

TGpioPin::TGpioPin(int apinnum, bool ainverted)
{
  Assign(apinnum, ainverted);
}

void TGpioPin::Assign(int apinnum, bool ainverted)
{
  pinnum = apinnum;
  inverted = ainverted;
  if (inverted)
  {
    value0 = 1;
    value1 = 0;
  }
  else
  {
    value1 = 1;
    value0 = 0;
  }
}

void TGpioPin::Setup(uint32_t aflags)
{
  flags = aflags;
  if (flags & PINCFG_OUTPUT)
  {
    if (flags & PINCFG_GPIO_INIT_1)
    {
      Set1();
    }
    else
    {
      Set0();
    }
    pinMode(pinnum, OUTPUT);
  }
  else if (flags & PINCFG_ANALOGUE)
  {
    adcAttachPin(pinnum);
  }
  else
  {
    pinMode(pinnum, INPUT);
  }
}
