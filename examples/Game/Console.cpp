#include "cgf/CloudGammingFramework.hh"
#include <iostream>
#include <Windows.h>

const int w = 1920;
const int h = 1080;
const int bpp = 3;
char buffer[w*h*bpp];
void input(Action action, Key key)
{
    std::cout << "Action: " << action << " Key: " << key << std::endl;
}

int main()
{
    for (int i = 0; i< w*h*bpp; i++)
    {
        buffer[i] = 'A' + i % 26;
    }
    buffer[0] = 1;
    buffer[1] = 1;
    buffer[2] = 1;
    buffer[ w*h*bpp - 1] = 1;
    buffer[ w*h*bpp - 2] = 1;
    buffer[ w*h*bpp - 3] = 1;

    InputCallback callback;
    callback.CursorPositionCallback = [](double xpos, double ypos) {};
    callback.MouseButtonCallback = [](Action action, MouseButton key) {};
    callback.KeyPressCallback = input;
    bool res = cgfRegisterGame("Test console", GraphicApi::DIRECTX_9, callback);
    if (!res)
    {
        std::cout << "Register failed\n";
        return -1;
    }
    cgfSetResolution(w, h, bpp);
    int i = 0;
    int fps = 30;
    DWORD perFrame = 1000/60;
    while(!cgfShouldExit())
    {
        cgfPollEvent(DispatchType::ALL);
        cgfSetFrame(buffer);
    }
    cgfFinalize();
    return 0;
}