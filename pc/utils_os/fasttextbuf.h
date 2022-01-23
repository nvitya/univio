#ifndef TFASTTEXTBUF_H
#define TFASTTEXTBUF_H

#define MAX_FTB_SIZE  4*1024*1024   // Max buffer size: 4M

#include <string>

using namespace std;

class TFastTextBuf
{
  public:
    unsigned bufsize;

    char * pbufstart;
    char * pbufend;
    char * pnext;

    TFastTextBuf(unsigned astartsize = 65536);
    ~TFastTextBuf();

    unsigned GetLen();
    void Clear();
    void AddChar(char c);
    void AddBuf(void * buf, unsigned len);
    void AddTB(TFastTextBuf * tb);
    void AddStr(const char * str);
    void AddJsStr(const char * str, unsigned maxlen);
    void AddJsStr(string str);
    void AddInt(int i);
    void AddUInt(unsigned i);
    void AddULL(unsigned long long i);
    void AddHex(unsigned i, char len);

    void AddIW(const char * leftstr, int i, const char * rightstr);
    void AddStrW(const char * leftstr, const char * str, const char * rightstr);

  private:
    void ResizeBuf(unsigned minsize);
};

typedef TFastTextBuf * PFastTextBuf;

#endif // TFASTTEXTBUF_H
