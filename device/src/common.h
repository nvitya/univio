/*
 * common.h
 *
 *  Created on: Nov 20, 2021
 *      Author: vitya
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include "platform.h"

#define US_TO_CLOCKS(aus)      (aus * (MCU_CLOCK_SPEED / 1000000))
#define MS_TO_CLOCKS(ams)      (ams * (MCU_CLOCK_SPEED / 1000))

#define CLOCKS_TO_US(aclk)     (aclk / (MCU_CLOCK_SPEED / 1000000))
#define CLOCKS_TO_MS(aclk)     (aclk / (MCU_CLOCK_SPEED / 1000))

#endif /* SRC_COMMON_H_ */
