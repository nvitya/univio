/*
 * nvconfig.h
 *
 *  Created on: Oct 30, 2022
 *      Author: vitya
 */

#ifndef SRC_NVCONFIG_H_
#define SRC_NVCONFIG_H_

#include "Arduino.h"

#define CFG_SIGNATURE   0xAA66CF55

typedef struct
{
  union
  {
    uint8_t   u8[4];
    uint32_t  u32;
  };

  inline void Set(int a0, int a1, int a2, int a3)
  {
    u8[0] = a0;
    u8[1] = a1;
    u8[2] = a2;
    u8[3] = a3;
  }
//
} TIp4Addr, * PIp4Addr;

typedef struct
{
  uint32_t          signature;
  uint32_t          cfg_csum;

  TIp4Addr          ip_address;    // IP
  TIp4Addr          net_mask;      // NETMASK
  TIp4Addr          gw_address;    // GW
  TIp4Addr          dns;           // DNS
  TIp4Addr          dns2;          // DNS2
  uint32_t          _reserved[1];

  char              wifi_ssid[64];     // WLSSID
  char              wifi_password[64]; // WLPW

  uint32_t          _tail_pad[256-8-32];  // pad up to 1k
//
} TCfgStb;

class TNvConfig
{
public:
  TCfgStb    data;

  virtual    ~TNvConfig() { }

  void       Init();

  void       Load();
  void       Save();
  void       UpdateFromConfig();
};

extern TNvConfig  g_config;

#endif /* SRC_NVCONFIG_H_ */
