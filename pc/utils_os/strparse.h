// StrParse.h

#ifndef __STRPARSE_H_
#define __STRPARSE_H_

#include <string>

using namespace std;

class TStrParseObj
{
  public:
    char * bufstart;
    char * bufend;

    char * readptr;    // current parsing position

    char * prevptr;    // usually signs token start
    int prevlen;       // usually signs token length

    char commentmarker = '#';

    TStrParseObj();
    TStrParseObj(char * astr, int buflen = 0);

    void      Init(char * astr, int buflen = 0);

    void      SkipSpaces(bool askiplineend = true);
    void      SkipWhite();

    bool      ReadLine();                 // sets prevptr, prevlen
    bool      ReadTo(const char * checkchars);  // sets prevptr, prevlen
    bool      ReadToChar(char achar);     // sets prevptr, prevlen
    bool      ReadAlphaNum();             // sets prevptr, prevlen
    bool      ReadNumber();               // sets prevptr, prevlen
    bool      ReadQuotedString();
    bool      CheckSymbol(const char * checkstring);
    bool      SearchPattern(const char * checkchars);  // sets prevptr, prevlen

    string    PrevStr();
    bool      UCComparePrev(const char * checkstring);
    int       PrevToInt();
    unsigned  PrevToUint();
    unsigned  PrevHexToInt();


    int       GetLineNum(const char * pos);
    int       GetLineNum();
};

typedef TStrParseObj *  PStrParseObj;

bool PCharUCCompare(char * * ReadPtr, int len, const char * checkstring);
int PCharToInt(char * ReadPtr, int len);
int PCharToUint(char * ReadPtr, int len);
unsigned PCharHexToInt(char * ReadPtr, int len);

#endif
