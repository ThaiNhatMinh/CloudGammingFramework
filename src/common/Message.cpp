#include <exception>
#include "Message.hh"

MessageHeader CreateHeaderMsg(Message code)
{
    MessageHeader header;
    header.code = code;
    return header;
}

MessageHeader ParseHeaderMsg(const char* pBuffer)
{
    MessageHeader header;
    std::memcpy(&header, pBuffer, MSG_HEADER_LENGTH);
    return header;
}

void CreateInputMsg(char* pBuffer, std::size_t bufferLength, const InputEvent& event)
{
    if (bufferLength < sizeof(InputEvent) + MSG_HEADER_LENGTH)
        throw std::exception("Input buffer length is smaller than require");

    MessageHeader header = CreateHeaderMsg(Message::MSG_INPUT);
    std::memcpy(pBuffer, &header, MSG_HEADER_LENGTH);
    std::memcpy(pBuffer + MSG_HEADER_LENGTH, &event, sizeof(InputEvent));
}

void CreateFrameMsg(char* pBuffer, std::size_t bufferLength, const void* pData, std::size_t dataLength)
{
    if (bufferLength < dataLength + MSG_HEADER_LENGTH)
        throw std::exception("Input buffer length is smaller than require");

    MessageHeader header = CreateHeaderMsg(Message::MSG_FRAME);
    std::memcpy(pBuffer, &header, MSG_HEADER_LENGTH);
    std::memcpy(pBuffer + MSG_HEADER_LENGTH, pData, dataLength);
}