#pragma once
#include "Win32.hh"
#include "common/AutoClose.hh"

typedef AutoClose<HANDLE, CloseHandle> AutoCloseHandle;
typedef AutoClose<LPCVOID, UnmapViewOfFile> AutoCloseMapView;