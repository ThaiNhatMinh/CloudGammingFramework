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

std::string BuildSetupCommand(uint32_t w, uint32_t h, const std::string& title);