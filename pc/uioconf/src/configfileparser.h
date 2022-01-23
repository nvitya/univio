// configfileparser.h

#ifndef CONFIGFILEPARSER_H_
#define CONFIGFILEPARSER_H_

#include <string>
#include "strparse.h"

class TConfigFileParser
{
public:
  string        filename = "";

  char *        filedata = NULL;
  unsigned      filedatasize = 0;

  bool          error = false;
  string        errormsg = "";

public:
  virtual       ~TConfigFileParser() { }

  bool          ReadConfigFile(string fname);
  bool          ParseConfig();

  bool          ParseCommandLine(int argc, char * const * argv, int argstart);

public:
  virtual void  ResetConfig() { }
  virtual bool  ParseConfigLine(string idstr);

public: // utility

  void          SkipSemiColon();

  int           ParseIndex();
  int           ParseIntAssignment();
  int           ParseIntValue();
  unsigned      ParseUintValue();
  string        ParseStringAssignment();
  string        ParseStringValue();
  bool          ParseBoolAssignment();
  bool          ParseBoolValue();

public:
  TStrParseObj  strparser;
  PStrParseObj  sp;
};


#endif /* CONFIGFILEPARSER_H_ */
