/*
 * uioconfigfile.cpp
 *
 *  Created on: Dec 2, 2021
 *      Author: vitya
 */

#include "string.h"
#include <string>
#include "general.h"
#include "uioconfigfile.h"
#include "udo_comm.h"

using namespace std;

TUioConfig  uioconfig;

bool TUioConfig::ParseConfigLine(string idstr)
{
	unsigned n;
	int i, idx;

	if (("FWID" == idstr) || ("DIID" == idstr))
	{
		diid = ParseStringAssignment();
	}
	else if (("DEVICEID" == idstr) || ("DEVID" == idstr))
	{
		deviceid = ParseStringAssignment();
	}
	else if ("MANUFACTURER" == idstr)
	{
		manufacturer = ParseStringAssignment();
	}
	else if (("USB_VID" == idstr) || ("USB_VENDOR_ID" == idstr))
	{
		usb_vid = ParseIntAssignment();
	}
	else if (("USB_PID" == idstr) || ("USB_PRODUCT_ID" == idstr))
	{
		usb_pid = ParseIntAssignment();
	}
	else if (("SERIALNUM" == idstr) || ("SN" == idstr))
	{
		serialnum = ParseStringAssignment();
	}
	else if ("PINS_PER_PORT" == idstr)
	{
		pins_per_port = ParseIntAssignment();
	}
	else if ("PINCONF" == idstr)
	{
		if (!ParsePinConf())
		{
			return false;
		}
	}
	else if (("PWM" == idstr) || ("PWM_OUT" == idstr))
	{
		idx = ParseIndex();
		if (error)  return false;
		if ((idx < 0) || (idx >= UIO_PWM_COUNT))
		{
			error = true;
			errormsg = StringFormat("Invalid PWM index: %i", idx);
			return false;
		}
		i = ParseIntAssignment();
		if (error)  return false;

		pwm_value[idx] = i;
	}
	else if ("PWM_FREQ" == idstr)
	{
		idx = ParseIndex();
		if (error)  return false;
		if ((idx < 0) || (idx >= UIO_PWM_COUNT))
		{
			error = true;
			errormsg = StringFormat("Invalid PWM index: %i", idx);
			return false;
		}
		i = ParseIntAssignment();
		if (error)  return false;

		pwm_freq[idx] = i;
	}
	else if ("DOUT_ALL" == idstr)
	{
		i = ParseIntAssignment();
		if (i & 1)
		{
			dout_value = 0xFFFFFFFF;
		}
		else
		{
			dout_value = 0x00000000;
		}
	}
	else if ("DOUT" == idstr)
	{
		idx = ParseIndex();
		if (error)  return false;
		if ((idx < 0) || (idx >= UIO_DOUT_COUNT))
		{
			error = true;
			errormsg = StringFormat("Invalid DOUT index: %i", idx);
			return false;
		}
		i = ParseIntAssignment();
		if (error)  return false;

		if (i & 1)
		{
			dout_value |= (1 << idx);
		}
		else
		{
			dout_value &= ~(1 << idx);
		}
	}
	else if (("AOUT" == idstr) || ("ANA_OUT" == idstr))
	{
		idx = ParseIndex();
		if (error)  return false;
		if ((idx < 0) || (idx >= UIO_DAC_COUNT))
		{
			error = true;
			errormsg = StringFormat("Invalid AOUT/DAC index: %i", idx);
			return false;
		}
		i = ParseIntAssignment();
		if (error)  return false;

		aout_value[idx] = i;
	}
	else if ("LEDBLP" == idstr)
	{
		idx = ParseIndex();
		if (error)  return false;
		if ((idx < 0) || (idx >= UIO_LEDBLP_COUNT))
		{
			error = true;
			errormsg = StringFormat("Invalid LEDBLP index: %i", idx);
			return false;
		}
		i = ParseIntAssignment();
		if (error)  return false;

		ledblp_value[idx] = i;
	}
	else if ("PWM_FREQ_ALL" == idstr)
	{
		i = ParseIntAssignment();
		for (n = 0; n < UIO_PWM_COUNT; ++n)
		{
			pwm_value[n] = i;
		}
	}
	else if (("AOUT_ALL" == idstr) || ("ANA_OUT_ALL" == idstr))
	{
		i = ParseIntAssignment();
		for (n = 0; n < UIO_DAC_COUNT; ++n)
		{
			aout_value[n] = i;
		}
	}
	else if (("PWM_ALL" == idstr) || ("PWM_OUT_ALL" == idstr))
	{
		i = ParseIntAssignment();
		for (n = 0; n < UIO_PWM_COUNT; ++n)
		{
			pwm_value[n] = i;
		}
	}
	else if ("LEDBLP_ALL" == idstr)
	{
		i = ParseIntAssignment();
		for (n = 0; n < UIO_LEDBLP_COUNT; ++n)
		{
			ledblp_value[n] = i;
		}
	}
	else
	{
  	errormsg = string("Unknown identifier: ") + idstr;
		return false;
	}

	if (error)  return false;

	return true;
}

