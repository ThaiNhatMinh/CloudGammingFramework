#pragma once
#include "Module.hh"
#include "common/AutoClose.hh"

typedef AutoClose<HANDLE, CloseHandle> AutoCloseHandle;
typedef AutoClose<LPCVOID, UnmapViewOfFile> AutoCloseMapView;