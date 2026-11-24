#include "application_boundary.h"
#include "window_controller.h"
// Included after main_keyboard_hook.cpp: shared observed mode interface/global.
// Original globals, names reconstructed; storage and final linking remain open.
extern HHOOK g_foregroundIdleHookPilot;              // 2020d010
extern int g_idleHookEnabledPilot;                  // 2020e04c
extern int g_legacyWindowsPlatformPilot;             // 201f9954, initially -1
extern int g_messageStartupModePilot;               // 201f9500
extern int g_messageQuitRequestedPilot;             // 2020e044
extern unsigned int g_idleIterationCountPilot;      // 2020e2e8
extern int g_idleReentrancyPilot;                   // 2020e2e4
extern int g_idleUnthrottledPilot;                  // 2020e040
extern unsigned int g_idleClockFlagsPilot;          // 2020e674
extern DWORD g_idleLastTickPilot;                   // 2020e670
extern int g_cursorSuppressedPilot;                 // 201f7160
extern int g_cursorSuppressionEnabledPilot;         // 201f7164
extern void (__cdecl *g_applicationIdleCallbackPilot)(); // 2020e02c
extern void* g_messageStartupCallbacksPilot;        // 2020bba8
extern void __stdcall DispatchCallbackListSourcePilot(void**, unsigned int, unsigned int); // 200f67bd
extern int __stdcall FilterApplicationMessagePilot(MSG*); // 2013a766
extern "C" __declspec(dllimport) int __stdcall UI_PreTranslateMessage(MSG*);
extern "C" __declspec(dllimport) bool __stdcall ImeUi_IgnoreHotKey(MSG*);
extern CurrentMessageCallbackBoundaryPilot g_currentMessageCallbackPilot;

// Original 2012294b. Reentrant result -1 remains distinct from no work (0).
extern "C" int __cdecl RunApplicationIdlePilot()
{
    if (g_idleReentrancyPilot != 0) return -1;
    g_idleReentrancyPilot = 1;
    bool dispatch = g_idleControllerPilot != NULL && g_idleControllerPilot->QueryActive();
    if (!dispatch) {
        dispatch = g_idleUnthrottledPilot != 0;
        if (!dispatch) {
            if ((g_idleClockFlagsPilot & 1) == 0) {
                g_idleClockFlagsPilot |= 1;
                g_idleLastTickPilot = GetTickCount();
            }
            if (GetTickCount() - g_idleLastTickPilot >= 32) {
                g_idleLastTickPilot = GetTickCount();
                dispatch = true;
            }
        }
    }
    int result = 0;
    if (dispatch && g_applicationIdleCallbackPilot != NULL) {
        g_applicationIdleCallbackPilot();
        result = 1;
    }
    --g_idleReentrancyPilot;
    if (g_windowModeBoundaryPilot->QueryCursorState() == 0 &&
        g_cursorSuppressedPilot == 0 && g_cursorSuppressionEnabledPilot != 0 &&
        GetCapture() == NULL) {
        g_cursorSuppressedPilot = -1;
        SetCursor(NULL);
    }
    return result;
}

// Original 20122a0b. EAX from CallNextHookEx survives cookie check and RET12.
extern "C" LRESULT CALLBACK ForegroundIdleHookPilot(int code, WPARAM wParam, LPARAM lParam)
{
    if (code == 0 && g_idleHookEnabledPilot != 0 && IsWindowEnabled(g_hookMainWindowPilot)) {
        MSG message;
        while (!PeekMessageA(&message, NULL, 0, 0, PM_NOREMOVE)) RunApplicationIdlePilot();
        if (g_legacyWindowsPlatformPilot == -1) {
            OSVERSIONINFOA version;
            version.dwOSVersionInfoSize = sizeof(version);
            GetVersionExA(&version);
            g_legacyWindowsPlatformPilot = version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;
        }
        if (g_legacyWindowsPlatformPilot == 0) PostMessageA(NULL, 0, 0, 0);
    }
    return CallNextHookEx(g_foregroundIdleHookPilot, code, wParam, lParam);
}

// Original 20122ae1..20122c49. No application replacement/runtime claim.
int __cdecl RunApplicationMessageLoopBodyPilot()
{
    g_foregroundIdleHookPilot = SetWindowsHookExA(WH_FOREGROUNDIDLE, ForegroundIdleHookPilot,
        NULL, GetCurrentThreadId());
    DispatchCallbackListSourcePilot(&g_messageStartupCallbacksPilot, 0, 0);
    if (g_messageStartupModePilot == 0) {
        g_messageQuitRequestedPilot = 1;
        PostMessageA(g_hookMainWindowPilot, WM_COMMAND, 0x110a, 1);
    }
    for (;;) {
        MSG localMessage;
        MSG* message = g_currentMessageCallbackPilot == NULL ? NULL : g_currentMessageCallbackPilot();
        if (message == NULL) message = &localMessage;
        while (PeekMessageA(message, NULL, 0, 0, PM_REMOVE)) {
            if (message->message == WM_QUIT) {
                ClipCursor(NULL);
                UnhookWindowsHookEx(g_foregroundIdleHookPilot);
                return 0;
            }
            if (FilterApplicationMessagePilot(message) == 0 &&
                UI_PreTranslateMessage(message) == 0 && !ImeUi_IgnoreHotKey(message)) {
                TranslateMessage(message);
                DispatchMessageA(message);
            }
        }
        if (g_messageQuitRequestedPilot != 0) PostQuitMessage(0);
        if (RunApplicationIdlePilot() == 0) Sleep(20);
        else if (!g_windowModeBoundaryPilot->QueryBusy()) Sleep(0);
        if (!g_windowModeBoundaryPilot->QueryBusy() && ++g_idleIterationCountPilot > 30) {
            g_windowModeBoundaryPilot->IdleMaintenance();
            g_idleIterationCountPilot = 0;
        }
    }
}
