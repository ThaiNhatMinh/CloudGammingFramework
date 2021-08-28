#include "cgf/CloudGammingFramework.hh"
#include <iostream>
int main()
{
    InputCallback callback;
    callback.CursorPositionCallback = [](double xpos, double ypos) {};
    callback.MouseButtonCallback = [](Action action, int key) {};
    callback.KeyPressCallback = [](Action action, Key key) {};
    bool res = cgfRegisterGame("Test console", GraphicApi::DIRECTX_9, callback);

    if (!res)
    {
        std::cout << "Register failed\n";
    }
    int temp;
    std::cin >>  temp;
    return 0;
}