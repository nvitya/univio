/*
 *  file:     board_pins.h
 *  brief:    Required board specific interface definitions
 *  version:  1.00
 *  date:     2021-11-18
 *  authors:  nvitya
*/

#ifndef SRC_BOARD_PINS_H_
#define SRC_BOARD_PINS_H_

#include "hwpins.h"
#include "hwuart.h"
#include "spiflash.h"

extern THwUart           traceuart;
extern TSpiFlash         extflash;

#if HAS_SPI_FLASH
  extern TSpiFlash  spiflash;

  bool spiflash_init();
#endif

void board_traces_init();

#endif /* SRC_BOARD_PINS_H_ */
