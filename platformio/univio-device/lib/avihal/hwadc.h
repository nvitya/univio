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

	uint32_t       pin_map = 0;

	bool           Init(uint32_t apin_map);
	uint16_t       ChValue(uint8_t apin);
};

#endif