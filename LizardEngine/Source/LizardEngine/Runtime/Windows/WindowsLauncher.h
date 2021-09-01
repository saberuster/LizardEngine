#pragma once

#include "Windows.h"
#include "Runtime/Macros.h"

namespace LizardEngine
{
    class IApplication;

    class LIZARD_API WindowsLauncher
    {
    public:
        static int Run(IApplication *Application, HINSTANCE hInstance, int nCmdShow);
        static HWND GetHWND() { return hwnd; }

    protected:
        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    private:
        static HWND hwnd;
    };
}
