#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <thread>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD2
#include "glad/glad.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "GlfwWindow.hh"
#include "Socket.h"
#include "Logger.hh"
#include "StreamProtocol.hh"

extern struct gladGLversionStruct GLVersion;

void Web(Socket* client, Window *window);

int main()
{
    Socket::InitSocket();
    Socket client;
    Window window(400, 400, "AAA");
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        if (!gladLoadGL())
        {
            std::cout << GLVersion.major << ":" << GLVersion.minor << std::endl;
            std::cout << "Failed to initialize OpenGL context" << std::endl;
            return -1;
        }
    }

    std::thread thClient;
    // if (!client.Connect("127.0.0.1", 5678))
    // {
    //     LOG << "Failed to connect to server\n";
    //     return -1;
    // }

    // std::string buffer;
    // LOG << "Recv: " << client.Recv(buffer) << std::endl;
    // if (buffer.size() != 9)
    // {
    //     LOG << "Not enough message\n";
    //     LOG << buffer << std::endl;
    //     return -1;
    // }
    // Command cmd = (Command)buffer[0];
    // uint32_t w, h;
    // std::memcpy(&w, &buffer[1], sizeof(uint32_t));
    // std::memcpy(&h, &buffer[1] + sizeof(uint32_t), sizeof(uint32_t));
    // LOG << "CMD: " << cmd << " W:" << w << " H:" << h << std::endl;
    // if (cmd == Command::SETUP)
    // {
    //     window.Resize(w, h);
    // }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window.GetGlfw(), true);
    ImGui_ImplOpenGL3_Init();
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    char ipAddress[20] = {"127.0.0.1"};
    int port = 0;
    while(!window.ShouldClose()) {
        window.HandleEvent();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some useful text.");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::InputText("Ip address", ipAddress, 20);
        ImGui::InputInt("Port", &port);
        if (ImGui::Button("Connect"))
        {
            if (!client.Connect(ipAddress, port))
            {
                ImGui::OpenPopup("ConnectFailed");
            } else
            {
                thClient = std::thread(Web, &client, &window);
            }
        }

        if (ImGui::BeginPopup("ConnectFailed"))
        {
            ImGui::Text("Failed to connect to server!");
            ImGui::EndPopup();
        }
        ImGui::End();
        ImGui::Render();
        glViewport(0, 0, 200, 200);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.SwapBuffer();
    }
    Socket::DestroySocket();

    return 0;
}

GLuint CreateTexture(int w, int h)
{
    GLuint textId;
    glGenTextures(1, &textId);
    glBindTexture(GL_TEXTURE_2D, textId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    return textId;
}

void Web(Socket* client, Window* window)
{
    std::string buffer;
    LOG << "Recv: " << client->Recv(buffer) << std::endl;
    if (buffer.size() != 9)
    {
        LOG << "Not enough message\n";
        LOG << buffer << std::endl;
    }
    Command cmd = (Command)buffer[0];
    uint32_t w, h;
    std::memcpy(&w, &buffer[1], sizeof(uint32_t));
    std::memcpy(&h, &buffer[1] + sizeof(uint32_t), sizeof(uint32_t));
    LOG << "CMD: " << cmd << " W:" << w << " H:" << h << std::endl;
    if (cmd == Command::SETUP)
    {
        window->Resize(w, h);
    }
}