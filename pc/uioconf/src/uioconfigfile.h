/*
 * uioconfigfile.h
 *
 *  Created on: Dec 2, 2021
 *      Author: vitya
 */

#ifndef SRC_UIOCONFIGFILE_H_
#define SRC_UIOCONFIGFILE_H_

#include <string>
#include "udo_comm.h"
#include "configfileparser.h"

#define UIO_MAX_PINS      256

// fix maximums
#define UIO_PWM_COUNT       8
#define UIO_ADC_COUNT      16
#define UIO_DAC_COUNT       8
#define UIO_DOUT_COUNT     32
#define UIO_DIN_COUNT      32
#define UIO_LEDBLP_COUNT   16

#define UIO_PINTYPE_PASSIVE          0 // default configuration
#define UIO_PINTYPE_DIG_IN           1 // pullup by default
#define UIO_PINTYPE_DIG_OUT          2
#define UIO_PINTYPE_ADC_IN           3
#define UIO_PINTYPE_DAC_OUT          4
#define UIO_PINTYPE_PWM_OUT          5
#define UIO_PINTYPE_LEDBLP           6
#define UIO_PINTYPE_SPI              7
#define UIO_PINTYPE_I2C              8
#define UIO_PINTYPE_UART             9
#define UIO_PINTYPE_CLKOUT          10


class TUioConfig : public TConfigFileParser
{
public:
	string         fwid = "";
	string         deviceid = "";
	string         manufacturer = "UnivIO";
	string         serialnum = "1";

	uint16_t       usb_vid = 0xDEAD;
	uint16_t       usb_pid = 0xBEE0;

	int            pins_per_port = 0; // must be set

public:
	uint32_t       pinconf[UIO_MAX_PINS];

	uint32_t       dout_value = 0;
	uint16_t       aout_value[UIO_DAC_COUNT];
	uint16_t       pwm_value[UIO_PWM_COUNT];
	uint32_t       pwm_freq[UIO_PWM_COUNT];
	uint32_t       ledblp_value[UIO_LEDBLP_COUNT];

public:
  virtual void   ResetConfig();
  virtual bool   ParseConfigLine(string idstr);

  bool           ParsePinConf();

public:
  uint8_t        dev_max_pins = 0;

  uint8_t        databuf[UDO_MAX_DATALEN];

  bool           CheckDevice();

  bool           SaveToFile(const char * afname);
  bool           LoadFromDevice();
  bool           SaveToDevice();

  string         GetPinName(uint8_t apinnum);
};

extern TUioConfig  uioconfig;

#endif /* SRC_UIOCONFIGFILE_H_ */
