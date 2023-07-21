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
#include "udo_comm.h"
#include "commh_udosl.h"
#include "commh_udoip.h"

using namespace std;

string confname = "";

void print_usage()
{
	printf("usage:\n");
	printf("  uioconf <config_file> [<ttydev>] [config overrides] \n");
}

int main(int argc, char * const * argv)
{
  printf("UnivIO Configurator - v2.0\n");

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

	if (argc < 3)
	{
		printf("No device address privided, exiting.\n");
		return 0; // that was it
	}

	if (argc > 3)
	{
		printf("Reading Command line arguments...\n");
		if (!uioconfig.ParseCommandLine(argc, argv, 3))
		{
	  	printf("  error: %s\n", &uioconfig.errormsg[0]);
	  	exit(1);
		}
		printf("  OK.\n");
	}

	string connstr = argv[2];
	if (connstr.find('.') != string::npos)  // IP address contains a dot
	{
		udoip_commh.ipaddrstr = connstr;
		udocomm.SetHandler(&udoip_commh);
	}
	else // serial connection
	{
		udosl_commh.devstr = string(argv[2]);
		udocomm.SetHandler(&udosl_commh);
	}

	printf("Connecting to device at \"%s\"...\n", udocomm.commh->ConnString().c_str());
	try
	{
		udocomm.Open();
	}
	catch (exception & e)
	{
		printf("Exception: %s\n", e.what());
		exit(1);
	}

	bool cfgerr = true;
	try
	{
  	cfgerr = !uioconfig.SaveToDevice();
	}
	catch (exception & e)
	{
		printf("Exception: %s\n", e.what());
	}

	udocomm.Close();

	if (cfgerr)
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

