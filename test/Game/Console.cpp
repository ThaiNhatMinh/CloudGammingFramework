#include "cgf/CloudGammingFramework.hh"
#include <iostream>
#include <Windows.h>

char buffer[100*100*3];
void input(Action action, Key key)
{
    // std::cout << "Action: " << action << " Key: " << key << std::endl;

    if (action == Action::PRESS && key == Key::KEY_SPACE)
    {
        std::cout << "Send frame\n";
        cgfSetFrame(buffer);
    }
}

int main()
{
    for (int i = 0; i< 100*100*3; i++)
    {
        buffer[i] = ('A' + i) % 26;
    }

    InputCallback callback;
    callback.CursorPositionCallback = [](double xpos, double ypos) {};
    callback.MouseButtonCallback = [](Action action, int key) {};
    callback.KeyPressCallback = input;
    bool res = cgfRegisterGame("Test console", GraphicApi::DIRECTX_9, callback);
    if (!res)
    {
        std::cout << "Register failed\n";
        return -1;
    }
    cgfSetResolution(100, 100);
    int i = 0;
    int fps = 60;
    float perFrame = 1000.0f/60.0f;
    while(!cgfShouldExit())
    {
        cgfPollEvent();
        cgfSetFrame(buffer);
        Sleep(perFrame);
    }
    cgfFinalize();
    return 0;
}