#include "common/Message.hh"
#include "common/BufferStream.hh"
#include "cgf/CloudGammingFrameworkClient.hh"
#include "glad/glad.h"
#include "glfw/GlfwWindow.hh"
#include "ipc/WsaSocket.hh"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

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
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, pFrameData);
}

int main(int argc, char** argv)
{
    window = new Window(500, 500, "Client game");
    window->EnableVsync(true);
    window->SetKeyCallback([](int key, int scancode, int action, int mods)
    {
        InputEvent event;
        event.type = InputEvent::EventType::KEY;
        event.key.key = static_cast<Key>(key);
        if (action == GLFW_PRESS)
            event.key.action = Action::PRESS;
        else if (action == GLFW_RELEASE)
            event.key.action = Action::RELEASE;
        else if (action == GLFW_REPEAT)
            event.key.action = Action::PRESSING;
        else
        {
            std::cout << "Unknow action: " << action;
            throw std::exception("Unknow action");
        }
        // std::cout << "Scancode: " << scancode << " mods: " << mods << std::endl;
        std::cout << "Action: " << event.key.action << " Key: " << event.key.key << std::endl;
        if (!cgfClientSendEvent(event))
        {
            std::cout << "ERROR\n";
        }
    });

    window->SetMouseMoveCallback([](float xpos, float ypos){
        InputEvent event;
        event.type = InputEvent::EventType::MOUSE_MOVE;
        event.mousePos.x = xpos;
        event.mousePos.y = ypos;
        if (!cgfClientSendEvent(event))
        {
            std::cout << "ERROR send event\n";
        }
    });
    window->SetMouseCallback([](int button, int action, int mods)
    {
        InputEvent event;
        event.type = InputEvent::EventType::MOUSE_ACTION;
        if (action == GLFW_PRESS) event.mouseAction.action = Action::PRESS;
        else if (action == GLFW_RELEASE) event.mouseAction.action = Action::RELEASE;
        else
        {
            std::cout << "Unknow action: " << action;
            throw std::exception("Unknow action");
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            event.mouseAction.key = MouseButton::LEFT;
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            event.mouseAction.key = MouseButton::RIGHT;
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
            event.mouseAction.key = MouseButton::MIDDLE;
        else 
        {
            std::cout << "Unknow button: " << button;
            throw std::exception("Unknow button");
        }
        if (!cgfClientSendEvent(event))
        {
            std::cout << "ERROR send SetMouseCallback event\n";
        }
    });
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        LOG << "Failed to initialize OpenGL context" << std::endl;
        if (!gladLoadGL())
        {
            LOG << GLVersion.major << ":" << GLVersion.minor << std::endl;
            LOG << "Failed to initialize OpenGL context" << std::endl;
            return -1;
        }
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window->GetGlfw(), true);
    ImGui_ImplOpenGL3_Init();

    if (!cgfClientInitialize(resFunc, frameFunc))
    {
        return -1;
    }
    bool isConnect = false;
    char m_ip[20] = {"127.0.0.1"};
    int m_port = 8989;
    while (!window->ShouldClose())
    {
        window->HandleEvent();
        glViewport(0, 0, w, h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (!isConnect)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("Hello, world!");
            ImGui::Text("This is some useful text.");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::InputText("Ip address", m_ip, 20);
            ImGui::InputInt("Port", &m_port);
            if (ImGui::Button("Connect"))
            {
                if (!cgfClientConnect(123, m_ip, m_port))
                {
                    ImGui::OpenPopup("ConnectFailed");
                } else if (cfgClientRequestGame(2))
                {
                    isConnect = true;
                }
            }

            if (ImGui::BeginPopup("ConnectFailed"))
            {
                ImGui::Text("Failed to connect to server!");
                ImGui::EndPopup();
            }
            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        } else {
            cgfClientPollEvent(1);
            glFrontFace(GL_CW);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textId);
            glBegin(GL_TRIANGLE_STRIP);

            glTexCoord2f(0.0, 1.0);
            glVertex2f(-1.0f, 1.0f); //vertex 1

            glTexCoord2f(0.0, 0.0);
            glVertex2f(-1.0f, -1.0f); //vertex 2

            glTexCoord2f(1.0, 1.0);
            glVertex2f(1.0f, 1.0f); //vertex 3

            glTexCoord2f(1.0, 0.0);
            glVertex2f(1.0f, -1.0f); //vertex 4
            glEnd();
        }
        window->SwapBuffer();
    }
    if (isConnect)
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    return textId;
}