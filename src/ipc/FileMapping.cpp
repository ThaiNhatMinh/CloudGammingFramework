#include "FileMapping.hh"
#include "common/Module.hh"
#include "common/Logger.hh"

bool FileMapping::Create(const std::string &fileName, std::size_t numByte)
{
    if (m_isOpenCreate)
    {
        LOG_ERROR << "FileMapping already open at " << m_fileName << std::endl;
        LOG_ERROR << "Call FileMapping::Release before create" << std::endl;
        return false;
    }

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
    m_isOpenCreate = true;
    m_fileName = fileName;
    m_numByte = numByte;
    return true;
}

bool FileMapping::Open(const std::string &fileName, std::size_t numByte)
{
    if (m_isOpenCreate)
    {
        LOG_ERROR << "FileMapping already open at " << m_fileName << std::endl;
        LOG_ERROR << "Call FileMapping::Release before create" << std::endl;
        return false;
    }

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
    m_isOpenCreate = true;
    m_fileName = fileName;
    m_numByte = numByte;
    return true;
}

void FileMapping::Release()
{
    m_address.Release();
    m_mappingHandle.Release();
    m_numByte = 0;
    m_isOpenCreate = false;
}

bool FileMapping::Write(const void* buffer, std::size_t length)
{
    if (length > m_numByte)
    {
        length = m_numByte;
    }
    std::memcpy(const_cast<void*>(m_address.get()), buffer, length);
    return true;
}

std::string FileMapping::Read(std::size_t length, int offset)
{
    if (length + offset > m_numByte)
    {
        LOG_ERROR << "length + offset " << (length + offset) << " greater than " << m_numByte << std::endl;
        return "";
    }
    
    std::string result(length, '\0');
    const char * address = static_cast<const char*>(m_address.get()) + offset;
    std::memcpy(const_cast<char*>(result.data()), address, length);
    return result;
}
