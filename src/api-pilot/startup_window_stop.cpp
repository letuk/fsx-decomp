#include <windows.h>
extern "C" HWND g_startupWindowPilot;
extern "C" HANDLE g_startupThreadPilot;
extern "C" HINSTANCE g_startupInstancePilot;
extern "C" void __cdecl StopStartupWindowPilot()
{
    if (g_startupWindowPilot != NULL && IsWindow(g_startupWindowPilot)) {
        PostMessageA(g_startupWindowPilot, WM_CLOSE, 0, 0);
        WaitForSingleObject(g_startupThreadPilot, INFINITE);
    }
    if (g_startupThreadPilot != NULL) {
        CloseHandle(g_startupThreadPilot);
        g_startupThreadPilot = NULL;
    }
    UnregisterClassA("FlightSimSplash", g_startupInstancePilot);
}
