// include the basic windows header file
#include <windows.h>
#include <windowsx.h>
#include "LizardEngine.h"
#include "Game.h"
#include "Logger.h"

using namespace LizardEngine;

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    Logger::Instance().PrintLog(L"{}", L"game start");

    auto game = new GameApp{};

    // return this part of the WM_QUIT message to Windows
    return LizardEngine::WindowsLauncher::Run(game, hInstance, nCmdShow);
}
