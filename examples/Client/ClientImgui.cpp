#include <chrono>

#include "common/Message.hh"
#include "common/BufferStream.hh"
#include "glad/glad.h"
#include "cgf/CloudGammingFrameworkClient.hh"
#include "cgf/CloudGammingOpenGL.hh"
#include "glfw/GlfwWindow.hh"
#include "ipc/WsaSocket.hh"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

int w, h, bpp1;
GLuint textId;
Window* window;
float uploadTime = 0;
void resFunc(unsigned int width, unsigned int height, unsigned char bpp)
{
    w = width;
    h = height;
    bpp1 = bpp;
    std::cout << w << " " << h << " " << bpp1 << std::endl;
    textId = cgfOpenglInit(w, h);
    window->Resize(w, h);
}

void frameFunc(const char* pFrameData)
{
    auto start = std::chrono::high_resolution_clock::now();
    cgfUpdateOpenglTexture(pFrameData);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::milli> delta = end - start;
    uploadTime = delta.count();
}

int main(int argc, char** argv)
{
    window = new Window(500, 500, "Client game");
    // window->EnableVsync(true);
    window->SetInputTextCallback([](unsigned int c)
    {
        InputEvent event;
        event.type = InputEvent::EventType::TEXT_INPUT;
        event.text.type = InputEvent::CharType::ASCII;
        event.text.character = c;
        cgfClientSendEvent(event);
    });
    window->SetScrollCallback([](double xoffset, double yoffset)
    {
        InputEvent event;
        event.type = InputEvent::EventType::SCROLL;
        event.scroll.xoffset = xoffset;
        event.scroll.yoffset = yoffset;
        cgfClientSendEvent(event);
    });
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
        cgfClientSendEvent(event);
    });

    window->SetMouseMoveCallback([](float xpos, float ypos){
        InputEvent event;
        event.type = InputEvent::EventType::MOUSE_MOVE;
        event.mousePos.x = xpos;
        event.mousePos.y = ypos;
        cgfClientSendEvent(event);
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
        cgfClientSendEvent(event);
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
    bool show_demo_window = false;
    float pollTime = 0;
    float openglTime = 0;
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
            auto start = std::chrono::high_resolution_clock::now();
            cgfClientPollEvent(0);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float, std::milli> duration = end - start;
            pollTime = duration.count();
            start = std::chrono::high_resolution_clock::now();
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
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);
            ImGui::Begin("Hello, world! from client");
            ImGui::Text("This is some useful text.");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("Upload time: %.2f ,\n poll time %.2f ,\n opengl time %.2f", uploadTime, pollTime, openglTime);
            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            end = std::chrono::high_resolution_clock::now();
            duration = end - start;
            openglTime = duration.count();
        }
        window->SwapBuffer();
    }
    if (isConnect)
        cfgClientCloseGame();
    else
        std::cout << "Not connected\n";
    cgfOpenglRelease();
    cgfClientFinalize();
    return 0;
}