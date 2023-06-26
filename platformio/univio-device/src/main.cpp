
#include "Arduino.h"

#include "WiFi.h"
#include "SPIFFS.h"

#include "board_pins.h"
#include "udo_ip_comm.h"
#include "udoslaveapp.h"
#include "cmdline_app.h"
#include "uio_device.h"

#include "traces.h"

unsigned      g_hbcounter = 0;
unsigned      last_hb_time = 0;


void setup()
{
  board_pins_init();
  traces_init();

  delay(500);

  TRACE("\r\n\r\n--------------------------------\r\n");
  TRACE("Initializing file system...\r\n");
  if (SPIFFS.begin(true))  // formatOnFail = true
  {
    TRACE("SPIFFS initialized.\r\n");
  }
  else
  {
    TRACE("Error initializing SPIFFS!\r\n");
  }

  g_uiodev.Init();
  g_uiodev.LoadSetup();

  g_cmdline.ShowNetAdapterInfo();
  g_cmdline.ShowNetInfo(nullptr, 0);

  board_net_init();

  if (!g_udoip_comm.Init())
  {
    TRACE("Error initializing UDO-IP communication !\r\n");
  }

  TRACE("\r\n");

  TRACE("\r\n");
  g_cmdline.Init();
  g_cmdline.WritePrompt();
}

void IRAM_ATTR loop()
{
  unsigned t = micros();

  //vTaskDelayUntil(pxPreviousWakeTime, xTimeIncrement)

  g_udoip_comm.Run();
  g_uiodev.Run();
  g_cmdline.Run();

  taskYIELD();
}
