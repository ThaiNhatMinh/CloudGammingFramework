#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <d3d11.h>
#include <d3d9.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include <dxgi.h>
#include <dxgi1_2.h>

#include "detours.h"
#include "Logger.hh"
#include "DllMain.h"
#include "Module.hh"
#include "RenderStream.hh"
#include "DXGI.hh"

#define DllExport _declspec( dllexport )
