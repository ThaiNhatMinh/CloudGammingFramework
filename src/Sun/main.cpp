#include "Sun.hh"
#include <iostream>

int main()
{
    Sun sun;
    if (!sun.LaunchGame(9))
    {
        std::cout << "Load game failed\n";
    }
    return 0;
}