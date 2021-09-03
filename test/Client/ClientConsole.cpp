#include "common/Message.hh"
#include "glfw/GlfwWindow.hh"
#include "ipc/WsaSocket.hh"
#include "cgf/CloudGammingFrameworkClient.hh"
#include "common/BufferStream.hh"

int main(int argc, char** argv)
{
    if (!cgfClientConnect(123, "127.0.0.1", 8989))
    {
        return -1;
    }
    // Sleep(100);
    if (!cfgClientRequestGame(1))
    {
        return -1;
    }

    Window window(500, 500, "AAA");
    window.EnableVsync(true);
    window.SetKeyCallback([](int key, int scancode, int action, int mods)
    {
        InputEvent event;
        event.type = InputEvent::EventType::KEY;
        event.key.key = static_cast<Key>(key);
        if (action == GLFW_PRESS)
            event.key.action = Action::PRESS;
        else if (action == GLFW_RELEASE)
            event.key.action = Action::RELEASE;
        else
            event.key.action = Action::PRESSING;
        // std::cout << "Scancode: " << scancode << " mods: " << mods << std::endl;
        std::cout << "Action: " << event.key.action << " Key: " << event.key.key << std::endl;
        if (!cgfClientSendEvent(event))
        {
            std::cout << "ERROR\n";
        }
    });
    while (!window.ShouldClose())
    {
        window.HandleEvent();
        window.SwapBuffer();
    }

    cgfClientFinalize();
    return 0;
}