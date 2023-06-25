/*
 * cmdline_app.h
 *
 *  Created on: Oct 29, 2022
 *      Author: vitya
 */

#ifndef SRC_CMDLINE_APP_H_
#define SRC_CMDLINE_APP_H_

#include <cmdline_base.h>
#include "strparse.h"
#include "nvconfig.h"

class TCmdLineApp: public TCmdLineBase
{
public:
  virtual bool      InitHw();  // override, in board specific sources
  virtual bool      ParseCmd();

  void              ShowHelp();
  bool              ParseCmdNet();

  void              ShowNetAdapterInfo();
  void              ShowNetInfo(char * idptr, unsigned idlen);

protected:
  char *            idptr = nullptr;
  unsigned          idlen = 0;

  char *            vstr = nullptr;
  unsigned          vstrlen = 0;

  TIp4Addr          ipaddr;
  uint8_t           macaddr[6];

  bool              ParseIpAddr();
  bool              ParseString(char * adst, unsigned adstlen);

};

extern TCmdLineApp g_cmdline;

#endif /* SRC_CMDLINE_APP_H_ */
