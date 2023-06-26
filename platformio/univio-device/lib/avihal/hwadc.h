/*
 *  file:     hwadc.h
 *  brief:    
 *  date:     2023-06-26
 *  authors:  nvitya
*/

#ifndef _AVIHAL_HWADC_H_
#define _AVIHAL_HWADC_H_

#include "Arduino.h"

class THwAdc
{
public:	// settings
	bool 					 initialized = false;

	int      			 devnum = -1;

	bool           Init(int adevnum, uint32_t achannel_map);
	uint16_t       ChValue(uint8_t ach);
};

#endif