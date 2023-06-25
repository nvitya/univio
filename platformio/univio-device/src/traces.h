#ifndef SRC_TRACES_H_
#define SRC_TRACES_H_

#include "Arduino.h"

#define TRACE(...)  Serial.printf( __VA_ARGS__ )

void traces_init();

#endif /* SRC_TRACES_H_ */
