#pragma once
#include <string>
#include "cgf/cgf.hh"

extern const std::string LAUNCH_EVENT;
extern const std::string LAUNCH_EVENT_MEMORY;
extern const std::string POLL_PROCESS_EVENT;
extern const std::string DISCONNECT_TIMER;
extern const std::string SHUTDOWN_EVENT;

std::string CreateDisconnectString(ClientId client, GameId id);
std::string CreateShutdownString(ClientId client, GameId id);
std::string CreateGameInfoString(ClientId client, GameId id);
