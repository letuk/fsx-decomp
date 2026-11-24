extern DWORD g_applicationReadyPilot; // 2020dea0
extern void (WINAPI *g_applicationReadyCallbackPilot)(int); // 2020e034
extern int g_timerCursorGatePilot; // 2020e03c
extern int g_cursorSuppressedPilot; // 201f7160

struct ApplicationVersionStatePilot {
    DWORD size, major, minor, build, extra;
};
extern ApplicationVersionStatePilot g_applicationVersionStatePilot; // 2020e0c8
extern void __cdecl OnStartupVersionEventPilot(ULONG_PTR, ULONG_PTR); // 2013e44a

// 2013e4d4. Reuse the owner also used by the Initialize wrapper.
static void PublishStartupVersionPilot()
{
    g_applicationVersionStatePilot.size = 0x48;
    g_applicationVersionStatePilot.major = 10;
    g_applicationVersionStatePilot.minor = 0;
    g_applicationVersionStatePilot.build = 0xf0c5;
    g_applicationVersionStatePilot.extra = 0;
    g_initializeCallbackRegistryBoundaryPilot.Register(&OnStartupVersionEventPilot, 0, 0, 0);
}

// 201228f0. The original uses RET 0x14, matching the multimedia callback.
extern "C" void CALLBACK MainWindowMultimediaTimerPilot(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR)
{
    if (g_windowModeBoundaryPilot->QueryCursorState() <= 1 &&
        (g_timerActivityFirstPilot != NULL || g_timerActivitySecondPilot != NULL)) {
        PostMessageA(g_hookMainWindowPilot, 0x465, 0, 0);
    }
    if (g_timerCursorGatePilot == 0 && g_cursorSuppressedPilot != 0 && g_cursorSuppressedPilot != -1)
        g_cursorSuppressedPilot = static_cast<int>(static_cast<DWORD>(g_cursorSuppressedPilot) - 1);
}

// 20120d8d. Called after the original pre-readiness cancellation check.
extern "C" int __cdecl FinishApplicationStartupPilot()
{
    g_applicationReadyPilot = 1;
    PublishStartupVersionPilot();
    g_applicationCloseCallbackEnabledPilot = 1;
    g_applicationReadyCallbackPilot(1);
    bool quitRequested = g_messageQuitRequestedPilot != 0;
    g_idleUnthrottledPilot = 0;
    if (quitRequested) return 1;

    if (!g_windowModeBoundaryPilot->QueryMode()) g_idleControllerPilot->Unknown28();
    UpdateWindow(g_hookMainWindowPilot);
    SetActiveWindow(g_hookMainWindowPilot);
    SetFocus(g_hookMainWindowPilot);

    TIMECAPS caps;
    bool timerSupported = timeGetDevCaps(&caps, sizeof(caps)) == TIMERR_NOERROR;
    if (timerSupported) {
        UINT period = caps.wPeriodMin > 50 ? caps.wPeriodMin : 50;
        if (period >= caps.wPeriodMax) period = caps.wPeriodMax;
        g_mainWindowTimerPeriodPilot = period;
        timerSupported = period == 50;
    }
    if (timerSupported) {
        timeBeginPeriod(50);
        g_mainWindowTimerIdPilot = timeSetEvent(50, 20, MainWindowMultimediaTimerPilot, 0, TIME_PERIODIC);
    } else {
        PostMainWindowCloseCommandPilot(1);
    }
    PostMessageA(g_hookMainWindowPilot, 0x46b, 0, 0);
    return 1;
}

static LRESULT HandleMainWindowTimerMessagePilot(
    IdleControllerBoundaryPilot* controller, WPARAM wParam, LPARAM lParam)
{
    ProcessMainWindowTimerWorkPilot();
    return DefWindowProcA(ReadMainWindowSizingStatePilot(controller).window, 0x465, wParam, lParam);
}

static BOOL CALLBACK RefreshFloatingStartupWindowPilot(HWND window, LPARAM)
{
    char name[256];
    GetClassNameA(window, name, sizeof(name));
    if (_stricmp(name, "FS98FLOAT") == 0) SendMessageA(window, 0x467, 0, 0);
    return TRUE;
}

// 2012f3e8 and visitor 20125085.
static LRESULT RefreshFloatingWindowsMessagePilot(
    IdleControllerBoundaryPilot* controller, WPARAM wParam, LPARAM lParam)
{
    EnumThreadWindows(GetCurrentThreadId(), RefreshFloatingStartupWindowPilot, 0);
    return DefWindowProcA(ReadMainWindowSizingStatePilot(controller).window, 0x46b, wParam, lParam);
}
