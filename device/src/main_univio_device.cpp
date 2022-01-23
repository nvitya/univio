/*
 * file:     main_cdc_echo.cpp
 * brief:    USB CDC Echo example for VIHAL
 * created:  2021-11-18
 * authors:  nvitya
 *
 * description:
 *   Simple CDC example, which just sends back the received data
*/

#include "platform.h"
#include "hwpins.h"
#include "hwclk.h"
#include "hwuart.h"
#include "cppinit.h"
#include "clockcnt.h"
#include "hwusbctrl.h"
#include "board_pins.h"
#include "usb_univio.h"
#include "univio_comm.h"
#include "uio_gendev.h"

#include "traces.h"

volatile unsigned hbcounter = 0;

extern "C" __attribute__((noreturn)) void _start(unsigned self_flashing)  // self_flashing = 1: self-flashing required for RAM-loaded applications
{
  // after ram setup and region copy the cpu jumps here, with probably RC oscillator
  mcu_disable_interrupts();

  // Set the interrupt vector table offset, so that the interrupts and exceptions work
  mcu_init_vector_table();


#if defined(MCU_FIXED_SPEED)

  SystemCoreClock = MCU_FIXED_SPEED;

#else
  #if 0
    SystemCoreClock = MCU_INTERNAL_RC_SPEED;
  #else
    if (!hwclk_init(EXTERNAL_XTAL_HZ, MCU_CLOCK_SPEED))  // if the EXTERNAL_XTAL_HZ == 0, then the internal RC oscillator will be used
    {
      while (1)
      {
        // error
      }
    }
  #endif
#endif

  // the cppinit must be done with high clock speed
  // otherwise the device responds the early USB requests too slow.
  // This is the case when the USB data line pull up is permanently connected to VCC, like at the STM32F103 blue pill board

  cppinit();  // run the C/C++ initialization (variable initializations, constructors)

  mcu_enable_fpu();    // enable coprocessor if present
  mcu_enable_icache(); // enable instruction cache if present

  clockcnt_init();

  // Init traces early
  traces_init();
  tracebuf.waitsend = false;  // force to buffered mode (default)
  //tracebuf.waitsend = true;  // better for basic debugging and stepping

  TRACE("\r\n--------------------------------------\r\n");
  TRACE("UNIVIO Device: %s\r\n", BOARD_NAME);
  TRACE("SystemCoreClock: %u\r\n", SystemCoreClock);

  TRACE_FLUSH();

  mcu_enable_interrupts();

  g_uiodev.Init();
  g_uiodev.LoadSetup();

#if USB_ENABLE
  usb_device_init();
#endif

#if UART_CTRL_ENABLE
  if (!g_uartctrl.Init())
  {
    TRACE("Ctrl Uart init error!\r\n");
  }
#endif

  TRACE("\r\nStarting main cycle...\r\n");

  unsigned hbclocks = SystemCoreClock / 2;

  unsigned t0, t1;

  t0 = CLOCKCNT;

  // Infinite loop
  while (1)
  {
    t1 = CLOCKCNT;

    #if USB_ENABLE
      usb_device_run();
    #endif

    #if UART_CTRL_ENABLE
      g_uartctrl.Run();
    #endif

    g_uiodev.Run();

    tracebuf.Run();

    if (t1-t0 > hbclocks)
    {
      ++hbcounter;

      t0 = t1;
    }
  }
}

// ----------------------------------------------------------------------------
