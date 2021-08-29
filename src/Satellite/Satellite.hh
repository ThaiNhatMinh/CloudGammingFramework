#include <string>
#include "cgf/CloudGammingFramework.hh"
#include "ipc/WsaSocket.hh"

class Satellite
{
private:
    WsaSocket m_socket;
public:
    bool Connect(const std::string& ip, unsigned short port);
};