bool TUioConfig::ParsePinConf()
{
	if ((pins_per_port != 16) && (pins_per_port != 32))
	{
		errormsg = "set pins_per_port before to 16 or 32!";
		return false;
	}

  if (!sp->CheckSymbol("("))
  {
    errormsg = ") is missing";
    return false;
  }
  sp->SkipWhite();

  // Port ID

  sp->CheckSymbol("P"); // skip P
  sp->CheckSymbol("p"); // skip p

  string  portname = "";
  int     portnum = 0;
  int     pinnum = 0;
  if (sp->ReadNumber())
  {
  	// P0.1 format
  	portnum = sp->PrevToInt();
  	if (!sp->CheckSymbol(".") || !sp->CheckSymbol("_"))
  	{
  		errormsg = "portnum - pinnum separator is missing";
  		return false;
  	}
  }
  else
  {
  	// format A12

  	if ((*sp->readptr >= 'A') && (*sp->readptr < 'A' + 16))
  	{
  		portnum = *sp->readptr - 'A';
  		++sp->readptr;
  	}
  	else if ((*sp->readptr >= 'a') && (*sp->readptr < 'a' + 16))
  	{
  		portnum = *sp->readptr - 'a';
  		++sp->readptr;
  	}
  	else
  	{
  		errormsg = "port letter is missing";
  		return false;
  	}
  }

	if (!sp->ReadNumber())
	{
		errormsg = "pin number is missing";
		return false;
	}
	pinnum = sp->PrevToInt();

	unsigned pinid = portnum * pins_per_port + pinnum;
	if (pinid >= UIO_MAX_PINS)
	{
		errormsg = "Invalid pin selection";
		return false;
	}

  sp->SkipWhite();
  if (!sp->CheckSymbol(","))
  {
		errormsg = "\",\" is missing";
		return false;
  }
  sp->SkipWhite();

  // Pintype

  if (!sp->ReadAlphaNum())
  {
		errormsg = "pintype is missing";
		return false;
  }
  string pintypename = sp->PrevStr();

  int pintype = 0;
  if (sp->UCComparePrev("PASSIVE"))
  {
  	pintype = UIO_PINTYPE_PASSIVE;
  }
  else if (sp->UCComparePrev("DIN") || sp->UCComparePrev("DIG_IN"))
  {
  	pintype = UIO_PINTYPE_DIG_IN;
  }
  else if (sp->UCComparePrev("DOUT") || sp->UCComparePrev("DIG_OUT"))
  {
  	pintype = UIO_PINTYPE_DIG_OUT;
  }
  else if (sp->UCComparePrev("ADC") || sp->UCComparePrev("ADC_IN") || sp->UCComparePrev("ANA_IN"))
  {
  	pintype = UIO_PINTYPE_ADC_IN;
  }
  else if (sp->UCComparePrev("DAC") || sp->UCComparePrev("DAC_OUT") || sp->UCComparePrev("ANA_OUT"))
  {
  	pintype = UIO_PINTYPE_DAC_OUT;
  }
  else if (sp->UCComparePrev("PWM") || sp->UCComparePrev("PWM_OUT"))
  {
  	pintype = UIO_PINTYPE_PWM_OUT;
  }
  else if (sp->UCComparePrev("LEDBLP"))
  {
  	pintype = UIO_PINTYPE_LEDBLP;
  }
  else if (sp->UCComparePrev("SPI"))
  {
  	pintype = UIO_PINTYPE_SPI;
  }
  else if (sp->UCComparePrev("I2C"))
  {
  	pintype = UIO_PINTYPE_I2C;
  }
  else if (sp->UCComparePrev("UART"))
  {
  	pintype = UIO_PINTYPE_UART;
  }
  else if (sp->UCComparePrev("CLKOUT"))
  {
  	pintype = UIO_PINTYPE_CLKOUT;
  }
  else
  {
  	errormsg = string("Invalid pintype: \"") + sp->PrevStr() + "\"";
    return false;
  }

  sp->SkipWhite();
  if (!sp->CheckSymbol(","))
  {
		errormsg = "\",\" is missing";
		return false;
  }

  int unitnum = ParseIntValue();
  if (error)  return false;

  // optional pinflags
  int pinflags = 0;
  sp->SkipWhite();
  if (sp->CheckSymbol(","))
  {
  	pinflags = ParseIntValue();
    if (error)  return false;
  }

  sp->SkipWhite();
  if (!sp->CheckSymbol(")"))
  {
    errormsg = ") is missing";
    return false;
  }

  SkipSemiColon();

  // save the configuration
  pinconf[pinid] = ((unsigned(pinflags) << 16) | (unitnum << 8) | pintype);

  return true;
}

