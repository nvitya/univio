/*
 *  file:     hwpwm.h
 *  brief:    Arduino / ESP32 PWM Channel Definition
 *  date:     2023-06-26
 *  authors:  nvitya
*/

#ifndef AVIHAL_HWPWM_H_
#define AVIHAL_HWPWM_H_

#include "Arduino.h"

class THwPwmChannel
{
public: // parameters

	uint32_t      frequency = 10000;
	bool          inverted = false;

public:

	uint32_t      periodclocks = 0;
	uint32_t      cpu_clock_shifts = 0;

public: // utility
	uint8_t       devnum = 0;
	uint8_t       chnum = 0;
	uint8_t       outnum = 0;  // 0 = A, 1 = B

	bool          initialized = false;

public: // mandatory
	bool          Init(int adevnum, int chnum)     { return false; }  // might vary from hw to hw

	void          SetFrequency(uint32_t afrequency) { }
	void          SetOnClocks(uint16_t aclocks) { }
	void          Enable()  { }
	void          Disable() { }
	inline bool   Enabled() { return false; }
};

#endif
