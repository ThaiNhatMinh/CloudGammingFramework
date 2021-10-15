#pragma once

typedef void(*frameProcessFnc)(const void* pFrameData);

void cgfCaptureOpenglInit(int width, int height);
void cgfCaptureOpenglFrame(frameProcessFnc fnc);
void cgfCaptureOpenglRelease();

unsigned int cgfOpenglInit(int width, int height);
void cgfUpdateTexture(const void* pData);
void cgfOpenglRelease();
