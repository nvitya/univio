/*
 *  file:     board/RP2040/board_spiflash.cpp
 *  brief:    SPI Flash Initialization
 *  created:  2022-06-11
 *  authors:  nvitya
*/

#include "board_pins.h"

TSpiFlash  spiflash;
THwQspi    fl_qspi;

bool spiflash_init()
{
  // because of the transfers are unidirectional the same DMA channel can be used here:
  fl_qspi.txdmachannel = DMACH_QSPI;
  fl_qspi.rxdmachannel = DMACH_QSPI;
  // for read speeds over 24 MHz dual or quad mode is required.
  // the writes are forced to single line mode (SSS) because the RP does not support SSM mode at write
  fl_qspi.multi_line_count = 4;
  fl_qspi.speed = 32000000;
  fl_qspi.Init();

  spiflash.qspi = &fl_qspi;
  spiflash.has4kerase = true;

  return spiflash.Init();
}




