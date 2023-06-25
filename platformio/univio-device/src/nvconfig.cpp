/*
 * nvconfig.cpp
 *
 *  Created on: Oct 30, 2022
 *      Author: vitya
 */

#include "nvconfig.h"
#include "SPIFFS.h"
#include "traces.h"

TNvConfig  g_config;

uint32_t vgboot_checksum(void * pdata, unsigned datalen)
{
  uint32_t *  pu32 = (uint32_t *)pdata;
  uint32_t *  pu32_end = pu32 + (datalen >> 2);
  uint32_t    csum = 0;
  while (pu32 < pu32_end)
  {
    csum += *pu32++;
  }

  uint32_t rbytes = (datalen & 3); // remaining bytes
  if (rbytes)
  {
    uint32_t d = *pu32;
    if       (rbytes == 3)  csum += (d & 0x00FFFFFF);
    else if  (rbytes == 2)  csum += (d & 0x0000FFFF);
    else if  (rbytes == 1)  csum += (d & 0x000000FF);
  }
  return -csum;
}

void TNvConfig::Init()
{
  // set the defaults

  memset(&data, 0xFF, sizeof(data));
  data.signature = CFG_SIGNATURE;

  data.ip_address.Set(192, 168, 2, 20);
  data.net_mask.Set(255, 255, 255, 0);
  data.gw_address.Set(0, 0, 0, 0);
  data.dns.Set(0, 0, 0, 0);
  data.dns2.Set(0, 0, 0, 0);

  data.wifi_ssid[0] = 0;
  data.wifi_password[0] = 0;

  Load();
  UpdateFromConfig();
}

TCfgStb spifl_cfg __attribute__((aligned(8)));  // local buffer for flash loading

void TNvConfig::Load()
{
  TCfgStb * pstb = &spifl_cfg;

  File file = SPIFFS.open("/netconfig.bin", FILE_READ);
  if (!file)
  {
    TRACE("Error reading config file!\r\n");
    return;
  }

  if (file.read((uint8_t *)&spifl_cfg, sizeof(spifl_cfg)))
  {
    if (pstb->signature != CFG_SIGNATURE)
    {
      TRACE("Configuration load error: invalid signature.\r\n");
    }
    else
    {
      if ( 0 != vgboot_checksum(pstb, sizeof(*pstb)) )
      {
        TRACE("Configuration checksum error\r\n");
        return;
      }

      TRACE("Saved configuration loaded successfully.\r\n");

      data = *pstb;
    }
  }
  else
  {
    TRACE("Setup load failed!\r\n");
  }

  file.close();
}

void TNvConfig::Save()
{
  TRACE("Saving setup...\r\n");

  // prepare signature, checksum
  data.signature = CFG_SIGNATURE;
  data.cfg_csum = 0;
  data.cfg_csum = vgboot_checksum(&data, sizeof(data));

  File file = SPIFFS.open("/netconfig.bin", FILE_WRITE);
  if (!file)
  {
    TRACE("Error writing config file!\r\n");
    return;
  }

  if (file.write((const uint8_t *)&data, sizeof(data)))
  {
    TRACE("Setup save completed.\r\n");
  }
  else
  {
    TRACE("Setup save failed!\r\n");
  }

  file.close();
}

void TNvConfig::UpdateFromConfig()
{
#if 0
  ip4_handler.ipaddress = data.ip_address;
  ip4_handler.netmask   = data.net_mask;
  ip4_handler.gwaddress = data.gw_address;
#endif
}
