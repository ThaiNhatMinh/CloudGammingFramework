#pragma once
#include <string>
#include "Handle.hh"

class FileMapping
{
private:

    AutoCloseHandle m_mappingHandle;
    AutoCloseMapView m_address;
    std::size_t m_numByte;
    std::string m_fileName;
    bool m_isOpenCreate;
public:
    FileMapping() = default;

    /**
     * Create a new file mapping
     * 
     * @param fileName Name of file mapping to create, must start with Global\ or Local\
     * @param numByte Size of memory to create, must in range of DWORD
     *
     * @return True on success
     * 
     * TODO: Support bigger range
     */
    bool Create(const std::string& fileName, std::size_t numByte);
    
    /**
     * Open a existng file mapping
     * 
     * @param fileName Name of file mapping to open, must start with Global\ or Local\
     * @param numByte Size of memory to open, must in range of DWORD
     *
     * @return True on success
     */
    bool Open(const std::string& fileName, std::size_t numByte);

    /**
     * Release open/created file mapping
     */
    void Release();

    /**
     * Write data to file mapping
     * 
     * @param buffer Pointer to data to write
     * @param length Size of data in byte to write
     * 
     * @return True on success
     */
    bool Write(const char* buffer, std::size_t length);

    /**
     * Read data from file mapping
     * 
     * @param numByte Size in byte to read
     * @param offset Possition to start reading
     * 
     * @return Data in string format, empty of read failed
     */
    std::string Read(std::size_t numByte, int offset = 0);

    /**
     * Read data in struct of <T> format
     * 
     * @param dst [Out] Pointer to allocate memory to fill data
     * @param offset Possition to start reading
     * 
     * @return True on success
     */
    template<class T>
    bool Read(T* dst, int offset = 0)
    {
        std::size_t structSize = sizeof(T);
        if (structSize > m_numByte)
        {
            LOG_ERROR << "Cannot read " << structSize << "byte, FileMapping open with " << m_numByte << std::endl;
            return false;
        }
        std::string buffer = Read(structSize, offset);
        std::memcpy(dst, buffer.c_str(), structSize);
    }
};
