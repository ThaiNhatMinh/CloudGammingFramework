#pragma once
#include "CloudGammingFramework.hh"

struct InputEvent;
bool cgfClientConnect(const char* ip, unsigned short port);
bool cfgClientRequestGame(GameId id);
bool cgfClientSendEvent(InputEvent event);