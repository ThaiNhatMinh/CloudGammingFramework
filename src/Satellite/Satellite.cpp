#include "Satellite.hh"

bool Satellite::Connect(const std::string& ip, unsigned short port)
{
    if (!m_socket.Connect(ip, port))
    {
        return false;
    }
}