extern "C" char g_savedWindowLocationPilot[]; // Shared scratch 2020d770.
extern void* g_mainWindowCommandCallbacksPilot; // 2020bb70
extern void __stdcall DispatchCallbackListSourcePilot(void**, unsigned int, unsigned int);

// 2012e130, export 11. The synthetic WM_SIZE is posted in both modes.
extern "C" int __cdecl RequestMainWindowDefaultSizePilot()
{
    RECT bounds = { 0, 0, 1024, 720 };
    if (!g_idleControllerPilot->QueryField38()) {
        DWORD exStyle = GetWindowLongA(g_hookMainWindowPilot, GWL_EXSTYLE);
        BOOL hasMenu = GetMenu(g_hookMainWindowPilot) != NULL;
        DWORD style = GetWindowLongA(g_hookMainWindowPilot, GWL_STYLE);
        AdjustWindowRectEx(&bounds, style, hasMenu, exStyle);
        SetWindowPos(g_hookMainWindowPilot, NULL, 0, 0, bounds.right - bounds.left,
            bounds.bottom - bounds.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
    PostMessageA(g_hookMainWindowPilot, WM_SIZE, 0, 0);
    return 0;
}

// 2012fded. The original receives the command in ECX and controller in ESI.
static bool PreprocessMainWindowCommandPilot(
    IdleControllerBoundaryPilot* controller, WORD command, LPARAM lParam)
{
    unsigned int resource;
    switch (command) {
    case 0x1103:
        if (GetKeyState(VK_SHIFT) < 0) RequestMainWindowDefaultSizePilot();
        return false;
    case 0x110a:
        if (g_mainWindowEventPilot != NULL) g_mainWindowEventPilot(0x100c4, lParam, 0);
        return true;
    case 0x1601: resource = 0xf60c; break;
    case 0x1603: resource = 0xf60b; break;
    case 0x1604: resource = 0xf60d; break;
    case 0x166a: {
        char target[0x824];
        LoadSplashStringPilot(0xf609, target, sizeof(target));
        g_savedWindowLocationPilot[0] = '\0';
        if (QueryProductTextSourcePilot(0, 0x102, g_savedWindowLocationPilot, MAX_PATH) >= 0 &&
            g_savedWindowLocationPilot[0] != '\0') {
            char* suffix = static_cast<char*>(memchr(target, '\0', sizeof(target)));
            if (suffix != NULL)
                CopyPathSourceBoundaryPilot(g_savedWindowLocationPilot, suffix, sizeof(target) - (suffix - target));
        }
        IdleControllerBoundaryPilot* receiver = g_idleControllerPilot;
        typedef HINSTANCE (__thiscall *LaunchCommandTargetPilot)(
            void*, HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, int);
        GraphicsClipSlotPilot<LaunchCommandTargetPilot>(receiver, 0x64)(
            receiver, ReadMainWindowSizingStatePilot(controller).window, NULL, target, NULL, NULL, SW_SHOWNORMAL);
        return true;
    }
    default:
        return false;
    }
    LoadSplashStringPilot(resource, g_savedWindowLocationPilot, MAX_PATH);
    DispatchCallbackListSourcePilot(&g_mainWindowCommandCallbacksPilot, 400,
        reinterpret_cast<unsigned int>(g_savedWindowLocationPilot));
    return true;
}

// 2012e96f. Original ECX holds packed WPARAM, EDX the controller.
extern "C" LRESULT __stdcall HandleMainWindowCommandPilot(
    IdleControllerBoundaryPilot* controller, WPARAM wParam, LPARAM lParam)
{
    WPARAM command = wParam & 0xffffff;
    if ((command >> 16) <= 1 && PreprocessMainWindowCommandPilot(controller, LOWORD(command), lParam))
        return 0;
    if (g_mainWindowEventPilot != NULL && g_mainWindowEventPilot(command, lParam, wParam >> 24) != 0)
        return 0;
    return DefWindowProcA(ReadMainWindowSizingStatePilot(controller).window, WM_COMMAND, command, lParam);
}
