#pragma once
#include "Win32.hh"
#include "common/AutoClose.hh"

typedef AutoClose<HANDLE, CloseHandle, INVALID_HANDLE_VALUE> AutoCloseHandle;
typedef AutoClose<LPCVOID, UnmapViewOfFile, nullptr> AutoCloseMapView;