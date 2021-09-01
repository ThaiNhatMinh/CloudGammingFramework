#include <exception>
#include "common/Logger.hh"
#include "common/Message.hh"
#include "common/Module.hh"
#include "ipc/Named.hh"
#include "GameParameter.hh"
#include "GameInstance.hh"
#include "Sun.hh"

const std::string PORT_RANG_START = "PortRangeStart";
const std::string PORT_RANG_END = "PortRangeEnd";
const std::string PORT = "Port";

std::string GetDir(const std::string& filePath)
{
    std::size_t pos = filePath.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        return filePath.substr(0, pos);
    }
    return "";
}

Sun::Sun(Configuration* config, const std::vector<GameParameter>& gamedb): m_pConfig(config), m_gameDb(gamedb)
{
    if (!m_launchGame.Create(LAUNCH_EVENT) || !m_launchData.Create(LAUNCH_EVENT_MEMORY, sizeof(GameRegister)))
    {
        throw std::exception("Create failed");
    }
    WsaSocket::Init();

    unsigned short port = config->GetValue(PORT, 8901);
    if (!m_socket.Open(port))
    {
        throw std::exception("Listen on port failed");
    }

    AddSocket(m_socket);
    std::size_t startPort = config->GetValue(PORT_RANG_START, 1000);
    std::size_t endPort = config->GetValue(PORT_RANG_END, 1000);
    for (std::size_t i = startPort; i < endPort; i++)
    {
        m_gameInstances[i].Id = INVALID_GAMEID;
    }

    LOG_DEBUG << "Init done\n";
}

StreamPort Sun::LaunchGame(GameId id)
{
    /**
     * 1. Get game information from id
     * 2. CreateProcess
     * 3. Waiting for game register
     * 4. Get free port and response back to game
     */
    const GameParameter* gameParam = FindGame(id);
    if (gameParam == nullptr)
    {
        LOG_ERROR << "Game with id " << id << " not found" << std::endl;
        return INVALID_PORT;
    }
    StreamPort port = FindFreePort();
    if (port == INVALID_PORT)
    {
        LOG_ERROR << "No free port found" << std::endl;
        return INVALID_PORT;
    }

    STARTUPINFO startupInfo = { sizeof(startupInfo) };
    memset(&startupInfo, 0, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION processInformation;
    char program[MAX_PATH] = {0};
    std::memcpy(program, gameParam->ExecuteLocation.c_str(), gameParam->ExecuteLocation.length());
    std::string directory = GetDir(program);
    if (directory.size() == 0)
    {
        char dir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, dir);
        directory = dir;
    }
    int mod = CREATE_DEFAULT_ERROR_MODE;
    mod |= CREATE_NEW_CONSOLE;
    bool ret = CreateProcess(NULL, program,
        NULL, NULL, FALSE, mod, NULL, directory.c_str(),
        &startupInfo, &processInformation);
    if (!ret) {
        LOG_ERROR << "Game Start failed: ";
        LastError();
        return INVALID_PORT;
    }
    if (WaitForInputIdle(processInformation.hProcess, 500) != 0)
    {
        LOG_ERROR << "Waiting for process idle failed\n";
    }

    if (!m_launchGame.Wait(gameParam->RegisterTimeOut))
    {
        LOG_ERROR << "Waiting for register timeout\n";
        return INVALID_PORT;
    }
    GameRegister info;
    info.port = port;
    if (!m_launchData.Write(&info, sizeof(GameRegister)))
    {
        LOG_ERROR << "Failed to send register information\n";
        if (!TerminateProcess(processInformation.hProcess, -1))
        {
            LOG_ERROR << "Failed to terminate process:" << std::endl;
            LastError();
        }
        return INVALID_PORT;
    }

    m_launchGame.Signal();
    GameInstance instance;
    instance.Id = gameParam->Id;
    instance.ProcessHandle = processInformation.hProcess;
    instance.status = GameStatus::STARTING;
    m_gameInstances[port] = instance;

    return port;
}

const GameParameter* Sun::FindGame(GameId id)
{
    for (auto& param : m_gameDb)
    {
        if (param.Id == id)
        {
            return &param;
        }
    }
    return nullptr;
}

StreamPort Sun::FindFreePort()
{
    for (auto& pair : m_gameInstances)
    {
        if (pair.second.Id == INVALID_GAMEID)
        {
            return pair.first;
        }
    }

    return INVALID_PORT;
}

void Sun::OnAccept(WsaSocket&& newConnect)
{
    // TODO: Start timer to waiting for MSG_INIT
    m_clients.emplace_back(std::move(newConnect), INVALID_CLIENTID);
    AddSocket(m_clients.back().first, nullptr, static_cast<SocketCallback>(&Sun::OnRecvFromClient));
}

void Sun::OnClose(WsaSocketInformation* sock)
{
    for (auto iter = m_clients.begin(); iter != m_clients.end(); iter++)
    {
        if (iter->first.GetHandle() == sock->socket->GetHandle())
        {
            m_clients.erase(iter);
            break;
        }
    }
}

void Sun::OnRecvFromClient(WsaSocketInformation* sock)
{
    if (sock->recvBuffer.Length() < MSG_HEADER_LENGTH) return;
    sock->recvBuffer.SetCurrentPosition(0);
    MessageHeader header;
    sock->recvBuffer >> header;
    if (header.code == Message::MSG_START_GAME && sock->recvBuffer.Length() >= sizeof(GameId))
    {
        GameId id;
        sock->recvBuffer >> id;
        LaunchGame(sock->socket, id);
    } else if (header.code == Message::MSG_INIT && sock->recvBuffer.Length() >= sizeof(ClientId))
    {
        ClientId id;
        sock->recvBuffer >> id;
        for (auto& el : m_clients)
        {
            if (el.first == *sock->socket)
            {
                el.second = id;
                break;
            }
        }
    } else
    {
        LOG_ERROR << "Unknow code: " << header.code << std::endl;
    }
}

void Sun::LaunchGame(const WsaSocket* client, GameId id)
{
    std::unique_lock<std::mutex> lock(m_lockLaunchGame);
    LOG_DEBUG << "Launching game " << id << std::endl;
    ClientId clientId = FindClient(client);
    StreamPort port = FindExistRunningGame(clientId);
    if (port == INVALID_PORT)
    {
        port = LaunchGame(id);
    } else if (m_gameInstances[port].Id != id)
    {
        LOG_ERROR << "Client request game: " << id << " but current running game: " << m_gameInstances[port].Id << std::endl;
    }
    BufferStream1KB stream;
    MessageHeader header;
    header.code = Message::MSG_START_GAME_RESP;
    stream << header;
    StreamPort status;
    if (port == INVALID_PORT)
    {
        status = INVALID_PORT;
        stream << status;
    } else
    {
        status = 1;
        stream << port;
        m_gameInstances[port].clientId = clientId;
    }

    if (!client->SendAll(stream.Get(), stream.Length()))
    {
        LOG_ERROR << "Send all failed\n";
    }
}

ClientId Sun::FindClient(const WsaSocket* client)
{
    for (auto& el : m_clients)
    {
        if (el.first == *client)
        {
            return el.second;
        }
    }
    return INVALID_CLIENTID;
}

StreamPort Sun::FindExistRunningGame(ClientId clientId)
{
    for(auto& el : m_gameInstances)
    {
        if (el.second.clientId == clientId)
        {
            return el.first;
        }
    }

    return INVALID_PORT;
}

