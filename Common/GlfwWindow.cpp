#include "GlfwWindow.hh"
#include "Logger.hh"

Window::Window():Window(WIDTH, HEIGHT, DEFAULT_NAME)
{
}


Window::Window(uint32_t width, uint32_t height, const std::string& name):m_width(width), m_height(height), m_name(name)
{
    TRACE;
    m_bIsWindowResize = false;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    m_pWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(m_pWindow);
    glfwSetWindowAttrib(m_pWindow, GLFW_RESIZABLE, GLFW_FALSE);
    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, FramebufferResizeCallback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwSwapInterval(1); // Enable vsync
    int display_w, display_h;
    glfwGetFramebufferSize(m_pWindow, &display_w, &display_h);
    LOG << "Frame size: " << display_w << "x" << display_h << std::endl;
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

void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    pWindow->OnResize(width, height);
}

void Window::HandleEvent()
{
    glfwPollEvents();
}

void Window::Resize(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;
    glfwSetWindowSize(m_pWindow, m_width, m_height);
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