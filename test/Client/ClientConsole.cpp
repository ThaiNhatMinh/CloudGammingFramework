#include "glfw/GlfwWindow.hh"
#include "ipc/WsaSocket.hh"
#include "cgf/InputEvent.hh"
#include "cgf/CloudGammingFramework.hh"

int main(int argc, char** argv)
{
    WsaSocket::Init();
    WsaSocket sock;
    unsigned short port = std::atoi(argv[1]);
    if (!sock.Connect("127.0.0.1", port))
    {
        return -1;
    }
    
    Window window(500, 500, "AAA");
    window.EnableVsync(true);
    window.SetKeyCallback([&sock](int key, int scancode, int action, int mods)
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
        std::cout << "Scancode: " << scancode << " mods: " << mods << std::endl;
        if (sock.SendAll(&event, sizeof(event)) != sizeof(event))
        {
            std::cout << "ERROR\n";
        }
    });
    while (!window.ShouldClose())
    {
        window.HandleEvent();
        window.SwapBuffer();
    }

    return 0;
}