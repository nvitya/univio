/*
 * main.cpp
 *
 *  Created on: Nov 21, 2021
 *      Author: vitya
 */

using namespace std;

#include "stdio.h"
#include "stdlib.h"
#include <string>
#include "univio_conn.h"

TUnivioConn  conn;

uint8_t databuf[2048];

void flushstdout()
{
	fflush(stdout);
}

int main(int argc, char * const * argv)
{
  printf("UnivIO Test - v1.0\n");

  string comport = "/dev/ttyACM0";

  if (argc > 1)
  {
  	comport = argv[1];
  }

  if (!conn.Open(&comport[0]))
  {
  	printf("Error opening univio port \"%s\"\n", &comport[0]);
  	exit(1);
  }

	printf("port \"%s\" opened.\n", &comport[0]);

	uint32_t  u32;
	uint16_t  iores;
	iores = conn.ReadUint32(0x0000, &u32);
	iores = conn.ReadUint32(0x0000, &u32);
	printf("ReadUint32(0x0000) result = %04X, data = %08X\n", iores, u32);

#if 0
	iores = conn.ReadUint32(0x8000, &u32);
	printf("ReadUint32(0x8000) result = %04X, data = %08X\n", iores, u32);

	u32 = 0x12345678;
	iores = conn.WriteUint32(0x8000, u32);
	printf("WriteUint32(0x8000) result = %04X, data = %08X\n", iores, u32);

	iores = conn.ReadUint32(0x8000, &u32);
	printf("ReadUint32(0x8000) result = %04X, data = %08X\n", iores, u32);
#endif

	// set the led blink pattern
	iores = conn.WriteUint32(0x1500, 0x00FF0101);

	conn.Close();

	printf("Test finished.\n");
	//flushstdout();

  return 0;
}

