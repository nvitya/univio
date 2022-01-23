/*
 * main.cpp
 *
 *  Created on: Nov 21, 2021
 *      Author: vitya
 */

#include "stdio.h"
#include "stdlib.h"
#include <string>

#include "uioconfigfile.h"
#include "univio_conn.h"

using namespace std;

TUnivioConn conn;

string confname = "";

void print_usage()
{
	printf("usage:\n");
	printf("  uioconf <config_file> [<ttydev>] [config overrides] \n");
}

int main(int argc, char * const * argv)
{
  printf("UnivIO Configurator - v1.0\n");

  //printf("argv[0]=%s\n", argv[0]);

  if (argc < 2)
  {
  	print_usage();
  	exit(1);
  }

 	confname = string(argv[1]);

  printf("Reading configuration \"%s\"...\n", &confname[0]);

  if (!uioconfig.ReadConfigFile(confname))
  {
  	printf("  error: %s\n", &uioconfig.errormsg[0]);
  	exit(1);
  }

	printf("  OK.\n");

	if (argc < 2)
	{
		return 0; // that was it
	}

	if (argc > 2)
	{
		printf("Reading Command line arguments...\n");
		if (!uioconfig.ParseCommandLine(argc, argv, 3))
		{
	  	printf("  error: %s\n", &uioconfig.errormsg[0]);
	  	exit(1);
		}
		printf("  OK.\n");
	}

	if (!conn.Open(argv[2]))
	{
		exit(1);
	}

	bool err = !uioconfig.SaveToDevice(&conn);

	conn.Close();

	if (err)
	{
	  printf("Errors detected, device is not configured properly!\n");
	  exit(1);
	}
	else
	{
		printf("Device configured and activated.\n");
	}

  return 0;
}

