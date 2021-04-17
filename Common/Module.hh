#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>

HMODULE CheckModule(const char* module);
void LastError();

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
