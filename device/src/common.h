/*
 *  file:     common.h
 *  brief:    Some project specific commonly used definitions
 *  version:  1.00
 *  date:     2021-11-18
 *  authors:  nvitya
*/

#pragma once

#include "platform.h"

#define US_TO_CLOCKS(aus)      (aus * (MCU_CLOCK_SPEED / 1000000))
#define MS_TO_CLOCKS(ams)      (ams * (MCU_CLOCK_SPEED / 1000))

#define CLOCKS_TO_US(aclk)     (aclk / (MCU_CLOCK_SPEED / 1000000))
#define CLOCKS_TO_MS(aclk)     (aclk / (MCU_CLOCK_SPEED / 1000))


