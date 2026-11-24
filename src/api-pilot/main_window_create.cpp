extern "C" void __cdecl InitializeSimPropSourcePilot(); // 200e13e0, simprop.dll
extern "C" void __cdecl StartMainWindowGraphicsSourcePilot(); // 200e1328, g2d.dll ordinal 1
extern "C" void __cdecl HandleMainWindowGraphicsFailureSourcePilot(); // 200e1324, g2d.dll ordinal 65
extern "C" void WINAPI InitializeStartupCommonControlsSourcePilot(); // 200e1c28, comctl32.dll ordinal 17
extern "C" void __cdecl ShutDownMainWindowServicesSourcePilot(); // 200f7c66
extern "C" void __cdecl ShutDownMainWindowGraphicsSourcePilot(); // 200e1320, g2d.dll ordinal 2
extern "C" void __cdecl ShutDownSimPropSourcePilot(); // 200e13dc, simprop.dll
extern "C" char g_startupWorkingDirectoryPilot[]; // 2020d360

typedef BOOL (WINAPI *RegisterStartupSessionPilot)(HWND, DWORD);
static RegisterStartupSessionPilot g_registerStartupSessionPilot; // 2020d01c
BOOL (WINAPI *g_mainWindowCloseHookPilot)(HWND); // 2020d014
HMODULE g_startupSessionModulePilot; // 2020de74

// Window creation and graphics startup phase of 2012205d.
extern "C" BOOL __stdcall CreateMainStartupWindowPilot(const RECT* bounds)
{
    InitializeSimPropSourcePilot();
    HWND hwnd = CreateWindowExA(WS_EX_APPWINDOW, "FS98MAIN", "", WS_OVERLAPPEDWINDOW,
        20000, 20000, bounds->right - bounds->left, bounds->bottom - bounds->top,
        NULL, NULL, g_mainWindowInstancePilot, NULL);
    // Both pilot names represent the original HWND at 2020e010.
    g_mainWindowHandlePilot = hwnd;
    g_hookMainWindowPilot = hwnd;
    if (hwnd == NULL) {
        ReportPanelWindowErrorPilot(0xf001, 0, 0);
        return FALSE;
    }

    IdleControllerBoundaryPilot* controller = g_idleControllerPilot;
    g_windowModeBoundaryPilot = reinterpret_cast<WindowModeBoundaryPilot*>(controller);
    typedef void (__thiscall *SetStartupBoundsPilot)(void*, RECT);
    GraphicsClipSlotPilot<SetStartupBoundsPilot>(controller, 0x3c)(controller, *bounds);
    StartMainWindowGraphicsSourcePilot();
    if (g_messageQuitRequestedPilot != 0) return FALSE;

    void* graphics = GetGraphicsControllerSourcePilot();
    typedef int (__thiscall *QueryStartupGraphicsPilot)(void*);
    if (GraphicsClipSlotPilot<QueryStartupGraphicsPilot>(graphics, 0x10)(graphics) == -1) {
        HandleMainWindowGraphicsFailureSourcePilot();
        return FALSE;
    }
    return TRUE;
}

// Completion phase of 2012205d, beginning at 20122587.
extern "C" BOOL __cdecl CompleteMainWindowStartupPilot()
{
    g_registerStartupSessionPilot = NULL;
    g_mainWindowCloseHookPilot = NULL;
    g_startupSessionModulePilot = LoadLibraryA("WTSAPI32.DLL");
    if (g_startupSessionModulePilot != NULL) {
        g_registerStartupSessionPilot = reinterpret_cast<RegisterStartupSessionPilot>(
            GetProcAddress(g_startupSessionModulePilot, "WTSRegisterSessionNotification"));
        g_mainWindowCloseHookPilot = reinterpret_cast<BOOL (WINAPI *)(HWND)>(
            GetProcAddress(g_startupSessionModulePilot, "WTSUnRegisterSessionNotification"));
    }
    if (g_registerStartupSessionPilot != NULL && g_mainWindowCloseHookPilot != NULL)
        g_registerStartupSessionPilot(g_mainWindowHandlePilot, 0);

    InitializeStartupCommonControlsSourcePilot();
    g_keyboardHookPilot = SetWindowsHookExA(WH_KEYBOARD, MainKeyboardHookPilot, NULL, GetCurrentThreadId());
    return TRUE;
}

// 2012260f. The close handler owns WTS session unregistration.
extern "C" void __cdecl ShutDownApplicationModulesPilot()
{
    UnhookWindowsHookEx(g_keyboardHookPilot);
    UnregisterClassA("FS98MAIN", g_mainWindowInstancePilot);
    UnregisterClassA("FS98CHILD", g_mainWindowInstancePilot);
    UnregisterClassA("FS98SHADOW", g_mainWindowInstancePilot);
    UnregisterClassA("FS98DLG3D", g_mainWindowInstancePilot);
    ShutDownMainWindowServicesSourcePilot();
    ShutDownMainWindowGraphicsSourcePilot();
    ShutDownSimPropSourcePilot();
    if (g_languageModulePilot != NULL) {
        FreeLibrary(g_languageModulePilot);
        g_languageModulePilot = NULL;
    }
    if (g_startupSessionModulePilot != NULL) {
        FreeLibrary(g_startupSessionModulePilot);
        g_startupSessionModulePilot = NULL;
    }
    SetCurrentDirectoryA(g_startupWorkingDirectoryPilot);
    ManageFontResourcesPilot(0);
}
