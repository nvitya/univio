/* -----------------------------------------------------------------------------
 * This file is a part of the UNIVIO project: https://github.com/nvitya/univio
 * Copyright (c) 2022 Viktor Nagy, nvitya
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 * --------------------------------------------------------------------------- */
/*
 *  file:     usb_application.h
 *  brief:    USB Device Application Setup and Operation
 *  date:     2023-06-07
 *  authors:  nvitya
*/

#ifndef SRC_USB_APPLICATION_H_
#define SRC_USB_APPLICATION_H_

#include "usbdevice.h"
#include "usbif_cdc.h"
#include "usbfunc_cdc_uart.h"
#include "udo_usb_comm.h"

class TUsbApplication : public TUsbDevice
{
private:
  typedef TUsbDevice super;

public: // mandatory functions

  virtual bool    InitDevice();

};

extern TUsbFuncUdo      usb_func_udo;
extern TUsbApplication  usb_app;

bool usb_app_init();
void usb_app_run();

#endif /* SRC_USB_APPLICATION_H_ */
