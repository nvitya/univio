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
 *  file:     usb_application.cpp
 *  brief:    USB Device Application Setup and Operation
 *  date:     2023-06-07
 *  authors:  nvitya
*/

#include "string.h"
#include "udo_usb_comm.h"
#include "usb_application.h"
#include "uio_device.h"
#include "traces.h"

TUsbFuncUdo      usb_func_udo;
TUsbFuncCdcUart  usb_func_uart;
TUsbApplication  usb_app;

#define UIO_DEFAULT_VENDOR_ID   0xDEAD
#define UIO_DEFAULT_PRODUCT_ID  0xBEEF

bool TUsbApplication::InitDevice()
{
  devdesc.vendor_id = g_uiodev.cfg.usb_vendor_id;
  if (0 == devdesc.vendor_id)
  {
    devdesc.vendor_id = UIO_DEFAULT_VENDOR_ID;
  }

  devdesc.product_id = g_uiodev.cfg.usb_product_id;
  if (0 == devdesc.product_id)
  {
    devdesc.product_id = UIO_DEFAULT_PRODUCT_ID;
  }

  devdesc.device_class = 0x02; // Communications and CDC Control
  manufacturer_name = &g_uiodev.cfg.manufacturer[0];
  device_name = &g_uiodev.cfg.device_id[0];
  device_serial_number = &g_uiodev.cfg.serial_number[0];
  if (strlen(manufacturer_name) == 0)
  {
    manufacturer_name = "UnivIO";
  }
  if (strlen(device_name) == 0)
  {
    device_name = UIO_HW_ID;
  }
  if (strlen(device_serial_number) == 0)
  {
    device_serial_number = "1";
  }


  manufacturer_name = "github.com/nvitya/udo";
  device_name = "UDO-USB Device";
  device_serial_number = "1";

  AddFunction(&usb_func_udo);

  if (g_uiodev.uart_active)
  {
    // Add the bridged real USB to UART
    TRACE("Activating USB-UART\r\n");

    usb_func_uart.AssignUart(&g_uart);
    AddFunction(&usb_func_uart);
  }


  return true;
}

bool usb_app_init()
{
  TRACE("Initializing UnivIO USB Device\r\n");

  if (!usb_app.Init()) // calls InitDevice first which sets up the device
  {
    TRACE("Error initializing USB device!\r\n");
    return false;
  }

  return true;
}

void usb_app_run()
{
  usb_app.HandleIrq();
  usb_app.Run();
}
