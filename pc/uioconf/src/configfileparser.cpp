#include "string.h"
#include <configfileparser.h>
#include "general.h"

bool TConfigFileParser::ReadConfigFile(string fname)
{
  sp = &strparser;

  error = false;

  filename = fname;

  if (filedata)
  {
    free(filedata);
    filedata = NULL;
  }

  filedata = read_file_contents(&filename[0], &filedatasize);
  if (!filedata)
  {
    error = true;
    errormsg = string("File \"") + fname + string("\" can not be read.");
  }

  if (!error)
  {
  	ResetConfig();

    sp->Init(filedata, filedatasize);
    errormsg = "";

    error = !ParseConfig();
  }

  return !error;
}

bool TConfigFileParser::ParseConfig()
{
  sp->SkipWhite();

  while (sp->readptr < sp->bufend)
  {
    // read section

    if (!sp->ReadAlphaNum())
    {
    	errormsg = string("identifier missing, current char: ") + *sp->readptr;
    	return false;
    }

    string idstr(sp->prevptr, sp->prevlen);
    StringToUpper(idstr);

    sp->SkipWhite();

    if (!ParseConfigLine(idstr))
    {
      return false;
    }

    sp->SkipWhite();

  } // while reading section

  return true;
}

bool TConfigFileParser::ParseCommandLine(int argc, char * const * argv, int argstart)
{
	int i = argstart;
	while (i < argc)
	{
    sp->Init(argv[i], strlen(argv[i]));
    errormsg = "";

		if (!ParseConfig())
		{
			return false;
		}

		++i;
	}

	return true;
}

bool TConfigFileParser::ParseConfigLine(string idstr)
{
  return false;
}

int TConfigFileParser::ParseIndex()
{
  int result = 0;

  sp->SkipWhite();
  if (!sp->CheckSymbol("["))
  {
    error = true;
    errormsg = "[ is missing";
    return 0;
  }

  sp->SkipWhite();
  result = ParseIntValue();

  sp->SkipWhite();
  if (!sp->CheckSymbol("]"))
  {
    error = true;
    errormsg = "] is missing";
    return 0;
  }

  return result;
}

int TConfigFileParser::ParseIntAssignment()
{
  int result = 0;
  sp->SkipWhite();
  if (!sp->CheckSymbol("="))
  {
    error = true;
    errormsg = "= is missing";
    return 0;
  }

  result = ParseIntValue();
  if (!error)
  {
    SkipSemiColon();
  }

  return result;
}

int TConfigFileParser::ParseIntValue()
{
  int result = 0;

  sp->SkipWhite();
  bool negative = sp->CheckSymbol("-");
  sp->SkipWhite();

  if (sp->CheckSymbol("0x"))
  {
    // hex value
    if (!sp->ReadAlphaNum())
    {
      error = true;
      errormsg = "Error reading hex value";
      return result;
    }
    result = sp->PrevHexToInt();
  }
  else
  {
    if (!sp->ReadAlphaNum())
    {
      error = true;
      errormsg = "Error reading int value";
      return result;
    }
    result = sp->PrevToInt();
  }
  if (negative)  result = -result;

  return result;
}

unsigned TConfigFileParser::ParseUintValue()
{
  unsigned result = 0;

  if (sp->CheckSymbol("0x"))
  {
    // hex value
    if (!sp->ReadAlphaNum())
    {
      error = true;
      errormsg = "Error reading hex value";
      return result;
    }
    result = sp->PrevHexToInt();
  }
  else
  {
    if (!sp->ReadAlphaNum())
    {
      error = true;
      errormsg = "Error reading uint value";
      return result;
    }
    result = sp->PrevToUint();
  }

  return result;
}

string TConfigFileParser::ParseStringAssignment()
{
  string result = "";
  sp->SkipWhite();
  if (!sp->CheckSymbol("="))
  {
    error = true;
    errormsg = "= is missing";
    return result;
  }

  result = ParseStringValue();
  if (!error)
  {
    SkipSemiColon();
  }

  return result;
}

string TConfigFileParser::ParseStringValue()
{
  string result = "";

  sp->SkipWhite();

  if (sp->CheckSymbol("\""))
  {
  	if (sp->ReadToChar('\"'))
  	{
  		result.assign(sp->prevptr, sp->prevlen);
  		sp->CheckSymbol("\"");
  	}
  	else
  	{
      error = true;
      errormsg = "closing \" for the string was not found.";
      return result;
    }
  }
  else
  {
    if (!sp->ReadAlphaNum())
    {
      error = true;
      errormsg = "Error reading string value";
      return result;
    }
    result.assign(sp->prevptr, sp->prevlen);
  }

  return result;
}


bool TConfigFileParser::ParseBoolAssignment()
{
  bool result = false;
  sp->SkipWhite();
  if (!sp->CheckSymbol("="))
  {
    error = true;
    errormsg = "= is missing";
    return 0;
  }

  sp->SkipWhite();

  result = ParseBoolValue();
  if (!error)
  {
    SkipSemiColon();
  }

  return result;
}

bool TConfigFileParser::ParseBoolValue()
{
	string vstr;
  bool result = false;

	if (!sp->ReadAlphaNum())
	{
		error = true;
		errormsg = "Error reading bool value";
		return result;
	}
	vstr.assign(sp->prevptr, sp->prevlen);
	StringToUpper(vstr);

	if (("1" == vstr) || ("TRUE" == vstr) || ("T" == vstr) || ("Y" == vstr) || ("YES" == vstr))
	{
		result = true;
	}

  return result;
}


void TConfigFileParser::SkipSemiColon()
{
  sp->SkipWhite();
  if (sp->CheckSymbol(";"))
  {
    sp->SkipWhite();
  }
}
