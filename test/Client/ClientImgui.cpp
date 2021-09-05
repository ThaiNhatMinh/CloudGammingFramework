#include "glad/glad.h"
#include "common/Message.hh"
#include "glfw/GlfwWindow.hh"
#include "ipc/WsaSocket.hh"
#include "cgf/CloudGammingFrameworkClient.hh"
#include "common/BufferStream.hh"

int w, h, bpp1;
GLuint textId;
GLuint CreateTexture(int w, int h);
Window* window;
void resFunc(unsigned int width, unsigned int height, unsigned char bpp)
{
    w = width;
    h = height;
    bpp1 = bpp;
    std::cout << w << " " << h << " " << bpp1 << std::endl;
    textId = CreateTexture(w, h);
    window->Resize(w, h);
}

void frameFunc(const char* pFrameData)
{
    glBindTexture(GL_TEXTURE_2D, textId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pFrameData);
}

int main(int argc, char** argv)
{
    window = new Window(500, 500, "Client game");
    window->EnableVsync(true);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        LOG << "Failed to initialize OpenGL context" << std::endl;
        if (!gladLoadGL())
        {
            LOG << GLVersion.major << ":" << GLVersion.minor << std::endl;
            LOG << "Failed to initialize OpenGL context" << std::endl;
            return -1;
        }
    }
    if (!cgfClientInitialize(resFunc, frameFunc))
    {
        return -1;
    }
    if (!cgfClientConnect(123, "127.0.0.1", 8989))
    {
        return -1;
    }
    // Sleep(100);
    if (!cfgClientRequestGame(2))
    {
        return -1;
    }
    window->SetKeyCallback([](int key, int scancode, int action, int mods)
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
    while (!window->ShouldClose())
    {
        window->HandleEvent();
        cgfClientPollEvent(5);
        glViewport(0, 0, w, h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        glFrontFace(GL_CW);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textId);
        glBegin(GL_TRIANGLE_STRIP);

        glTexCoord2f(0.0, 0.0);
        glVertex2f(-1.0f, 1.0f); //vertex 1

        glTexCoord2f(0.0, 1.0);
        glVertex2f(-1.0f, -1.0f); //vertex 2

        glTexCoord2f(1.0, 0.0);
        glVertex2f(1.0f, 1.0f); //vertex 3

        glTexCoord2f(1.0, 1.0);
        glVertex2f(1.0f, -1.0f); //vertex 4
        glEnd();
        window->SwapBuffer();
    }
    cfgClientCloseGame();
    cgfClientFinalize();
    return 0;
}

GLuint CreateTexture(int w, int h)
{
    TRACE;
    GLuint textId;
    glGenTextures(1, &textId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    return textId;
}