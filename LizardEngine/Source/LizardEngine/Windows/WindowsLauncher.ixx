module;

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

export module LizardEngine.Windows : WindowsLauncher;
import LizardEngine;

export class WindowsLauncher
{
public:
    static int Run(IApplication *Application, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHWND() { return hwnd; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND hwnd;
};

HWND WindowsLauncher::hwnd = nullptr;

int WindowsLauncher::Run(IApplication *Application, HINSTANCE hInstance, int nCmdShow)
{
    OutputDebugString(L"test");

    WNDCLASSEX windowClass = {0};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"IApplication";
    RegisterClassEx(&windowClass);

    // create the window and use the result as the handle
    hwnd = CreateWindowEx(0,
                          windowClass.lpszClassName, // name of the window class
                          windowClass.lpszClassName, // title of the window
                          WS_OVERLAPPEDWINDOW,       // window style
                          CW_USEDEFAULT,             // x-position of the window
                          CW_USEDEFAULT,             // y-position of the window
                          1920,                      // width of the window
                          1080,                      // height of the window
                          nullptr,                   // we have no parent window, NULL
                          nullptr,                   // we aren't using menus, NULL
                          hInstance,                 // application handle
                          Application);

    Application->Init();

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    Application->Quit();

    return static_cast<char>(msg.wParam);
}

LRESULT CALLBACK WindowsLauncher::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto app = reinterpret_cast<IApplication *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    switch (message)
    {
    case WM_CREATE:
        // Save the IApplication* passed in to CreateWindow.
        {
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
            return 0;
        }

    case WM_DESTROY:
        /* Quit */
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        /* Update */
        if (app)
        {
            /* code */
            app->Tick();
        }
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}