// Both 2020de9c and 2020de98 are published from the same constructor result.
extern IdleControllerBoundaryPilot* g_mainWindowControllerPilot; // 2020de9c
typedef int (WINAPI *MainWindowEventPilot)(DWORD, DWORD, DWORD);
extern MainWindowEventPilot g_mainWindowEventPilot; // 2020e030

// Observed controller fields starting at +4. Unknown bytes are kept opaque.
struct MainWindowSizingStatePilot {
    HWND window;
    RECT clientBounds;
    BYTE unknown18[0x10];
    HBITMAP snapshot;
    LONG snapshotWidth;
    LONG snapshotHeight;
    LONG controllerOperationDepth;
    BYTE modeCallbacksEnabled;
    BYTE unknown39[3];
    BYTE graphicsTransitionFlag;
    BYTE inSizeMove;
    BYTE modalWindowFlag;
    BYTE unknown3f;
    int allowRemoteSession;
    BYTE unknown44[8];
    LONG modalDepth;
    DWORD closeOverride;
    HWND modalSavedFocus;
    DWORD unknown58;
    DWORD unknown5c;
    void* ownedBlock;
    DWORD unknown64;
    DWORD unknown68;
};
C_ASSERT(offsetof(MainWindowSizingStatePilot, snapshot) == 0x28 - 4);
C_ASSERT(offsetof(MainWindowSizingStatePilot, inSizeMove) == 0x3d - 4);

static MainWindowSizingStatePilot& ReadMainWindowSizingStatePilot(IdleControllerBoundaryPilot* controller)
{
    return *reinterpret_cast<MainWindowSizingStatePilot*>(reinterpret_cast<BYTE*>(controller) + 4);
}

static void DeleteMainWindowSnapshotPilot(MainWindowSizingStatePilot& state)
{
    if (state.snapshot != NULL) {
        DeleteObject(state.snapshot);
        state.snapshot = NULL;
    }
}

// 2012f1aa. Original receiver in ESI, message arguments on the stack.
extern "C" LRESULT __stdcall EnterMainWindowSizeMovePilot(
    IdleControllerBoundaryPilot* controller, WPARAM wParam, LPARAM lParam)
{
    MainWindowSizingStatePilot& state = ReadMainWindowSizingStatePilot(controller);
    DeleteMainWindowSnapshotPilot(state);
    HDC windowDc = GetDC(state.window);
    RECT bounds;
    GetClientRect(state.window, &bounds);
    HDC memoryDc = CreateCompatibleDC(windowDc);
    bool captured = false;
    if (memoryDc != NULL) {
        LONG width = bounds.right - bounds.left + 1;
        LONG height = bounds.bottom - bounds.top + 1;
        state.snapshotWidth = width;
        state.snapshotHeight = height;
        state.snapshot = CreateCompatibleBitmap(windowDc, width, height);
        captured = state.snapshot != NULL && SelectObject(memoryDc, state.snapshot) != NULL &&
            BitBlt(memoryDc, 0, 0, width, height, windowDc, 0, 0, SRCCOPY) != FALSE;
        DeleteDC(memoryDc);
    }
    ReleaseDC(state.window, windowDc);
    if (!captured) DeleteMainWindowSnapshotPilot(state);
    if (g_mainWindowEventPilot != NULL) g_mainWindowEventPilot(0x20017, 0, 0);
    state.inSizeMove = 1;
    return DefWindowProcA(state.window, WM_ENTERSIZEMOVE, wParam, lParam);
}

extern "C" BOOL CALLBACK RefreshSizedChildPilot(HWND window, LPARAM) // 20125264
{
    SendMessageA(window, 0x467, 0, 0);
    return TRUE;
}

extern "C" BOOL CALLBACK NotifySizedChildPilot(HWND window, LPARAM) // 20125285
{
    SendMessageA(window, WM_EXITSIZEMOVE, 0, 0);
    return TRUE;
}

// 2012f2c2. Original receiver in ESI, message arguments on the stack.
extern "C" LRESULT __stdcall ExitMainWindowSizeMovePilot(
    IdleControllerBoundaryPilot* controller, WPARAM wParam, LPARAM lParam)
{
    MainWindowSizingStatePilot& state = ReadMainWindowSizingStatePilot(controller);
    if (g_mainWindowEventPilot != NULL) g_mainWindowEventPilot(0x20018, 0, 0);
    state.inSizeMove = 0;
    DeleteMainWindowSnapshotPilot(state);
    if (!controller->QueryField34() || state.modeCallbacksEnabled == 0) {
        if (IsIconic(state.window)) return 0;
        RECT bounds;
        GetClientRect(state.window, &bounds);
        if (!EqualRect(&bounds, &state.clientBounds) || lParam == 0) {
            state.clientBounds = bounds;
            if (controller->QueryClipRegionRefresh()) {
                void* graphics = GetGraphicsControllerSourcePilot();
                void* entry = GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0x1c)(
                    graphics, reinterpret_cast<DWORD>(state.window));
                if (entry != NULL) GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(entry, 0x6c)(entry);
                EnumChildWindows(state.window, RefreshSizedChildPilot, 0);
            }
        }
    }
    EnumChildWindows(state.window, NotifySizedChildPilot, 0);
    controller->RefreshVisibleWindows();
    void* graphics = GetGraphicsControllerSourcePilot();
    GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(graphics, 0x38)(graphics);
    return DefWindowProcA(state.window, WM_EXITSIZEMOVE, wParam, lParam);
}

// 2012f709. This paints the parent snapshot, while child paint is suppressed.
extern "C" LRESULT __stdcall PaintMainWindowSizeMovePilot(
    IdleControllerBoundaryPilot* controller, WPARAM wParam, LPARAM lParam)
{
    MainWindowSizingStatePilot& state = ReadMainWindowSizingStatePilot(controller);
    if (state.inSizeMove == 0 && !controller->QueryField34()) {
        controller->RefreshVisibleWindows();
        return DefWindowProcA(state.window, WM_PAINT, wParam, lParam);
    }
    PAINTSTRUCT paint;
    HDC dc = BeginPaint(state.window, &paint);
    RECT bounds;
    GetClientRect(state.window, &bounds);
    LONG width = bounds.right - bounds.left;
    LONG height = bounds.bottom - bounds.top;
    if (state.snapshot == NULL) {
        PatBlt(dc, bounds.left, bounds.top, width, height, BLACKNESS);
    } else {
        HDC memoryDc = CreateCompatibleDC(dc);
        SelectObject(memoryDc, state.snapshot);
        int previousMode = SetStretchBltMode(dc, COLORONCOLOR);
        if (previousMode != 0) {
            StretchBlt(dc, bounds.left, bounds.top, width, height, memoryDc, 0, 0,
                state.snapshotWidth, state.snapshotHeight, SRCCOPY);
            SetStretchBltMode(dc, previousMode);
        }
        DeleteDC(memoryDc);
    }
    EndPaint(state.window, &paint);
    return 0;
}
