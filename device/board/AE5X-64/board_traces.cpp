/*
 *  file:     board/AE5X-64/board_traces.cpp
 *  brief:    Board specific stuff
 *  version:  1.00
 *  date:     2021-11-07
 *  authors:  nvitya
*/

#include "board_pins.h"

void board_traces_init()
{
  // console (trace) UART
  hwpinctrl.PinSetup(PORTNUM_A, 0,  PINCFG_OUTPUT | PINCFG_AF_D);  // SERCOM0[0]
  traceuart.Init(1);
}

