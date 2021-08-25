#include "common/Module.hh"
#include "cgf/CloudGammingFramework.hh"
#include <iostream>
int main()
{
    LastError();
    bool res = cgfRegisterGame("Test console", GraphicApi::DIRECTX_9, NULL, [](unsigned int msg, MY_WPARAM wParam, MY_LPARAM lParam) {

    });

    if (!res)
    {
        std::cout << "Register failed\n";
    }
    return 0;
}