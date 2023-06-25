/*
 * cmdline_base.cpp
 *
 *  Created on: Oct 29, 2022
 *      Author: vitya
 */

#include <cmdline_base.h>

#define TRACE(...)  Serial.printf( __VA_ARGS__ )

void TCmdLineBase::Init()
{
  cmdlen = 0;

  initialized = true;
}

bool TCmdLineBase::InitHw()
{
  return false;
}

bool TCmdLineBase::ParseCmd()
{
  return false;
}

bool TCmdLineBase::ExecCmd()
{

  return ParseCmd();
}

void TCmdLineBase::Run()
{
  if (!initialized)
  {
    return;
  }

  while (Serial.available() > 0)
  {
    uint8_t b = Serial.read();
    //TRACE("%02X\r\n", b);
    if (13 == b)
    {
      cmdbuf[cmdlen] = 0; // zero terminate the command line
      TRACE("\r\n");

      if (cmdlen > 0)
      {
#if 0
        TRACE("cmd = ");
        for (unsigned n = 0; n < cmdlen; ++n)
        {
          TRACE(" %02X", cmdbuf[n]);
        }
        TRACE("\r\n");
#endif

        sp.Init((char *)&cmdbuf[0], cmdlen);

        // execute the command
        if (!ExecCmd())
        {
          TRACE("Unknown command \"%s\"\r\n", &cmdbuf[0]);
        }
      }

      WritePrompt();

      cmdlen = 0;
    }
    else if ((8 == b) || (127 == b)) // backspace
    {
      if (cmdlen > 0)
      {
        TRACE("\b\x1b[K"); // cursor back by one, "ESC [ K" = clear the line from the cursor
        --cmdlen;
      }
    }
    else if ((b >= 32) && (b < 0x7F)) // add to the rx message length
    {
      if (cmdlen < sizeof(cmdbuf))
      {
        cmdbuf[cmdlen] = b;
        ++cmdlen;
      }
      TRACE("%c", b);  // echo back
    }
    else if (27 == b) // escape char !
    {

    }
  }
}

void TCmdLineBase::WritePrompt()
{
  //uart.printf("%s", prompt);
  TRACE("%s", prompt);
}
