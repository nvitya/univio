
#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "Arduino.h"

extern bool          g_wifi_connected;

void board_net_init();
void board_pins_init();

#endif /* SRC_BOARD_PINS_H_ */
