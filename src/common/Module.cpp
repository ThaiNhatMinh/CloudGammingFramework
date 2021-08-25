#include "Logger.hh"
#include "Module.hh"

void LastError()
{
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    LocalFree(lpMsgBuf);
    LOG_ERROR << (char*)lpMsgBuf << std::endl;
}

HMODULE CheckModule(const char* module)
{
    HMODULE hMod = GetModuleHandle(module);
    if (hMod == NULL)
    {
        LOG << module <<  " not yet load" << std::endl;
        hMod = LoadLibrary(module);
        if (hMod == NULL)
        {
            LOG << module << " load failed" << std::endl;
            LastError();
        }
    }
    return hMod;
}