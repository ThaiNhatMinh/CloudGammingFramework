#include "Sun/Sun.hh"
#include "Sun/GameParameter.hh"
#include "common/Configuration.hh"
#include <iostream>

int main()
{
    Configuration config;
    config.SetValue("PortRangeStart", 1000);
    config.SetValue("PortRangeEnd", 1010);
    config.SetValue("Port", 8989);
    std::vector<GameParameter> games;
    GameParameter console;
    console.Id = 1;
    console.ExecuteLocation = "build\\examples\\Game\\Debug\\Console.exe";
    games.push_back(console);
    GameParameter imgui;
    imgui.Id = 2;
    imgui.ExecuteLocation = "build\\examples\\Game\\Debug\\ImGuiOpenGL.exe";
    imgui.ClientDisconnectTimeOut = 10 * 1000;
    games.push_back(imgui);
    Sun sun(&config, games);
    
    sun.Start();

    return 0;
}