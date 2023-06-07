/*
 *  file:     board/SG473-48/board_traces.cpp
 *  brief:    Board specific stuff
 *  version:  1.00
 *  date:     2022-01-30
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "clockcnt.h"

void board_traces_init()
{
  // console (trace) UART
  hwpinctrl.PinSetup(PORTNUM_A, 9,  PINCFG_OUTPUT | PINCFG_AF_7);  // USART1_TX
  traceuart.Init(1);
}

