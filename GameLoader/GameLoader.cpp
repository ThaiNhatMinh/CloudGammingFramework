// GameLoader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <iostream>
#include <string>

#include "detours.h"

std::string GetDir(const std::string& filePath)
{
    std::size_t pos = filePath.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        return filePath.substr(0, pos);
    }
    return "";
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Missing parameters\n";
        return -1;
    }

    const char* dllLocation = argv[1];
    char path[MAX_PATH];
    std::string programLocation = argv[2];
    memcpy(path, programLocation.c_str(), programLocation.size() + 1);
    std::string directory = GetDir(programLocation);
    if (directory.size() == 0)
    {
        char dir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, dir);
        directory = dir;
    }

    const char* DLLs[1];
    DLLs[0] = dllLocation;
    char program[MAX_PATH];
    if (!GetFullPathName(programLocation.c_str(), MAX_PATH, program, NULL)) {
        return 9002;
    }

    std::cout << "Create process: " << program << " DLL: " << dllLocation << std::endl;
    std::cout << "Directory process: " << directory << std::endl;

    STARTUPINFO startupInfo = { sizeof(startupInfo) };
    memset(&startupInfo, 0, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION processInformation;
	bool ret = DetourCreateProcessWithDlls(NULL, program,
        NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, directory.c_str(),
        &startupInfo, &processInformation, 1, DLLs, NULL);
    if (!ret) {
        std::cout << "Game Start failed\n";
        auto error = GetLastError();
        std::cout << error << std::endl;
        return -1;
    }

    // Wait until child process exits.
    WaitForSingleObject(processInformation.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(processInformation.hProcess);
    CloseHandle(processInformation.hThread);
}