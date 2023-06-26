
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

  digitalWrite(PIN_LED, 1);
  delay(500);

  TRACE("\n\n--------------------------------\n");
  TRACE("Initializing file system...\r\n");
  if (SPIFFS.begin(true))  // formatOnFail = true
  {
    TRACE("SPIFFS initialized.\n");
  }
  else
  {
    TRACE("Error initializing SPIFFS!\n");
  }

  g_uiodev.Init();
  g_uiodev.LoadSetup();

  g_cmdline.ShowNetAdapterInfo();
  g_cmdline.ShowNetInfo(nullptr, 0);

  board_net_init();

  if (!g_udoip_comm.Init())
  {
    TRACE("Error initializing UDO-IP communication !\n");
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

  if (t - last_hb_time > 1000*1000)
  {
    ++g_hbcounter;
    digitalWrite(PIN_LED, g_hbcounter & 1);

    last_hb_time = t;
  }

  taskYIELD();
}
