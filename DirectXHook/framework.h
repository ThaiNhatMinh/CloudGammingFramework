#pragma once
#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <d3d11.h>
#include <d3d9.h>
#include <dxgi.h>
#include <dxgi1_2.h>

#include "detours.h"
#include "Logger.hh"
#include "DirectX.h"

#define DllExport _declspec( dllexport )
