#include <windows.h>
extern "C" HWND g_startupWindowPilot;
extern "C" HINSTANCE g_startupInstancePilot;
extern "C" HMONITOR g_startupMonitorPilot;
extern "C" LRESULT CALLBACK StartupWindowProcedurePilot(HWND, UINT, WPARAM, LPARAM);
extern "C" DWORD WINAPI StartupWindowThreadPilot(void* parameter)
{
    RECT bounds;
    MSG message;
    WNDCLASSA windowClass = {0};
    windowClass.style = 0x40;
    windowClass.lpfnWndProc = StartupWindowProcedurePilot;
    windowClass.hInstance = g_startupInstancePilot;
    windowClass.hCursor = LoadCursorA(NULL, MAKEINTRESOURCEA(0x7f02));
    windowClass.hbrBackground = NULL;
    windowClass.lpszClassName = "FlightSimSplash";
    RegisterClassA(&windowClass);
    g_startupWindowPilot = CreateWindowExA(0x08000000, "FlightSimSplash", NULL,
        0, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, GetDesktopWindow(),
        NULL, g_startupInstancePilot, NULL);
    if (g_startupWindowPilot != NULL) {
        SetWindowLongA(g_startupWindowPilot, GWL_STYLE, static_cast<LONG>(WS_POPUP));
        GetWindowRect(g_startupWindowPilot, &bounds);
        g_startupMonitorPilot = MonitorFromRect(&bounds, 2);
        if (SetEvent(*static_cast<HANDLE*>(parameter))) {
            while (GetMessageA(&message, NULL, 0, 0) > 0) {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
            if (g_startupWindowPilot != NULL && IsWindow(g_startupWindowPilot))
                DestroyWindow(g_startupWindowPilot);
            return 0;
        }
    }
    return 1;
}
