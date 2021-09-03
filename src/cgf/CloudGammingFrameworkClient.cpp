#include "CloudGammingFrameworkClient.hh"
#include "Satellite/Satellite.hh"

static Satellite meowww;

bool cgfClientInitialize(cgfResolutionfun resFunc, cgfFramefun frameFunc)
{
    if (resFunc == nullptr || frameFunc == nullptr) return false;
    return meowww.Initialize(resFunc, frameFunc);
}

bool cgfClientConnect(ClientId id, const char* ip, unsigned short port)
{
    return meowww.Connect(id, ip, port);
}

bool cfgClientRequestGame(GameId id)
{
    return meowww.RequestGame(id);
}

bool cgfClientSendEvent(InputEvent event)
{
    return meowww.SendInput(event);
}

bool cgfClientPollEvent(int timeOut)
{
    std::size_t myTimeOut = timeOut;
    if (timeOut == -1) myTimeOut = INFINITE;
    return meowww.PollEvent(myTimeOut);
}

void cgfClientFinalize()
{
    meowww.Finalize();
}