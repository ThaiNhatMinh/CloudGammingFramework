#include "GlfwWindow.hh"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "StreamReceiver.hh"
#include "Logger.hh"

int main()
{
    Socket::InitSocket();
    Window window(500, 500, "AAA");
    window.EnableVsync(false);
    StreamReceiver stream(&window);
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
    ImGui_ImplGlfw_InitForOpenGL(window.GetGlfw(), true);
    ImGui_ImplOpenGL3_Init();
    while (!window.ShouldClose())
    {
        window.HandleEvent();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        stream.Draw();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.SwapBuffer();
    }
    Socket::DestroySocket();

    return 0;
}