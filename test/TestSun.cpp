#include "Sun.hh"
#include "GameParameter.hh"
#include "common/Configuration.hh"
#include <iostream>

int main()
{
    Configuration config;
    config.SetValue("PortRangeStart", 1000);
    config.SetValue("PortRangeEnd", 1010);
    std::vector<GameParameter> games;
    GameParameter console;
    console.Id = 1;
    console.ExecuteLocation = "D:\\Study\\CloudGaming\\CloudGamming\\build\\test\\Game\\Debug\\Console.exe";
    games.push_back(console);
    Sun sun(&config, games);
    if (!sun.LaunchGame(console.Id))
    {
        std::cout << "Load game failed\n";
    }
    int temp;
    std::cin >>  temp;
    return 0;
}