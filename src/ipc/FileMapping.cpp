#include "FileMapping.hh"
#include "Logger.hh"

bool FileMapping::Create(const std::string &fileName, std::size_t numByte)
{
    AutoCloseHandle fileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, numByte, fileName.c_str());
    if (fileMap == NULL)
    {
        LOG_ERROR << "Open file mapping failed: " << std::endl;
        LastError();
        return false;
    }

    AutoCloseMapView address = MapViewOfFile(fileMap.get(), FILE_MAP_ALL_ACCESS, 0, 0, numByte);
    if (address == NULL)
    {
        LOG_ERROR << "Map view of file failed: " << std::endl;
        LastError();
        return false;
    }

    m_mappingHandle = std::move(fileMap);
    m_address = std::move(address);
    return true;
}

bool FileMapping::Open(const std::string &fileName, std::size_t numByte)
{
    AutoCloseHandle hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS, // read/write access
        FALSE,               // do not inherit the name
        fileName.c_str());   // name of mapping object
    if (hMapFile == NULL)
    {
        LOG_ERROR << "OpenFileMapping failed: " << std::endl;
        LastError();
        return false;
    }
    AutoCloseMapView address = MapViewOfFile(hMapFile.get(), FILE_MAP_ALL_ACCESS, 0, 0, numByte);
    if (address == NULL)
    {
        LOG_ERROR << "MapViewOfFile failed: " << std::endl;
        LastError();
        return false;
    }

    m_mappingHandle = std::move(hMapFile);
    m_address = std::move(address);
    return true;
}

void FileMapping::Release()
{
    m_address.Release();
    m_mappingHandle.Release();
    m_numByte = 0;
}

int FileMapping::Write(const char* buffer, std::size_t length)
{
    if (length > m_numByte)
    {
        length = m_numByte;
    }
    std::memcpy(const_cast<void*>(m_address.get()), buffer, length);
    return true;
}

std::string FileMapping::Read(std::size_t length)
{
    if (length > m_numByte)
    {
        length = m_numByte;
    }
    
    std::string result(length, '\0');
    std::memcpy(const_cast<char*>(result.data()), m_address.get(), length);
    return result;
}
