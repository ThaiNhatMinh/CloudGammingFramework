#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include "Logger.hh"

HMODULE CheckModule(const char* module);
void LastError(const char* func, int line);
void LastSocketError(const char* func, int line);
void LastErrorWithCode(DWORD error, const char* func, int line);

#define LASTERROR LastError(__FUNCTION__, __LINE__);
#define LASTSOCKETERROR LastSocketError(__FUNCTION__, __LINE__);

template<class T>
T GetProcAddress(HMODULE module, const char* funcName)
{
    T res = (T)::GetProcAddress(module, funcName);
    if (res == NULL)
    {
        LASTERROR;
    }
    return res;
}
