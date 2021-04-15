#pragma once
#include <Windows.h>

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
