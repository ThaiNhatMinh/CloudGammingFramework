#include "cgf/CloudGammingFramework.hh"
#include <iostream>
#include <Windows.h>

int main()
{
    InputCallback callback;
    callback.CursorPositionCallback = [](double xpos, double ypos) {};
    callback.MouseButtonCallback = [](Action action, int key) {};
    callback.KeyPressCallback = [](Action action, Key key)
    {
        std::cout << "Action: " << action << " Key: " << key << std::endl;
    };
    bool res = cgfRegisterGame("Test console", GraphicApi::DIRECTX_9, callback);

    if (!res)
    {
        std::cout << "Register failed\n";
    }
    
    while(true)
    {
        cgfPollEvent();
    }
    return 0;
}