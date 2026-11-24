// 200e1514 and 200e1388. The exporting modules own their state.
extern "C" void __stdcall SetWindowLayerModalBoundaryPilot(DWORD); // window.dll ordinal 5
extern "C" BOOL __cdecl QueryMainModalExitBoundaryPilot(); // main.dll ordinal 20

C_ASSERT(offsetof(MainWindowSizingStatePilot, modalDepth) == 0x4c - 4);
C_ASSERT(offsetof(MainWindowSizingStatePilot, closeOverride) == 0x50 - 4);
C_ASSERT(offsetof(MainWindowSizingStatePilot, modalSavedFocus) == 0x54 - 4);

// 201250ee. Enumeration continues for every window on the thread.
static BOOL CALLBACK HideModalFloatingWindowPilot(HWND window, LPARAM)
{
    char name[256] = {0};
    GetClassNameA(window, name, sizeof(name));
    if (_stricmp(name, "FS98FLOAT") == 0) ShowWindow(window, SW_HIDE);
    return TRUE;
}

static void FillModalClientAreaPilot(HWND window)
{
    HDC dc = GetDCEx(window, NULL, DCX_CACHE | DCX_CLIPSIBLINGS);
    RECT bounds = {0};
    GetClientRect(window, &bounds);
    FillRect(dc, &bounds, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
    ReleaseDC(window, dc);
}

// 20125883. G2D supplies the indexed records and HWNDs for the clip-refresh path.
static void ClearModalWindowSurfacesPilot(IdleControllerBoundaryPilot* controller)
{
    if (!controller->QueryClipRegionRefresh()) {
        FillModalClientAreaPilot(ReadMainWindowSizingStatePilot(controller).window);
        return;
    }

    void* graphics = GetGraphicsControllerSourcePilot();
    DWORD count = GraphicsClipSlotPilot<GraphicsClipIndexPilot>(graphics, 0x0c)(graphics);
    for (DWORD index = 0; index < count; ++index) {
        graphics = GetGraphicsControllerSourcePilot();
        void* surface = GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0)(graphics, index);
        if (GraphicsClipSlotPilot<GraphicsClipFlagPilot>(surface, 0x40)(surface)) {
            graphics = GetGraphicsControllerSourcePilot();
            typedef HWND (__thiscall *ReadGraphicsWindowPilot)(void*, DWORD);
            HWND window = GraphicsClipSlotPilot<ReadGraphicsWindowPilot>(graphics, 4)(graphics, index);
            FillModalClientAreaPilot(window);
        }
    }
}

// 20125979, controller slot +0x4c.
void IdleControllerBoundaryPilot::EnterModalWindowPilot()
{
    MainWindowSizingStatePilot& state = ReadMainWindowSizingStatePilot(this);
    state.modalDepth = static_cast<LONG>(static_cast<DWORD>(state.modalDepth) + 1);
    if (state.modalDepth != 1) return;
    state.modalSavedFocus = GetFocus();
    SetWindowLayerModalBoundaryPilot(1);
    EnumThreadWindows(GetCurrentThreadId(), HideModalFloatingWindowPilot, 0);
    SetDialogMode(2, true);
    if (g_mainWindowEventPilot) g_mainWindowEventPilot(0x20004, 0, 0);
    ClearModalWindowSurfacesPilot(this);
}

// 201259d8, controller slot +0x50.
void IdleControllerBoundaryPilot::LeaveModalWindowPilot()
{
    MainWindowSizingStatePilot& state = ReadMainWindowSizingStatePilot(this);
    state.modalDepth = static_cast<LONG>(static_cast<DWORD>(state.modalDepth) - 1);
    if (state.modalDepth != 0) return;
    if (QueryMainModalExitBoundaryPilot()) SetWindowLayerModalBoundaryPilot(0);
    if (g_mainWindowEventPilot) g_mainWindowEventPilot(0x20005, 0, 0);
    SetDialogMode(2, false);
    EnumThreadWindows(GetCurrentThreadId(), RefreshFloatingStartupWindowPilot, 0);
    if (state.modalSavedFocus && IsWindow(state.modalSavedFocus) && IsWindowVisible(state.modalSavedFocus))
        SetFocus(state.modalSavedFocus);
    state.modalSavedFocus = NULL;
}

// 20125a5d and 20125a61, controller slots +0x54 and +0x58.
BYTE IdleControllerBoundaryPilot::ReadModalWindowFlagPilot()
{
    return ReadMainWindowSizingStatePilot(this).modalWindowFlag;
}

void IdleControllerBoundaryPilot::SetModalWindowFlagPilot(BYTE enabled)
{
    if (enabled) EnterModalWindowPilot();
    else LeaveModalWindowPilot();
    ReadMainWindowSizingStatePilot(this).modalWindowFlag = enabled;
}

// 20125a8f, controller slot +0x48. The DWORD also feeds the WM_CLOSE gate.
void IdleControllerBoundaryPilot::SetCloseOverridePilot(DWORD enabled)
{
    MainWindowSizingStatePilot& state = ReadMainWindowSizingStatePilot(this);
    state.closeOverride = enabled;
    if (enabled) EnableWindow(state.window, TRUE);
    LONG style = GetWindowLongA(state.window, GWL_STYLE);
    SetWindowLongA(state.window, GWL_STYLE, enabled ? style & ~WS_MAXIMIZEBOX : style | WS_MAXIMIZEBOX);
}
