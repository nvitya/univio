/*
 *  file:     hwpwm.h
 *  brief:    Arduino / ESP32 PWM Channel Definition
 *  date:     2023-06-26
 *  authors:  nvitya
*/

#ifndef AVIHAL_HWPWM_H_
#define AVIHAL_HWPWM_H_

#include "Arduino.h"
#include "driver/ledc.h"

class THwPwmChannel
{
public: // parameters

	uint32_t      frequency = 10000;
	bool          inverted = false;

public:

	uint32_t      periodclocks = 0;    // contains the (1 << bit_resolution) here for compatibility
	uint32_t      bit_resolution = 0;

public: // utility
	uint8_t       pinnum = 0;
	uint8_t       chnum = 0;
	uint8_t       outnum = 0;  // 0 = A, 1 = B

	bool          initialized = false;

public: // mandatory
	bool          Init(int apinnum, int achnum);  // might vary from hw to hw

	void          SetFrequency(uint32_t afrequency);
	void          SetOnClocks(uint16_t aclocks);
	void          Enable();
	void          Disable();
	inline bool   Enabled() { return is_enabled; };

protected:
  uint16_t        last_on_clocks = 0;	
	bool            is_enabled = false;
	ledc_timer_t    timer_id;
	ledc_mode_t     mode_id;
	ledc_channel_t  ch_id;
};

#endif
