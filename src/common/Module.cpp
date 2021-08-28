#include "Logger.hh"
#include "Module.hh"

void LastErrorWithCode(DWORD error)
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
    LOG_ERROR << (char*)lpMsgBuf << std::endl;
}

void LastError()
{
    LastErrorWithCode(GetLastError());
}

void LastSocketError()
{
    LastErrorWithCode(WSAGetLastError());
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