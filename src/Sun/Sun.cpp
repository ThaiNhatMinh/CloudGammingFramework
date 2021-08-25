#include <exception>
#include "common/Logger.hh"
#include "common/Module.hh"
#include "ipc/Named.hh"
#include "Sun.hh"

std::string GetDir(const std::string& filePath)
{
    std::size_t pos = filePath.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        return filePath.substr(0, pos);
    }
    return "";
}

Sun::Sun()
{
    if (!m_launchGame.Create(LAUNCH_EVENT) || !m_launchData.Create(LAUNCH_EVENT_MEMORY, sizeof(GameRegister)))
    {
        throw std::exception("Create failed");
    }
    LOG_DEBUG << "Init done\n";
}

bool Sun::LaunchGame(Sun::GameId id)
{
    /**
     * 1. Get game information from id
     * 2. CreateProcess
     * 3. Waiting for game register
     * 4. Get free port and response back to game
     */
    STARTUPINFO startupInfo = { sizeof(startupInfo) };
    memset(&startupInfo, 0, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION processInformation;
    char program[MAX_PATH] = {"D:\\Study\\CloudGaming\\CloudGamming\\build\\test\\Game\\Debug\\Console.exe"};
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
        LOG_ERROR << "Game Start failed\n";
        LastError();
        return false;
    }

    if (!m_launchGame.Wait(30*1000))
    {
        LOG_ERROR << "Waiting for register timeout\n";
        return false;
    }
    GameRegister info;
    info.port = 1234;
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

    return true;
}