// include the basic windows header file
#include <windows.h>
#include <windowsx.h>

import LizardEngine.Common;
import LizardEngine.Windows;
import ExampleGame;

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    Logger::Instance().PrintLog(L"{}", L"game start");

    auto game = new ExampleGame{};

    // return this part of the WM_QUIT message to Windows
    return WindowsLauncher::Run(game, hInstance, nCmdShow);
}