void TUioConfig::ResetConfig()
{
	unsigned n;

	pins_per_port = 0;
	deviceid = "";
	diid = "";
	for (n = 0; n < UIO_MAX_PINS; ++n)
	{
		pinconf[n] = 0;
	}
	dout_value = 0;
	for (n = 0; n < UIO_DAC_COUNT; ++n)
	{
		aout_value[n] = 0;
	}
	for (n = 0; n < UIO_PWM_COUNT; ++n)
	{
		pwm_value[n] = 0;
		pwm_freq[n] = 1000;
	}
	for (n = 0; n < UIO_LEDBLP_COUNT; ++n)
	{
		ledblp_value[n] = 0;
	}
}

bool TUioConfig::SaveToFile(const char * afname)
{
	return false;
}

bool TUioConfig::LoadFromDevice()
{
	return false;
}

bool TUioConfig::SaveToDevice()
{
	unsigned n;
	uint16_t err;
	uint16_t rlen;
	uint32_t v;

	if (!CheckDevice())
	{
		return false;
	}

	printf("Entering config mode...\n");
	// set CONFIG mode
	udocomm.WriteU8(0x0180, 0, 0);  // turn off RUN mode, set to CONFIG mode
	printf("  OK.\n");

	printf("Resetting configuration...\n");
	udocomm.WriteU8(0x01FF, 0, 1);  // reset configuration
	printf("  OK.\n");

	printf("Setting device identifications...\n");

	printf("  Device id:      \"%s\"\n", &deviceid[0]);
	udocomm.UdoWrite(0x0181, 0, &deviceid[0], deviceid.size());

	printf("  Manufacturer:   \"%s\"\n", &manufacturer[0]);
	udocomm.UdoWrite(0x0184, 0, &manufacturer[0], manufacturer.size());

	printf("  USB vendor id:  0x%04X\n", usb_vid);
	udocomm.UdoWrite(0x0182, 0, &usb_vid, sizeof(usb_vid));

	printf("  USB product id: 0x%04X\n", usb_pid);
	udocomm.UdoWrite(0x0183, 0, &usb_pid, sizeof(usb_pid));

	printf("  Serial number:  \"%s\"\n", &serialnum[0]);
	udocomm.UdoWrite(0x0185, 0, &serialnum[0], serialnum.size());

	printf("Configuring Pins...\n");
	// confiure pins
	bool cfgerr = false;

	for (n = 0; n < dev_max_pins; ++n)
	{
		if (pinconf[n])
		{
			try
			{
			  udocomm.UdoWrite(0x0200 + n, 0, &pinconf[n], 4);
			}
			catch (EUdoAbort & e)
			{
				string pinname = GetPinName(n);
				printf("  PIN-%s config error: %04X\n", &pinname[0], e.ecode);
				cfgerr = true;
			}
		}
	}

	if (cfgerr)
	{
		return false;
	}
	printf("  OK.\n");

	printf("Setting output defaults...\n");
	try
	{
		udocomm.UdoWrite(0x0300, 0, &dout_value, 4);
	}
	catch (EUdoAbort & e)
	{
		printf("  DOUT error: %04X\n", e.ecode);
		cfgerr = true;
	}

	for (n = 0; n < UIO_DAC_COUNT; ++n)
	{
		try
		{
  		udocomm.UdoWrite(0x0320 + n, 0, &aout_value[n], sizeof(aout_value[0]));
		}
		catch (EUdoAbort & e)
		{
			printf("  DAC-%u error: %04X\n", n, e.ecode);
			cfgerr = true;
		}
	}

	for (n = 0; n < UIO_PWM_COUNT; ++n)
	{
		try
		{
  		udocomm.UdoWrite(0x0340 + n, 0, &pwm_value[n], sizeof(pwm_value[0]));
		}
		catch (EUdoAbort & e)
		{
			printf("  PWM-%u value error: %04X\n", n, e.ecode);
			cfgerr = true;
		}

		try
		{
  		udocomm.UdoWrite(0x0700 + n, 0, &pwm_freq[n], sizeof(pwm_freq[0]));
		}
		catch (EUdoAbort & e)
		{
			printf("  PWM-%u freq error: %04X\n", n, e.ecode);
			cfgerr = true;
		}
	}

	for (n = 0; n < UIO_LEDBLP_COUNT; ++n)
	{
		try
		{
  		udocomm.UdoWrite(0x0360 + n, 0, &ledblp_value[n], sizeof(ledblp_value[0]));
		}
		catch (EUdoAbort & e)
		{
			printf("  LEDBLP-%u error: %04X\n", n, e.ecode);
			cfgerr = true;
		}
	}

	if (cfgerr)
	{
		printf("Configuration error, staying in CONFIG mode.\n");
		return false;
	}
	printf("  OK.\n");

	printf("Entering RUN mode...\n");
	udocomm.WriteU8(0x0180, 0, 1);
	printf("  OK.\n");

	return true;
}

