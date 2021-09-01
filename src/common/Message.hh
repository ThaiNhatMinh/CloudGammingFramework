#pragma once
#include <string>
#include "cgf/cgf.hh"

enum Message
{
    MSG_INIT,
    MSG_INPUT,
    MSG_FRAME,
    MSG_RESOLUTION,
    MSG_REQUEST_RESOLUTION,
    MSG_START_GAME,
    MSG_START_GAME_RESP,
    MSG_STOP_GAME,
    MSG_GAME_PORT,
    MSG_RECONNECT
};

struct MessageHeader
{
    Message code;
};

/**
 * Header length
 * - Message code
 * - TBD
 */
constexpr std::size_t MSG_HEADER_LENGTH = sizeof(MessageHeader);
constexpr std::size_t MSG_INPUT_PACKAGE_SIZE = MSG_HEADER_LENGTH + sizeof(InputEvent);
constexpr std::size_t MSG_STARTGAME_PACKAGE_SIZE = MSG_HEADER_LENGTH + sizeof(GameId);

MessageHeader CreateHeaderMsg(Message code);
MessageHeader ParseHeaderMsg(const char* pBuffer);
void CreateInputMsg(char* pBuffer, std::size_t bufferLength, const InputEvent& event);
void CreateFrameMsg(char* pBuffer, std::size_t bufferLength, const void* pData, std::size_t dataLength);




