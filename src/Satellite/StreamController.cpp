#include "StreamController.hh"
#include "common/BufferStream.hh"
#include "common/Message.hh"
#include "ipc/Named.hh"

bool StreamController::Init(const std::string& ip, StreamPort startPort, const std::vector<uint32_t>& bytePerSocket)
{
    m_frameCount = 0;
    uint32_t offset = 0;
    m_data.resize(bytePerSocket.size());
    std::string eventStr = CreateStopPollString(this);
    if (!m_stopThreadEvent.Create(eventStr))
    {
        throw std::exception("Failed to create event");
    }
    for (int i = 0; i < bytePerSocket.size(); i++)
    {
        SockPackage& package = m_data[i];
        LOG_DEBUG << "Port connect: " << startPort + i << '\n';
        if (!package.socket.Connect(ip, i + startPort))
        {
            m_data.clear();
            return false;
        }
        // package.finish = false;
        package.numByte = bytePerSocket[i];
        package.offset = offset;
        package.isRecv = false;
        package.recvThread = std::thread(&StreamController::PollThread, this, i);
        package.currentByte = 0;
        if (!package.stopPollEvent.Open(eventStr)) throw std::exception("Failed to open event");
        package.poll.AddSocketForRecv(package.socket, std::bind(&StreamController::OnClose, this, std::placeholders::_1, std::placeholders::_2),
                std::bind(&StreamController::OnRecv, this, i, std::placeholders::_1, std::placeholders::_2));
        package.poll.AddEvent(package.stopPollEvent, [](const Event* e){ return PollAction::STOP_POLL; });
        offset += bytePerSocket[i];
    }
    m_currentFrame.reset(new char[offset]);
    m_frames.Init(20, offset);
    m_numFinish = 0;
    if (!m_frameCompleteEvent.Create(CreateStreamControllerString(this), false, true))
        throw std::exception("Failed to create frame complete event");
    return true;
}

void StreamController::Stop()
{
    for (std::size_t i = 0; i < m_data.size(); i++)
    {
        m_stopThreadEvent.Signal();
        // FIXME: Somehow thread is finsh but .join() not return, use detach is a workaround
        m_data[i].recvThread.detach();
    }
}

void StreamController::OnClose(WsaSocket* sock, BufferStream<MAX_BUFFER>*)
{
    sock->Release();
}

void StreamController::PollThread(int slot)
{
    SockPackage& package = m_data[slot];
    package.poll.PollSocket(INFINITE);
    LOG_DEBUG << "Poll thread finish\n";
}

void StreamController::OnRecv(int slot, WsaSocket* sock, BufferStream<MAX_BUFFER>* buffer)
{
    if (buffer->Length() < MSG_HEADER_LENGTH) return;
    SockPackage& package = m_data[slot];
    buffer->SetCurrentPosition(0);
    if (package.isRecv)
    {
        uint32_t byteToCopy = (package.numByte - package.currentByte) > buffer->Length() ? buffer->Length() : package.numByte - package.currentByte;
        buffer->Extract(m_currentFrame.get() + package.offset + package.currentByte, byteToCopy);
        package.currentByte += byteToCopy;
        if (package.currentByte == package.numByte)
        {
            package.currentByte = 0;
            package.isRecv = false;
            m_numFinish += 1;
            if (m_numFinish == m_data.size() && m_lock.try_lock())
            {
                m_frameCount++;
                m_frames.PushBack(m_currentFrame.get());
                m_frameCompleteEvent.Signal();
                m_numFinish = 0;
                m_lock.unlock();
            }
        }
    } else
    {
        MessageHeader header;
        *buffer >> header;
        if (header.code == Message::MSG_FRAME)
        {
            package.isRecv = true;
        } else 
        {
            throw std::exception("Invalid code");
        }
    }

    buffer->SetCurrentPosition(buffer->Length());
}