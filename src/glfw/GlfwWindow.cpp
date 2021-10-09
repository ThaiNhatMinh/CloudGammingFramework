#include "GlfwWindow.hh"
#include "common/Logger.hh"

void glfwErrorCallback(int error_code,const char* des)
{
    LOG_ERROR << error_code << " " << des << std::endl;
}

Window::Window()
{
}


Window::Window(uint32_t width, uint32_t height, const std::string& name):m_width(width), m_height(height), m_name(name)
{
    TRACE;
    m_bIsWindowResize = false;
    glfwInit();
    glfwSetErrorCallback(&glfwErrorCallback);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    m_pWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    MakeContext();
    glfwSwapInterval(0);
    glfwSetWindowAttrib(m_pWindow, GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, FramebufferResizeCallback);
    glfwSetWindowPosCallback(m_pWindow, WindowMoveCallback);
    glfwSetKeyCallback(m_pWindow, KeyCallback);
    glfwSetMouseButtonCallback(m_pWindow, MouseCallback);
    glfwSetCursorPosCallback(m_pWindow, CursorPosCallback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    int display_w, display_h;
    glfwGetFramebufferSize(m_pWindow, &display_w, &display_h);
    m_mouseMoveCallback = [](double, double){};
}

Window::~Window()
{
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

int Window::ShouldClose()
{
    return glfwWindowShouldClose(m_pWindow);
}

void Window::OnResize(int width, int height)
{
    m_bIsWindowResize = true;
}

void Window::OnMove(int x, int y)
{
    if (m_moveCallback) m_moveCallback(x, y);
}

void Window::OnKey(int key, int scancode, int action, int mods)
{
    if (m_keyCallback) m_keyCallback(key, scancode, action, mods);
}

void Window::OnMouse(int button, int action, int mods)
{
    if (m_mouseCallback) m_mouseCallback(button, action, mods);
}

void Window::OnMouseMove(double xpos, double ypos)
{
    m_mouseMoveCallback(static_cast<float>(xpos), static_cast<float>(ypos));
}

void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    static Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    pWindow->OnResize(width, height);
}

void Window::WindowMoveCallback(GLFWwindow* window, int x, int y) {
    static Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    pWindow->OnMove(x, y);
}

void Window::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    static Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    pWindow->OnKey(key, scancode, action, mods);
}

void Window::MouseCallback(GLFWwindow* window,int button, int action, int mods)
{
    static Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    pWindow->OnMouse(button, action, mods);
}

void Window::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    static Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    pWindow->OnMouseMove(xpos, ypos);
}

void Window::HandleEvent()
{
    glfwPollEvents();
}

void Window::EnableVsync(bool enable)
{
    glfwSwapInterval(enable); // Enable vsync
}

void Window::Resize(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    glfwSetWindowSize(m_pWindow, m_width, m_height);
}

void Window::GetSize(uint32_t* width, uint32_t* height)
{
    *width = m_width;
    *height = m_height;
}

void Window::SetName(const std::string& name)
{
    m_name = name;
    glfwSetWindowTitle(m_pWindow, m_name.c_str());
}

GLFWwindow* Window::GetGlfw()
{
    return m_pWindow;
}

void Window::SwapBuffer()
{
    glfwSwapBuffers(m_pWindow);
}

void Window::MakeContext()
{
    glfwMakeContextCurrent(m_pWindow);
}

void Window::SetWinMoveCallback(std::function<void(int, int)> callback)
{
    m_moveCallback = callback;
}

void Window::SetKeyCallback(std::function<void(int, int, int, int)> callback)
{
    m_keyCallback = callback;
}

void Window::SetMouseCallback(std::function<void(int, int, int)> callback)
{
    m_mouseCallback = callback;
}

void Window::SetMouseMoveCallback(std::function<void(float, float)> callback)
{
    m_mouseMoveCallback = callback;
}

void Window::GetFramebufferSize(uint32_t* width, uint32_t* height)
{
    int w,h;
    glfwGetFramebufferSize(m_pWindow, &w, &h);
    *width = w;
    *height = h;
}