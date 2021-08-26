#include <exception>
#include "common/Logger.hh"
#include "common/Module.hh"
#include "ipc/Named.hh"
#include "GameParameter.hh"
#include "GameInstance.hh"
#include "Sun.hh"

const std::string PORT_RANG_START = "PortRangeStart";
const std::string PORT_RANG_END = "PortRangeEnd";
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

    std::size_t startPort = config->GetValue(PORT_RANG_START, 1000);
    std::size_t endPort = config->GetValue(PORT_RANG_END, 1000);
    for (std::size_t i = startPort; i < endPort; i++)
    {
        m_gameInstances[i].Id = INVALID_GAMEID;
    }

    LOG_DEBUG << "Init done\n";
}

bool Sun::LaunchGame(GameId id)
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
        return false;
    }
    StreamPort port = FindFreePort();
    if (port == INVALID_PORT)
    {
        LOG_ERROR << "No free port found" << std::endl;
        return false;
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
    bool ret = CreateProcess(NULL, program,
        NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE, NULL, directory.c_str(),
        &startupInfo, &processInformation);
    if (!ret) {
        LOG_ERROR << "Game Start failed: ";
        LastError();
        return false;
    }

    if (!m_launchGame.Wait(gameParam->RegisterTimeOut))
    {
        LOG_ERROR << "Waiting for register timeout\n";
        return false;
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
        return false;
    }

    m_launchGame.Signal();
    GameInstance instance;
    instance.Id = gameParam->Id;
    instance.ProcessHandle = processInformation.hProcess;
    m_gameInstances[port] = instance;

    return true;
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