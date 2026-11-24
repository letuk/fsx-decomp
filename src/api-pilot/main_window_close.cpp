#include <mmsystem.h>

extern int g_messageQuitRequestedPilot; // 2020e044
extern int g_idleUnthrottledPilot; // 2020e040
extern void (__cdecl *g_applicationIdleCallbackPilot)(); // 2020e02c
extern BOOL (WINAPI *g_unregisterDeviceNotificationPilot)(HDEVNOTIFY); // 2020e110
extern HDEVNOTIFY g_mainWindowDeviceNotificationPilot; // 2020e114
extern BOOL (WINAPI *g_mainWindowCloseHookPilot)(HWND); // 2020d014, WTS unregister
extern UINT g_mainWindowTimerIdPilot; // 2020e018
extern UINT g_mainWindowTimerPeriodPilot; // 2020e014
extern void (__cdecl *g_applicationCloseCallbackPilot)(); // 2020e038
extern DWORD g_applicationCloseCallbackEnabledPilot; // 2020e048
extern "C" void __cdecl CallDemoCloseOrdinal7Pilot(); // api.dll IAT 200e123c

C_ASSERT(offsetof(MainWindowSizingStatePilot, modalDepth) == 0x4c - 4);
C_ASSERT(offsetof(MainWindowSizingStatePilot, closeOverride) == 0x50 - 4);

// 2012edbb. The original controller is passed in ESI.
extern "C" LRESULT __stdcall CloseMainWindowPilot(
    IdleControllerBoundaryPilot* controller, WPARAM wParam, LPARAM lParam)
{
    MainWindowSizingStatePilot& state = ReadMainWindowSizingStatePilot(controller);
    if (state.modalDepth > 0 || (g_idleUnthrottledPilot != 0 && state.closeOverride == 0)) {
        CallDemoCloseOrdinal7Pilot();
        return TerminateProcess(GetCurrentProcess(), 0);
    }

    if (g_unregisterDeviceNotificationPilot != NULL && g_mainWindowDeviceNotificationPilot != NULL)
        g_unregisterDeviceNotificationPilot(g_mainWindowDeviceNotificationPilot);
    if (g_mainWindowCloseHookPilot != NULL) g_mainWindowCloseHookPilot(state.window);
    timeKillEvent(g_mainWindowTimerIdPilot);
    timeEndPeriod(g_mainWindowTimerPeriodPilot);
    bool alreadyQuitting = g_messageQuitRequestedPilot != 0;
    g_mainWindowEventPilot = NULL;
    g_applicationIdleCallbackPilot = NULL;
    if (!alreadyQuitting) {
        g_messageQuitRequestedPilot = 1;
        controller->Unknown24();
    }
    if (g_applicationCloseCallbackPilot != NULL && g_applicationCloseCallbackEnabledPilot != 0)
        g_applicationCloseCallbackPilot();
    return DefWindowProcA(state.window, WM_CLOSE, wParam, lParam);
}

// 2012df8e, export 4. Hide precedes the synchronous WM_CLOSE.
extern "C" int __cdecl SendMainWindowClosePilot()
{
    IdleControllerBoundaryPilot* controller = g_idleControllerPilot;
    g_messageQuitRequestedPilot = 1;
    controller->Unknown24();
    SendMessageA(g_hookMainWindowPilot, WM_CLOSE, 0, 0);
    return 1;
}

// 2012dfbd, export 5. The complete LPARAM is also the quit-flag condition.
extern "C" int __stdcall PostMainWindowCloseCommandPilot(LPARAM commandData)
{
    if (commandData != 0) g_messageQuitRequestedPilot = 1;
    PostMessageA(g_hookMainWindowPilot, WM_COMMAND, 0x110a, commandData);
    return 1;
}
