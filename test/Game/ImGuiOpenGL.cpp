#include "glad/glad.h"
#include "cgf/CloudGammingFramework.hh"
#include "common/Logger.hh"
#include "glfw/GlfwWindow.hh"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <chrono>

constexpr std::size_t W = 800, H = 600;
const int PBO_COUNT = 2;
GLuint pboIds[PBO_COUNT];           // IDs of PBOs
const int DATA_SIZE = W * H * 4;
void createfpo();
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
    createfpo();
    InputCallback callback;
    callback.CursorPositionCallback = [](double xpos, double ypos) {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2((float)xpos, (float)ypos);
    };
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
    cgfSetResolution(W, H, 4);
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
    ImGui_ImplGlfw_InitForOpenGL(window.GetGlfw(), false);
    ImGui_ImplOpenGL3_Init();
    bool show_demo_window, show_another_window;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    double imguiTime = 0;
    double sendFrameTime = 0;
    uint32_t frameCount = 0;
    while (!cgfShouldExit())
    {
        window.HandleEvent();
        auto start = std::chrono::high_resolution_clock::now();
        cgfPollEvent();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end-start;
        double pollTime = elapsed.count();

        start = std::chrono::high_resolution_clock::now();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_AlwaysAutoResize);                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            ImGui::SameLine();
            ImGui::Text("Frame counter = %d", ++frameCount);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("cgfPollEvent: %.3f imgui: %.3f sendFrameTime: %.10f", pollTime, imguiTime, sendFrameTime);
            ImGui::End();
        }
        ImGui::Render();
        end = std::chrono::high_resolution_clock::now();
        elapsed = end-start;
        imguiTime = elapsed.count();

        glViewport(0, 0, W, H);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        start = std::chrono::high_resolution_clock::now();
        static int index = 0;
        int nextIndex = 0;                  // pbo index used for next frame
        index = (index + 1) % 2;
        nextIndex = (index + 1) % 2;

        glReadBuffer(GL_FRONT);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[index]);
        glReadPixels(0, 0, W, H, GL_BGRA, GL_UNSIGNED_BYTE , 0);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[nextIndex]);
        GLubyte* src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

        if(src)
        {
            cgfSetFrame(src);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);        // release pointer to the mapped buffer
        } else {
            LOG_ERROR << "NULL\n";
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        
        end = std::chrono::high_resolution_clock::now();
        elapsed = end-start;
        sendFrameTime = elapsed.count();

        window.SwapBuffer();
    }
    cgfFinalize();

    return 0;
}

void createfpo()
{
    // create 2 pixel buffer objects, you need to delete them when program exits.
    // glBufferData() with NULL pointer reserves only memory space.
    glGenBuffers(PBO_COUNT, pboIds);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[0]);
    glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[1]);
    glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}