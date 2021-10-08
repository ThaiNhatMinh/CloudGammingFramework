#include "Logger.hh"
#include "Module.hh"

void LastErrorWithCode(DWORD error, const char* func, int line)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    LocalFree(lpMsgBuf);
    LOG << func << ":" << line << " " << (char*)lpMsgBuf << std::endl;
}

void LastError(const char* func, int line)
{
    LastErrorWithCode(GetLastError(), func, line);
}

void LastSocketError(const char* func, int line)
{
    LastErrorWithCode(WSAGetLastError(), func, line);
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
            LASTERROR;
        }
    }
    return hMod;
}