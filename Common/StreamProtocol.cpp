#include "StreamProtocol.hh"

std::string BuildSetupCommand(uint32_t w, uint32_t h, const std::string& title)
{
    char buffer[64];
    memset(buffer, 0, 64);
    uint32_t length = 0;
    buffer[0] = Command::SETUP;
    length += 1;
    char* pData = &buffer[1];
    memcpy(pData, &w, sizeof(uint32_t));
    length += sizeof(uint32_t);
    pData += sizeof(uint32_t);
    memcpy(pData, &h, sizeof(uint32_t));
    length += sizeof(uint32_t);
    std::string res;
    res.resize(length);
    memcpy((void*)res.data(), buffer, length);
    res[length] = '\0';
    return res;
}