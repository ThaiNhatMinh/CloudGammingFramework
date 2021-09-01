#include "common/BufferStream.hh"
#include "cgf/cgf.hh"
#include <iostream>

int main()
{
    BufferStream1KB buffer;
    {
        InputEvent event;
        event.key.key = Key::KEY_F12;
        event.key.action = Action::PRESSING;
        buffer << event;
        
        event.key.key = Key::KEY_F13;
        event.key.action = Action::RELEASE;
        buffer << event;
    }
    {
        buffer.SetCurrentPosition(0);
        InputEvent event;
        buffer >> event;
        if (event.key.key != Key::KEY_F12 || event.key.action != Action::PRESSING) std::cerr << "Failed!\n";
        buffer >> event;
        if (event.key.key != Key::KEY_F13 || event.key.action != Action::RELEASE) std::cerr << "Failed!\n";
    }

    return 0;
}