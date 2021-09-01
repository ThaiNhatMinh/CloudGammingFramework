#include "Satellite/Satellite.hh"

static Satellite meowww;

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

void cgfClientFinalize()
{
    meowww.Finalize();
}