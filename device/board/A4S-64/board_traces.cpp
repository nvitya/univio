/*
 *  file:     board/mibo64_atsam4s/board_traces.cpp
 *  brief:    Board specific stuff
 *  version:  1.00
 *  date:     2021-11-07
 *  authors:  nvitya
*/

#include "board_pins.h"

void board_traces_init()
{
  // console (trace) UART
  hwpinctrl.PinSetup(PORTNUM_A, 6,  PINCFG_OUTPUT | PINCFG_AF_A);  // USART0_TX
  traceuart.Init(0x100);
}

