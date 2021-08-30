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
    
    while(!cgfShouldExit())
    {
        cgfPollEvent();
        if (cgfGetKeyStatus(Key::KEY_SPACE) == Action::PRESSING)
            break;
    }
    std::cout <<  "Start cgfFinalize\n";
    cgfFinalize();
    std::cout <<  "Finish cgfFinalize\n";
    return 0;
}