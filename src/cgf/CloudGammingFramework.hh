#pragma once
#include "cgf.hh"

enum GraphicApi
{
    DIRECTX_9,
    DIRECTX_10,
    DIRECTX_11,
    DIRECTX_12,
    OPENGL,
    VULKAN,
};

typedef void (*cgfCursorposfun)(double xpos, double ypos);
typedef void (*cgfMousebuttonfun)(Action action, MouseButton key);
typedef void (*cgfKeyPressfun)(Action action, Key key);
typedef void (*cgfCharfun)(InputEvent::CharType type, unsigned int ch);
typedef void (*cgfSCrollfun)(double xoffset, double yoffset);

typedef struct
{
    cgfCursorposfun CursorPositionCallback;
    cgfMousebuttonfun MouseButtonCallback;
    cgfKeyPressfun KeyPressCallback;
    cgfCharfun TextInputCallback;
    cgfSCrollfun ScrollCallback;
} InputCallback;

bool cgfRegisterGame(const char *gameName, GraphicApi type, InputCallback callback);
void cgfPollEvent();

/**
 * Get status of key
 *
 * @param key Key to check
 * @return Action::PRESSING if key is pressing, otherwise -1
 */
int cgfGetKeyStatus(Key key);

void cgfFinalize();

void cgfSetResolution(unsigned int width, unsigned int height, unsigned char bpp);

/**
 * 3 byte per pixel
 */
void cgfSetFrame(const void* pData);

/**
 * Return true if client quit game or client disconnect timeout
 */
bool cgfShouldExit();

// TODO: Move to C++ interface
