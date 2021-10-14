#include <exception>
#include <chrono>
#include "Message.hh"

MessageHeader CreateHeaderMsg(Message code)
{
    MessageHeader header;
    header.code = code;
    header.reserved = 0;
    header.timeSinceEpoch = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return header;
}