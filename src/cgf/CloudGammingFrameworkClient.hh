#pragma once
#include "cgf.hh"

typedef void (*cgfResolutionfun)(unsigned int width, unsigned int height, unsigned char bpp);
typedef void (*cgfFramefun)(const char* pFrameData);

bool cgfClientInitialize(cgfResolutionfun resFunc, cgfFramefun frameFunc);
bool cgfClientConnect(ClientId id, const char* ip, unsigned short port);
bool cfgClientRequestGame(GameId id);
bool cfgClientCloseGame();
bool cgfClientSendEvent(InputEvent event);
bool cgfClientPollEvent(int timeOut);
void cgfClientFinalize();