#pragma once
#include "cgf.hh"

bool cgfClientConnect(const char* ip, unsigned short port);
bool cfgClientRequestGame(GameId id);
bool cgfClientSendEvent(InputEvent event);
void cgfClientFinalize();