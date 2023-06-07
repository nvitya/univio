/*
 *  file:     board/RP2040/app_header.cpp
 *  brief:    Application header for self-flashing with our own bootloader stage 2
 *  created:  2022-06-11
 *  authors:  nvitya
*/

#include "platform.h"
#include "app_header.h"

extern unsigned __app_image_end;

extern "C" void cold_entry();

__attribute__((section(".application_header"),used))
const TAppHeader application_header =
{
  .signature = APP_HEADER_SIGNATURE,
  .length = unsigned(&__app_image_end) - 0x21000000,
	.addr_load = 0x21000000,
	.addr_entry = (unsigned)cold_entry,

	.customdata = 0,
	.compid = 0,
	.csum_body = 0,
	.csum_head = 0
};
