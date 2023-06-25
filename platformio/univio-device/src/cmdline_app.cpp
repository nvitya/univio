/*
 * cmdline_app.cpp
 *
 *  Created on: Oct 29, 2022
 *      Author: vitya
 */

#include "cmdline_app.h"
#include "WiFi.h"
#include "traces.h"

TCmdLineApp g_cmdline;

bool TCmdLineApp::InitHw()
{
  return true;
}

bool TCmdLineApp::ParseCmd() // sp is already prepared
{
  sp.SkipSpaces();
  if (!sp.ReadAlphaNum())
  {
    TRACE("The command must begin with an identifier!\r\n");
    return true;
  }

  if (sp.UCComparePrev("H") or sp.UCComparePrev("HELP"))
  {
    ShowHelp();
    return true;
  }

  if (sp.UCComparePrev("SAVE"))
  {
    g_config.Save();
    return true;
  }

  if (sp.UCComparePrev("NET"))
  {
    ParseCmdNet();
    return true;
  }

  if (sp.UCComparePrev("REBOOT") || sp.UCComparePrev("RESTART"))
  {
    ESP.restart();
    return true;
  }

  return false;
}

bool TCmdLineApp::ParseCmdNet()
{
  sp.SkipSpaces();
  if (!sp.ReadAlphaNum())
  {
    ShowNetInfo(nullptr, 0);
    return true;
  }

  idptr = sp.prevptr;
  idlen = sp.prevlen;
  char * ridp = idptr;

  sp.SkipSpaces();

  if (sp.readptr >= sp.bufend)
  {
    // no value, just print the info
    ShowNetInfo(idptr, idlen);
    return true;
  }

  // there is a value

  if (PCharUCCompare(&ridp, idlen, "IP"))
  {
    if (!ParseIpAddr())
    {
      return true;
    }
    g_config.data.ip_address = ipaddr;
    ShowNetInfo(idptr, idlen);
    return true;
  }

  if (PCharUCCompare(&ridp, idlen, "NETMASK"))
  {
    if (!ParseIpAddr())
    {
      return true;
    }
    g_config.data.net_mask = ipaddr;
    ShowNetInfo(idptr, idlen);
    return true;
  }

  if (PCharUCCompare(&ridp, idlen, "GW"))
  {
    if (!ParseIpAddr())
    {
      return true;
    }
    g_config.data.gw_address = ipaddr;
    ShowNetInfo(idptr, idlen);
    return true;
  }

  if (PCharUCCompare(&ridp, idlen, "DNS"))
  {
    if (!ParseIpAddr())
    {
      return true;
    }

    g_config.data.dns = ipaddr;
    ShowNetInfo(idptr, idlen);
    return true;
  }

  if (PCharUCCompare(&ridp, idlen, "WLSSID"))
  {
    if (!ParseString(g_config.data.wifi_ssid, sizeof(g_config.data.wifi_ssid)))
    {
      return true;
    }

    ShowNetInfo(idptr, idlen);
    return true;
  }

  if (PCharUCCompare(&ridp, idlen, "WLPW"))
  {
    if (!ParseString(g_config.data.wifi_password, sizeof(g_config.data.wifi_password)))
    {
      return true;
    }

    ShowNetInfo(idptr, idlen);
    return true;
  }


  TRACE("Unknown NET section.\r\n");

  return true;
}

void TCmdLineApp::ShowNetAdapterInfo()
{
  TRACE("ESP Mac Address: %s\r\n", WiFi.macAddress().c_str());
}

void TCmdLineApp::ShowNetInfo(char * idptr, unsigned idlen)
{
  TCfgStb * pcfg = &g_config.data;

  TIp4Addr ip;

  if (!idptr || PCharUCCompare(&idptr, idlen, "IP"))
  {
    ip = pcfg->ip_address;
    TRACE("  IP       = %d.%d.%d.%d\r\n", ip.u8[0], ip.u8[1], ip.u8[2], ip.u8[3]);
  }
  if (!idptr || PCharUCCompare(&idptr, idlen, "NETMASK"))
  {
    ip = pcfg->net_mask;
    TRACE("  NETMASK  = %d.%d.%d.%d\r\n", ip.u8[0], ip.u8[1], ip.u8[2], ip.u8[3]);
  }
  if (!idptr || PCharUCCompare(&idptr, idlen, "GW"))
  {
    ip = pcfg->gw_address;
    TRACE("  GW       = %d.%d.%d.%d\r\n", ip.u8[0], ip.u8[1], ip.u8[2], ip.u8[3]);
  }
  if (!idptr || PCharUCCompare(&idptr, idlen, "DNS"))
  {
    ip = pcfg->dns;
    TRACE("  DNS      = %d.%d.%d.%d\r\n", ip.u8[0], ip.u8[1], ip.u8[2], ip.u8[3]);
  }

  if (!idptr || PCharUCCompare(&idptr, idlen, "WLSSID"))
  {
    TRACE("  WLSSID   = \"%s\"\r\n", pcfg->wifi_ssid);
  }

  if (!idptr || PCharUCCompare(&idptr, idlen, "WLPW"))
  {
    TRACE("  WLPW     = ***\r\n");
  }
}

bool TCmdLineApp::ParseString(char * adst, unsigned adstlen)
{
  sp.SkipSpaces();
  if (*sp.readptr == '\"')
  {
    if (!sp.ReadQuotedString())
    {
      TRACE("Invalid String.\r\n");
      return false;
    }
  }
  else
  {
    sp.ReadTo(" \r\n\0");
    if (sp.prevlen <= 0)
    {
      TRACE("Invalid String.\r\n");
      return false;
    }
  }

  vstr = sp.prevptr;
  vstrlen = sp.prevlen;

  if (adst)
  {
    if (adstlen <= sp.prevlen)  sp.prevlen = adstlen - 1;
    memcpy(adst, sp.prevptr, sp.prevlen);
    adst[sp.prevlen] = 0;
  }

  return true;
}

bool TCmdLineApp::ParseIpAddr()
{
  for (int n = 0; n < 4; ++n)
  {
    sp.SkipSpaces();
    if (n > 0)
    {
      if (!sp.CheckSymbol("."))
      {
        TRACE("Invalid IP\r\n");
        return false;
      }
      sp.SkipSpaces();
    }

    if (!sp.ReadDecimalNumbers())
    {
      TRACE("Invalid IP\r\n");
      return false;
    }
    ipaddr.u8[n] = sp.PrevToInt();
  }

  return true;
}

void TCmdLineApp::ShowHelp()
{
  TRACE("Available commands:\r\n");
  TRACE("  NET [var] [value]  (displays, sets network parameters)\r\n");
  TRACE("  SAVE  (saves configuration permanently)\r\n");
  TRACE("  REBOOT  (restarts the device)\r\n");
}
