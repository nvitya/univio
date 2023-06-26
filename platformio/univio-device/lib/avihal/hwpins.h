/*
 * hwpins.h
 *
 *  Created on: Sep 3, 2022
 *      Author: vitya
 */

#ifndef AVIHAL_HWPINS_H_
#define AVIHAL_HWPINS_H_

#include "Arduino.h"

#define PINCFG_INPUT          0x0000
#define PINCFG_OUTPUT         0x0001

#define PINCFG_OPENDRAIN      0x0002
#define PINCFG_PULLUP         0x0004
#define PINCFG_PULLDOWN       0x0008

#define PINCFG_ANALOGUE       0x0010

#define PINCFG_DRIVE_NORMAL   0x0000  // default
#define PINCFG_DRIVE_MEDIUM   0x0000
#define PINCFG_DRIVE_WEAK     0x0020
#define PINCFG_DRIVE_STRONG   0x0040
#define PINCFG_DRIVE_MASK     0x0060

#define PINCFG_SPEED_MASK     0x0F00
#define PINCFG_SPEED_MEDIUM   0x0000  // default
#define PINCFG_SPEED_SLOW     0x0100
#define PINCFG_SPEED_MED2     0x0200
#define PINCFG_SPEED_MEDIUM2  0x0200  // alias
#define PINCFG_SPEED_FAST     0x0300
#define PINCFG_SPEED_VERYFAST 0x0400  // special ST value, wich usually does not work even for SDRAM pins

#define PINCFG_GPIO_INVERT    0x4000  // for HW invert support
#define PINCFG_GPIO_INIT_1    0x8000
#define PINCFG_GPIO_INIT_0    0x0000

class TGpioPin
{
public:
  uint8_t  pinnum = 0xFF;
  bool     inverted = false;
  uint32_t flags = 0;

  TGpioPin() { }
  TGpioPin(int apinnum, bool ainverted);
  virtual ~TGpioPin() { } // never destroyed

  void Assign(int apinnum, bool ainverted);

  void Setup(uint32_t aflags);
  inline void Set1() { digitalWrite(pinnum, value1); }
  inline void Set0() { digitalWrite(pinnum, value0); }
  inline void SetTo(uint8_t avalue)
  {
    if (avalue & 1)
      Set1();
    else
      Set0();
  }
  inline int Value() { return digitalRead(pinnum); }

protected:
  int value1 = 1;
  int value0 = 0;
};

#endif /* AVIHAL_HWPINS_H_ */
