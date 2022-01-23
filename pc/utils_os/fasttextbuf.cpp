#include <stdexcept>

using namespace std;

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "fasttextbuf.h"

const char hextableuc[] = "0123456789ABCDEF";

TFastTextBuf::TFastTextBuf(unsigned astartsize)
{
  bufsize = astartsize;

  pbufstart = (char *)malloc(bufsize);
  pbufend = pbufstart + bufsize;

  pnext = pbufstart;
}

TFastTextBuf::~TFastTextBuf()
{
  free(pbufstart);
}

void TFastTextBuf::ResizeBuf(unsigned minsize)
{
  unsigned newsize = (minsize + 16383) & (0xFFFFFFFF - 16383);
  if (newsize <= bufsize)
  {
    return;
  }

  if (newsize > MAX_FTB_SIZE)
  {
    throw logic_error("TFastTextBuf::ResizeBuf: maximum buffer size reached.");
  }

  char * newbuf = (char *) realloc(pbufstart, newsize);

  bufsize   = newsize;
  pbufend   = newbuf + bufsize;
  pnext     = newbuf + (pnext - pbufstart);
  pbufstart = newbuf;

  //printf("TB resized to %u\n", bufsize);
}

unsigned TFastTextBuf::GetLen()
{
  return pnext - pbufstart;
}

void TFastTextBuf::Clear()
{
  pnext = pbufstart;
}

void TFastTextBuf::AddChar(char c)
{
  if (pnext >= pbufend)
  {
    ResizeBuf(bufsize + 1);
  }

  *pnext = c;
  ++pnext;
}

void TFastTextBuf::AddBuf(void * buf, unsigned len)
{
  if (pnext + len >= pbufend)
  {
    ResizeBuf(bufsize + len);
  }

  memcpy(pnext, buf, len);  // uses wider copy inside, much faster for larger blocks
  pnext += len;
}

void TFastTextBuf::AddTB(TFastTextBuf * tb)
{
  AddBuf(tb->pnext, tb->GetLen());
}

void TFastTextBuf::AddStr(const char * str)
{
  char * c = (char *)str;

repeat_copy:
  while ((pnext < pbufend) && (*c != 0))
  {
    *pnext = *c;
    ++c;
    ++pnext;
  }

  if (*c != 0)
  {
    ResizeBuf(bufsize + 16384);
    goto repeat_copy;
  }
}

void TFastTextBuf::AddJsStr(const char * str, unsigned maxlen)  // adds quotes + escapes the string for network transfer
{
  AddChar('\"');

  if (maxlen == 0)  maxlen = 16*1024;

  unsigned len = 0;
  char * c = (char *)str;

  while ((*c != 0) && (len < maxlen))
  {
    if (pbufend - pnext < 8)
    {
      ResizeBuf(bufsize + 16384);
    }

    if      (*c == '\n')  { *pnext = '\\'; ++pnext;  *pnext = 'n'; ++pnext; }
    else if (*c == '\r')  { *pnext = '\\'; ++pnext;  *pnext = 'r'; ++pnext; }
    else if (*c == '\"')  { *pnext = '\\'; ++pnext;  *pnext = '\"'; ++pnext; }
    else if (*c == '\t')  { *pnext = '\\'; ++pnext;  *pnext = 't'; ++pnext; }
    else if (*c == '\\')  { *pnext = '\\'; ++pnext;  *pnext = '\\'; ++pnext; }
    else
    {
      *pnext = *c;
      ++pnext;
    }

    ++c;
    ++len;
  }

  AddChar('\"');
}

void TFastTextBuf::AddJsStr(string str)
{
  AddJsStr(&str[0], str.length());
}


void TFastTextBuf::AddInt(int i)
{
  if (i < 0)
  {
    AddChar('-');
    AddUInt(-i);
  }
  else
  {
    AddUInt(i);
  }
}

void TFastTextBuf::AddUInt(unsigned i)
{
  char tmp[16];
  char * tmpend = &tmp[16];
  char * c = tmpend;

  unsigned v = i;
  unsigned m;

  do
  {
    m = v % 10;
    v = v / 10;

    --c;
    *c = hextableuc[m];

  } while (v > 0);

  AddBuf(c, tmpend - c);
}

void TFastTextBuf::AddULL(unsigned long long i)
{
  char tmp[32];
  char * tmpend = &tmp[32];
  char * c = tmpend;

  unsigned long long v = i;
  unsigned m;

  do
  {
    m = v % 10;
    v = v / 10;

    --c;
    *c = hextableuc[m];

  } while (v > 0);

  AddBuf(c, tmpend - c);
}

void TFastTextBuf::AddHex(unsigned i, char len)
{
  for (int n = len-1; n >= 0; --n)
  {
    AddChar(hextableuc[(i >> (n * 4)) & 0x0F]);
  }
}

void TFastTextBuf::AddIW(const char * leftstr, int i, const char * rightstr)
{
  AddStr(leftstr);
  AddInt(i);
  AddStr(rightstr);
}

void TFastTextBuf::AddStrW(const char * leftstr, const char * str, const char * rightstr)
{
  AddStr(leftstr);
  AddStr(str);
  AddStr(rightstr);
}
