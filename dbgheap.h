#ifdef CRT_DLL
#define CRT_EXPORT __declspec(dllexport)
#else
#define CRT_EXPORT
#endif

typedef unsigned int    dword;
typedef unsigned char   byte;
typedef unsigned        Bool;
typedef unsigned long   Hdl; // long for OS/2 toolkit compatibility
typedef Hdl             Hev;
typedef Hdl             Hmtx;
typedef Hdl             Tid;

void CRT_EXPORT Sleep(dword dMsec);

#if defined(NEED_SNPRINTF)
int CRT_EXPORT vsnprintf(char *str, dword sz, const char *format, va_list args);
#else
#include <stdio.h>
#endif

class CRT_EXPORT SysMutex
{
    Hmtx hMtx;

public:
    SysMutex();
    virtual ~SysMutex();

    dword Enter(dword timeout = -1);
    void Leave();
};

class CRT_EXPORT RamMutex
{
    volatile byte m_busy;

public:
    RamMutex(): m_busy(0) {}
    virtual ~RamMutex() {}

    void Enter();
    void Leave();
};


