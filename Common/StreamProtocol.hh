#pragma once
#include <string>

enum Command
{
    /**
     * Send information about w and h, and byte per pixel
     */
    SETUP,
    /**
     * Send actualy frame
     */
    FRAME
};

struct SetupCommand
{
    int width;
    int height;
    std::string name;
};
struct FrameCommand
{
    std::string frame;
};
const int BYTE_PER_PIXEL = 3;

std::string BuildSetupCommand(uint32_t w, uint32_t h, const std::string& title);
SetupCommand ParseSetupCommand(const std::string& buffer);

std::string BuildFrameCommand(void* pData, std::size_t length);
FrameCommand ParseFrameCommand(const std::string& buffer, std::size_t frameSize);