#include <sstream>
#include "Named.hh"

const std::string LAUNCH_EVENT = "Local\\LaunchEvent";
const std::string LAUNCH_EVENT_MEMORY = "Local\\LaunchEventMemory";
const std::string POLL_PROCESS_EVENT = "Local\\PollProcess";
const std::string DISCONNECT_TIMER = "Local\\Timer";
const std::string STREAMCONTROLLER_EVENT = "Local\\StreamController";
const std::string STOPOLL_EVENT = "Local\\StopPoll";
const std::string SENDFRAME_EVENT = "Local\\SendFrame";


std::string CreateGameInfoString(ClientId client, GameId id)
{
    std::stringstream ss;
    ss << "Local\\GameInfo_" << client << '_' << id;
    return ss.str();
}

std::string CreateDisconnectString(ClientId client, GameId id)
{
    std::stringstream ss;
    ss << "Local\\Disconnect_" << client << '_' << id;
    return ss.str();
}

std::string CreateShutdownString(ClientId client, GameId id)
{
    std::stringstream ss;
    ss << "Local\\ShutdownGame_" << client << '_' << id;
    return ss.str();
}

std::string CreateStreamControllerString(void* ptr)
{
    std::stringstream ss;
    ss << STREAMCONTROLLER_EVENT << ptr;
    return ss.str();
}

std::string CreateStopPollString(void *ptr)
{
    std::stringstream ss;
    ss << STOPOLL_EVENT << ptr;
    return ss.str();
}