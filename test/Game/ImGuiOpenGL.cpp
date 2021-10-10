#include "glad/glad.h"
#include "cgf/CloudGammingFramework.hh"
#include "common/Logger.hh"
#include "glfw/GlfwWindow.hh"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <chrono>

constexpr std::size_t W = 800, H = 600;
const int PBO_COUNT = 2;
GLuint pboIds[PBO_COUNT];           // IDs of PBOs
const int DATA_SIZE = W * H * 4;
void createfpo();
void ImGui_ImplGlfw_NewFrame(Window * pWindow);
void ImgGui_InitForCloud();

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
    callback.TextInputCallback = [](InputEvent::CharType type, unsigned int ch)
    {
        ImGuiIO& io = ImGui::GetIO();
        if (type == InputEvent::CharType::ASCII)
            io.AddInputCharacter(ch);
        // else if (type == InputEvent::CharType::UTF8)
        //     io.AddInputCharactersUTF8(static_cast<const char*>(&ch));
        else if (type == InputEvent::CharType::UTF16)
            io.AddInputCharacterUTF16(ch);
    };
    callback.CursorPositionCallback = [](double xpos, double ypos) {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2((float)xpos, (float)ypos);
    };
    callback.ScrollCallback = [](double xoffset, double yoffset)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheelH += (float)xoffset;
        io.MouseWheel += (float)yoffset;
    };
    callback.MouseButtonCallback = [](Action action, MouseButton key)
    {
        ImGuiIO& io = ImGui::GetIO();
        bool down = action == Action::PRESS;
        if (key == MouseButton::LEFT) io.MouseDown[0] = down;
        else if (key == MouseButton::RIGHT) io.MouseDown[1] = down;
        else if (key == MouseButton::MIDDLE) io.MouseDown[2] = down;
    };
    callback.KeyPressCallback = [](Action action, Key key)
    {
        std::cout << "Action: " << action << " Key: " << key << std::endl;
        ImGuiIO& io = ImGui::GetIO();
        if (action == Action::PRESS)
            io.KeysDown[key] = true;
        if (action == Action::RELEASE)
            io.KeysDown[key] = false;
        // Modifiers are not reliable across systems
        io.KeyCtrl = io.KeysDown[Key::KEY_LEFT_CONTROL] || io.KeysDown[Key::KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[Key::KEY_LEFT_SHIFT] || io.KeysDown[Key::KEY_RIGHT_SHIFT];
        io.KeyAlt = io.KeysDown[Key::KEY_LEFT_ALT] || io.KeysDown[Key::KEY_RIGHT_ALT];
        io.KeySuper = false;
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
    ImgGui_InitForCloud();
    ImGui_ImplOpenGL3_Init();
    bool show_demo_window = false, show_another_window;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    double imguiTime = 0;
    double sendFrameTime = 0;
    uint32_t frameCount = 0;
    char buffer[512] = {0};
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
        ImGui_ImplGlfw_NewFrame(&window);
        ImGui::NewFrame();
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        {
            ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_AlwaysAutoResize);                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            ImGui::SameLine();
            ImGui::Text("Frame counter = %d", ++frameCount);
            ImGui::InputText("Type something:", buffer, 512);

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

void ImGui_ImplGlfw_NewFrame(Window *pWindow)
{
    ImGuiIO& io = ImGui::GetIO();
    static double lasTime = 0;
    // Setup display size (every frame to accommodate for window resizing)
    uint32_t w, h;
    uint32_t display_w, display_h;
    pWindow->GetSize(&w, &h);
    pWindow->GetFramebufferSize(&display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    // Setup time step
    double current_time = glfwGetTime();
    io.DeltaTime = lasTime > 0.0 ? (float)(current_time - lasTime) : (float)(1.0f / 60.0f);
    lasTime = current_time;

    // Update mouse buttons
    // (if a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame)
//     for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
//     {
//         io.MouseDown[i] = cgfGetKeyStatus != 0;
//     }
}

void ImgGui_InitForCloud(Window* win)
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_cloud";
    io.ImeWindowHandle = win->GetNativeHandle();
    // Keyboard mapping. Dear ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = Key::KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = Key::KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = Key::KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = Key::KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = Key::KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = Key::KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = Key::KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = Key::KEY_HOME;
    io.KeyMap[ImGuiKey_End] = Key::KEY_END;
    io.KeyMap[ImGuiKey_Insert] = Key::KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = Key::KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = Key::KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = Key::KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = Key::KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = Key::KEY_ESCAPE;
    // io.KeyMap[ImGuiKey_KeyPadEnter] = Key::P;
    io.KeyMap[ImGuiKey_A] = Key::KEY_A;
    io.KeyMap[ImGuiKey_C] = Key::KEY_C;
    io.KeyMap[ImGuiKey_V] = Key::KEY_V;
    io.KeyMap[ImGuiKey_X] = Key::KEY_X;
    io.KeyMap[ImGuiKey_Y] = Key::KEY_Y;
    io.KeyMap[ImGuiKey_Z] = Key::KEY_Z;
}