bool TUioConfig::CheckDevice()
{
	// check comm basics
	int r;
	uint16_t err;
	uint16_t rlen;
	uint32_t v;

	printf("Checking device...\n");

	try
	{
		r = udocomm.UdoRead(0x0100, 0, &databuf[0], sizeof(databuf));
	}
	catch (EUdoAbort & e)
	{
		printf("  Error getting UnivIO identifier: %04X\n", e.ecode);
		return false;
	}

	if (    (strcmp((const char *)&databuf[0], "UnivIO-V2") != 0)
			 && (strcmp((const char *)&databuf[0], "UIO-V3") != 0)
			 && (strcmp((const char *)&databuf[0], "UIO-V2") != 0)
		 )
	{
		printf("  Unexpected device type: \"%s\"\n", &databuf[0]);
		return false;
	}

	try
	{
		r = udocomm.UdoRead(0x0101, 0, &databuf[0], sizeof(databuf));
	}
	catch (EUdoAbort & e)
	{
		printf("  Error Reading DIID: %04X\n", e.ecode);
		return false;
	}

	string lfwid = string((const char *)&databuf[0]);
	if (lfwid != diid)
	{
		printf("  DIID \"%s\" is not compatible with the configuration file (\"%s\")\n", &lfwid[0], &diid[0]);
		return false;
	}

	try
	{
		r = udocomm.UdoRead(0x0110, 0, &dev_max_pins, 1);
	}
	catch (EUdoAbort & e)
	{
		printf("  Error reading max pins: %04X\n", e.ecode);
		return false;
	}

	printf("  OK.\n");
	return true;
}

string TUioConfig::GetPinName(uint8_t apinnum)
{
	const char * port_letter = "ABCDEFGHIJKLMNOP";

	unsigned portnum = apinnum / pins_per_port;
	unsigned pinnum = apinnum % pins_per_port;

	if (portnum > 15)  portnum = 15;

	return StringFormat("%c%u", port_letter[portnum], pinnum);
}
