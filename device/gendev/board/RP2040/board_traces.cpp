/*
 *  file:     board/RP2040/board_traces.cpp
 *  brief:    Board specific stuff
 *  created:  2022-06-11
 *  authors:  nvitya
*/

#include "board_pins.h"
#include "clockcnt.h"

void board_traces_init()
{
  // console (trace) UART
  hwpinctrl.PinSetup(0,  0, PINCFG_OUTPUT | PINCFG_AF_2); // UART0_TX:
  hwpinctrl.PinSetup(0,  1, PINCFG_INPUT  | PINCFG_AF_2); // UART0_RX:
  traceuart.Init(0);
}

