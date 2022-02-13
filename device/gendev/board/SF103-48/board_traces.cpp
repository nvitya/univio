/*
 *  file:     board/SF103-48/board_traces.cpp
 *  brief:    Board specific stuff
 *  version:  1.00
 *  date:     2021-12-08
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "clockcnt.h"

void board_traces_init()
{
  // console (trace) UART
  hwpinctrl.PinSetup(PORTNUM_A, 9,  PINCFG_OUTPUT | PINCFG_AF_0);  // USART1_TX
  traceuart.Init(1);
}

