// include the basic windows header file
#include <windows.h>
#include <windowsx.h>
#include "LizardEngine.h"
#include "Game.h"

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{

    OutputDebugString(L"start game...");
    auto game = new GameApp{};

    // return this part of the WM_QUIT message to Windows
    return LizardEngine::WindowsLauncher::Run(game, hInstance, nCmdShow);
}
