#pragma once
#include <string>
#include "Module.hh"
#include "common/AutoClose.hh"

class FileMapping
{
private:
    typedef AutoClose<HANDLE, CloseHandle> AutoCloseHandle;
    typedef AutoClose<LPCVOID, UnmapViewOfFile> AutoCloseMapView;

    AutoCloseHandle m_mappingHandle;
    AutoCloseMapView m_address;
    std::size_t m_numByte;
public:
    FileMapping() = default;
    bool Create(const std::string& fileName, std::size_t numByte);
    bool Open(const std::string& fileName, std::size_t numByte);
    void Release();
    int Write(const char* buffer, std::size_t length);
    std::string Read(std::size_t numByte);
};
