/*
 *  file:     traces.cpp
 *  brief:    Buffered trace message redirection to UART, init required !
 *  created:  2021-10-02
 *  authors:  nvitya
*/

#include "traces.h"
#include "board_pins.h"

#ifndef TRACE_BUFFER_SIZE
  #define TRACE_BUFFER_SIZE  1024
#endif

char trace_buffer[TRACE_BUFFER_SIZE];

void traces_init()
{
  board_traces_init();
  tracebuf.waitsend = false; // the USB requires fast processing !
	tracebuf.Init(&traceuart, &trace_buffer[0], sizeof(trace_buffer));
}




