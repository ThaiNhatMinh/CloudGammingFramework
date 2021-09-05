#include "glad/glad.h"
#include "cgf/CloudGammingFramework.hh"
#include "common/Logger.hh"
#include "glfw/GlfwWindow.hh"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

constexpr std::size_t W = 800, H = 600;
char buffer[W* H * 3];
int main()
{
    Window window(W, H, "Server game");
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        LOG << "Failed to initialize OpenGL context" << std::endl;
        if (!gladLoadGL())
        {
            LOG << GLVersion.major << ":" << GLVersion.minor << std::endl;
            LOG << "Failed to initialize OpenGL context" << std::endl;
            return -1;
        }
    }
    window.EnableVsync(true);
    InputCallback callback;
    callback.CursorPositionCallback = [](double xpos, double ypos) {};
    callback.MouseButtonCallback = [](Action action, int key) {};
    callback.KeyPressCallback = [](Action action, Key key)
    {
        std::cout << "Action: " << action << " Key: " << key << std::endl;
    };
    if (!cgfRegisterGame("Test console", GraphicApi::OPENGL, callback))
    {
        std::cout << "Register failed\n";
        return -1;
    }
    cgfSetResolution(W, H);
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
    bool show_demo_window, show_another_window;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    while (!cgfShouldExit())
    {
        window.HandleEvent();
        cgfPollEvent();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        ImGui::Render();
        glViewport(0, 0, W, H);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glReadBuffer( GL_FRONT );
        glReadPixels( 0, 0, W, H, GL_RGB, GL_UNSIGNED_BYTE, buffer); 
        cgfSetFrame(buffer);
        window.SwapBuffer();
    }
    cgfFinalize();

    return 0;
}