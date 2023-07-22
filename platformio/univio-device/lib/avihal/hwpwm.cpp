/*
 *  file:     hwpwm.cpp
 *  brief:    Arduino / ESP32 PWM Channel (using LEDC)
 *  date:     2023-06-26
 *  authors:  nvitya
*/

#include "hwpwm.h"
#include "Arduino.h"
#include "driver/ledc.h"

bool THwPwmChannel::Init(int apinnum, int achnum)
{
  pinnum = apinnum;
  chnum  = achnum;
  is_enabled = false;

  timer_id = ledc_timer_t(chnum >> 1);
  mode_id = ledc_mode_t(0);  // ??? force high-speed mode ?
  ch_id = ledc_channel_t(chnum);

  ledcAttachPin(pinnum, chnum);
  SetFrequency(frequency);

  initialized = true;

  return true;
}

void THwPwmChannel::SetFrequency(uint32_t afrequency)
{
  frequency = afrequency;

  unsigned basefreq = 80000000;  // arduino uses 40 MHz (clock)

  unsigned real_pclocks = basefreq / frequency;
  bit_resolution = 14;
  while ((real_pclocks < (1 << bit_resolution)) && (bit_resolution > 2))
  {
    --bit_resolution;
  }

  periodclocks = (1 << bit_resolution) - 1;

  ledc_timer_config_t ledc_timer = 
  {
      .speed_mode       = mode_id,
      .duty_resolution  = ledc_timer_bit_t(bit_resolution),
      .timer_num        = timer_id,
      .freq_hz          = frequency,
      .clk_cfg          = LEDC_USE_APB_CLK
  };

  if (ledc_timer_config(&ledc_timer) != ESP_OK)
  {
    // todo: some error handling...
  }
}

void THwPwmChannel::SetOnClocks(uint16_t aclocks)
{  
  last_on_clocks = aclocks;
  if (is_enabled)
  {
    ledc_set_duty(mode_id, ch_id, last_on_clocks);
    ledc_update_duty(mode_id, ch_id);
  }
}

void THwPwmChannel::Enable()
{
  ledc_set_duty(mode_id, ch_id, last_on_clocks);
  ledc_update_duty(mode_id, ch_id);
  is_enabled = true;
}

void THwPwmChannel::Disable()
{
  ledc_set_duty(mode_id, ch_id, 0);
  ledc_update_duty(mode_id, ch_id);
  is_enabled = false;
}
