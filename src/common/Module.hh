#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include "Logger.hh"

HMODULE CheckModule(const char* module);
void LastError();
void LastSocketError();
void LastErrorWithCode(DWORD error);

#define LASTSOCKETERROR {LOG_ERROR << __LINE__ << ": "; LastSocketError();}

template<class T>
T GetProcAddress(HMODULE module, const char* funcName)
{
    T res = (T)::GetProcAddress(module, funcName);
    if (res == NULL)
    {
        LastError();
    }
    return res;
}
