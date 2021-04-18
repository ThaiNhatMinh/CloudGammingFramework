#include "glad/glad.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "StreamReceiver.hh"
#include "Logger.hh"

StreamReceiver::State::State(StreamReceiver* owner):m_owner(owner)
{
}

StreamReceiver::HomeState::HomeState(StreamReceiver* owner):State(owner), m_ip("127.0.0.1"), m_port(5678)
{
}

void StreamReceiver::HomeState::Draw()
{
    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::InputText("Ip address", m_ip, 20);
    ImGui::InputInt("Port", &m_port);
    if (ImGui::Button("Connect"))
    {
        if (!m_owner->ConnectTo(m_ip, m_port))
        {
            ImGui::OpenPopup("ConnectFailed");
        }
    }

    if (ImGui::BeginPopup("ConnectFailed"))
    {
        ImGui::Text("Failed to connect to server!");
        ImGui::EndPopup();
    }
    ImGui::End();
}

StreamReceiver::Streamming::Streamming(StreamReceiver* owner, V2uInt32 size):State(owner), m_size(size), m_texture(0), m_update(false)
{
}

void StreamReceiver::Streamming::Draw()
{
    ImGui::Begin("Hello, world!");
    ImGui::Text("Queue size %d", m_frames.size());
    if (m_update)
    {
        Update();
    }
    ImGui::End();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texture);
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

GLuint StreamReceiver::Streamming::CreateTexture()
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
    // int w, h, comp;
    // auto data = stbi_load("AAA.bmp",  &w, &h, &comp, 4);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // stbi_image_free(data);
    return textId;
}

void StreamReceiver::Streamming::Update()
{
    if (m_texture == 0)
    {
        m_texture = CreateTexture();
    }
    glBindTexture(GL_TEXTURE_2D, m_texture);
    // int w, h, comp;
    // auto data = stbi_load("AAA.jpg",  &w, &h, &comp, 4);
    // LOG << " " << w << " " << h << " " << comp <<  " " << data << std::endl;
    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
    // std::unique_lock<std::mutex> lock(m_frameLock);
    FrameCommand frame = m_frames.front();
    m_frames.pop();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_size.x, m_size.y, GL_RGBA, GL_UNSIGNED_BYTE, frame.frame.data());
    // stbi_write_bmp("AAA.bmp", m_size.x, m_size.y, BYTE_PER_PIXEL, m_frame.frame.data());
    // stbi_image_free(data);
    m_update = false;
}

void StreamReceiver::Streamming::Update(const FrameCommand& frame)
{
    // std::unique_lock<std::mutex> lock(m_frameLock);
    m_frames.push(frame);
    // LOG << "Push frame, size: " << m_frames.size() << std::endl;
    m_update = true;
}

bool StreamReceiver::ConnectTo(const std::string& ip, int port)
{
    if (m_socket.Connect(ip, port))
    {
        m_pCurrentState.reset();
        m_thread = std::thread(&StreamReceiver::Receive, this);
        return true;
    }
    return false;
}

StreamReceiver::StreamReceiver(Window* pWindow):m_pWindow(pWindow)
{
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        LOG << "Failed to initialize OpenGL context" << std::endl;
        if (!gladLoadGL())
        {
            LOG << GLVersion.major << ":" << GLVersion.minor << std::endl;
            LOG << "Failed to initialize OpenGL context" << std::endl;
            return;
        }
    }
    m_pWindow->GetSize(&m_size.x, &m_size.y);
    m_pCurrentState.reset(new HomeState(this));
    // m_pCurrentState.reset(new Streamming(this, m_size));
}

StreamReceiver::~StreamReceiver()
{
}

void StreamReceiver::Draw()
{
    glViewport(0, 0, m_size.x, m_size.y);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFrontFace(GL_CW);
    if (m_pCurrentState) m_pCurrentState->Draw();
}

void StreamReceiver::Receive()
{
    std::string buffer;
    while (buffer.size() < 9)
    {
        std::string tmp;
        m_socket.Recv(tmp);
        buffer.append(tmp);
    }
    SetupCommand setup = ParseSetupCommand(buffer);
    m_size.x = setup.width;
    m_size.y = setup.height;
    m_pWindow->Resize(setup.width, setup.height);
    m_pWindow->SetName(setup.name);
    buffer.clear();
    auto pStream = new Streamming(this, m_size);
    m_pCurrentState.reset(pStream);
    while(true)
    {
        std::string tmp;
        int recv = m_socket.Recv(tmp);
        if (recv == -1)
        {
            LOG << "Error from server\n";
            return;
        }
        buffer.append(tmp);
        if (buffer.size() >= setup.width * setup.height * BYTE_PER_PIXEL + 1)
        {
            FrameCommand frame = ParseFrameCommand(buffer, setup.width * setup.height * BYTE_PER_PIXEL);
            pStream->Update(frame);
            buffer = buffer.substr(setup.width * setup.height * BYTE_PER_PIXEL + 1);
        }
    }
}
