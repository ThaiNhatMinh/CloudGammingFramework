#include "StreamController.hh"
#include "common/Module.hh"
#include "ipc/Named.hh"
#include "common/Message.hh"
#include <chrono>

void StreamController::SetInformation(const StreamInformation &info, StreamPort startPort)
{
    m_info = info;
    m_startPort = startPort;
    m_totalByte = info.Width * info.Heigh * info.BytePerPixel;
    uint32_t currentSize = 0;
    uint32_t bytePerSocket = m_totalByte / info.NumSocket;
    std::string stopPollStr = CreateStopPollString(this);
    if (!m_stopPollEvent.Create(stopPollStr)) throw std::exception("Create m_stopPollEvent event failed");
    if (!m_sendFrameEvent.Create(SENDFRAME_EVENT)) throw std::exception("Failed to create SENDFRAME_EVENT");

    for (StreamPort i = 0; i < info.NumSocket; i++)
    {
        LOG_DEBUG << "Port open: " << startPort + i << '\n';
        WsaSocket listen;
        if (!listen.Open(startPort + i))
        {
            throw std::exception("Failed to open port");
        }
        m_packages.push_back({});
        Package& package = m_packages.back();
        m_listenSocket.push_back(std::move(listen));
        if (!POLL_ADD_SOCKET_LISTEN(m_socketPoll, m_listenSocket.back(), &StreamController::OnClose, &StreamController::OnAccept))
        {
            throw std::exception("Failed to add socket to listen");
        }
        package.offset = currentSize;
        if (i < info.NumSocket - 1)
        {
            package.length = bytePerSocket;
            currentSize += bytePerSocket;
        } else {
            package.length = m_totalByte - currentSize;
            currentSize += package.length;
        }
        LOG_DEBUG << "Byte send: " << package.length << '\n';
        if (!package.sendEvent.Open(SENDFRAME_EVENT)) throw std::exception("Failed to open SENDFRAME_EVENT");
        if (!package.stopEvent.Open(stopPollStr)) throw std::exception("Failed to open SENDFRAME_EVENT");
        package.thread = std::thread(&StreamController::SendThread, this, &package);
        package.status = Status::ILDE;
        package.poll.AddEvent(package.sendEvent, std::bind(&StreamController::SendData, this, &package, std::placeholders::_1));
        package.poll.AddEvent(package.stopEvent, [](const Event*) { return PollAction::STOP_POLL;});
        package.socket = WsaSocket();
    }

    LOG_DEBUG << "currentSize: " << currentSize << " m_totalByte: " << m_totalByte << std::endl;
    if (m_totalByte != currentSize) throw std::exception("Wrong size");

    m_socketPoll.AddEvent(m_stopPollEvent, [](const Event* e){ return PollAction::STOP_POLL; });
    m_pollThread = std::thread(&StreamController::InternalThread, this);
    m_buffer.reset(new char[m_totalByte]);
    m_bFullSocket = false;
}

void StreamController::OnAccept(WsaSocket* newConnect, BufferStream10KB*)
{
    int iOptVal = 0;
    int iOptLen = sizeof (int);
    int iResult = getsockopt(newConnect->GetHandle(), SOL_SOCKET, SO_SNDBUF, (char *) &iOptVal, &iOptLen);
    if (iResult == SOCKET_ERROR)
    {
        LASTERROR;
    } else {
        LOG_DEBUG << "SO_SNDBUF:" << iOptVal << std::endl;
    }
    iOptVal = m_info.Width * m_info.Heigh * m_info.BytePerPixel * 3;
    iResult = setsockopt(newConnect->GetHandle(), SOL_SOCKET, SO_SNDBUF, (char *) &iOptVal, iOptLen);
    if (iResult == SOCKET_ERROR)
    {
        LASTERROR;
    } else {
        LOG_DEBUG << "SO_SNDBUF:" << iOptVal << std::endl;
    }
    uint8_t count = 0;
    auto iter = m_packages.begin();
    for (; iter != m_packages.end(); iter++, count++)
    {
        LOG_DEBUG << iter->socket.GetHandle() << '\n';
        if (!iter->socket.IsValid()) break;
    }
    if (iter == m_packages.end())
    {
        LOG_ERROR << "Invalid connect to game\n";
    } else
    {
        iter->socket = std::move(*newConnect);
        POLL_ADD_SOCKET_RECV(m_socketPoll, iter->socket, &StreamController::OnClose, &StreamController::OnEmptyRecv);
        LOG_INFO << "Stream socket connected\n";
        if (count == m_info.NumSocket - 1) m_bFullSocket = true;
    }
}
void StreamController::OnEmptyRecv(WsaSocket*, BufferStream10KB*)
{
    throw std::exception("This socket should not recieve data");
}

void StreamController::OnClose(WsaSocket* sock, BufferStream10KB* buffer)
{
    sock->Release();
    auto iter = m_packages.begin();
    while (iter != m_packages.end())
    {
        if (iter->socket == *sock)
        {
            m_bFullSocket = false;
            break;
        }
        iter++;
    }
    
}

void StreamController::InternalThread()
{
    m_socketPoll.PollSocket(INFINITE);
}

void StreamController::StopPoll()
{
    m_stopPollEvent.Signal();
    if (m_pollThread.joinable()) m_pollThread.join();
}

void StreamController::SetFrame(const void* pData)
{
    if (!m_bFullSocket) return;
    auto start = std::chrono::high_resolution_clock::now();
    m_frameCounter++;
    std::memcpy(m_buffer.get(), pData, m_totalByte);
    m_sendFrameEvent.Signal();
}

void StreamController::SendThread(Package* package)
{
    LOG_DEBUG << "Send thread start\n";
    package->poll.Poll(INFINITE);
}

PollAction StreamController::SendData(Package* package, const Event* e)
{
    if (package->status == Status::SENDING)
    {
        LOG_ERROR << "Still sending\n";
        throw std::exception("Still sending");
    }
    package->status = Status::SENDING;
    MessageHeader header;
    header.code = Message::MSG_FRAME;
    BufferStream1KB stream;
    stream << header;
    WsaSocket& sock = package->socket;
    if (sock.SendAll(stream.Get(), stream.Length()) < stream.Length())
    {
        LOG_ERROR << "Send frame header failed\n";
    }
    if (sock.SendAll(&m_buffer.get()[package->offset], package->length) < package->length)
    {
        LOG_ERROR << "Send frame failed\n";
    }
    package->status = Status::ILDE;
    return PollAction::NONE;
}

std::vector<uint32_t> StreamController::GetBytePerSocket()
{
    std::vector<uint32_t> res;
    for(auto i = m_packages.begin(); i != m_packages.end(); i++)
    {
        res.push_back(i->length);
    }
    return res;
}
