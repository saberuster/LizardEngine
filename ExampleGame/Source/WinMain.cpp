// include the basic windows header file
#include <windows.h>
#include <windowsx.h>
#pragma warning(disable : 5050)

import LizardEngine;
import LizardEngine.Windows;
import ExampleGame;
import std.filesystem;
import std.core;

namespace fs = std::filesystem;

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    Logger::Instance().PrintLog(L"{} on {}", L"game start");
    auto assetsPath = std::format(L"{}\\..\\..\\..\\ExampleGame\\", fs::current_path().wstring());
    Logger::Instance().PrintLog(L"asset path in : {}", assetsPath);
    auto game = new ExampleGame{std::move(assetsPath)};

    // return this part of the WM_QUIT message to Windows
    return WindowsLauncher::Run(game, hInstance, nCmdShow);
}
