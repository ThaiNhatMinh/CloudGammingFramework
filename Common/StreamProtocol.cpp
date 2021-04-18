#include "StreamProtocol.hh"
#include "Logger.hh"

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

SetupCommand ParseSetupCommand(const std::string& buffer)
{
    SetupCommand res;
    res.name = "Test name";
    Command cmd = (Command)buffer[0];
    std::memcpy(&res.width, &buffer[1], sizeof(uint32_t));
    std::memcpy(&res.height, &buffer[1] + sizeof(uint32_t), sizeof(uint32_t));
    LOG << "CMD: " << cmd << " W:" << res.width << " H:" << res.height << std::endl;
    return res;
}

std::string BuildFrameCommand(void* pData, std::size_t length)
{
    std::string res;
    res.resize(length);
    res[0] = Command::FRAME;
    void* pStart = &res[1];
    memcpy(pStart, pData, length - 1);
    return res;
}

FrameCommand ParseFrameCommand(const std::string& buffer, std::size_t frameSize)
{
    FrameCommand res;
    Command cmd = (Command)buffer[0];
    res.frame = buffer.substr(1, frameSize);
    // LOG << "CMD: " << cmd << " Size:" << res.frame.size() << std::endl;
    return res;
}