#include <vector>

extern "C" bool __cdecl QueryGraphicsTransitionReadyBoundaryPilot(); // g2d.dll ordinal 61
extern "C" void __cdecl HandleMainWindowGraphicsFailureSourcePilot(); // g2d.dll ordinal 65
extern "C" void __stdcall ReadTransitionWindowsBoundaryPilot(
    ClipWindowObservedPilot*** windows, DWORD* count, DWORD selector); // window.dll ordinal 15
extern "C" void __stdcall ReleaseTransitionWindowsBoundaryPilot(
    ClipWindowObservedPilot*** windows); // window.dll ordinal 16

C_ASSERT(offsetof(MainWindowSizingStatePilot, controllerOperationDepth) == 0x34 - 4);
C_ASSERT(offsetof(MainWindowSizingStatePilot, graphicsTransitionFlag) == 0x3c - 4);
C_ASSERT(offsetof(ClipWindowLinkObservedPilot, contextState) == 0x10);

typedef HWND (__thiscall *GraphicsTransitionWindowPilot)(void*, DWORD);
typedef void* (__thiscall *GraphicsTransitionLookupPilot)(void*, HWND);

static HWND ReadGraphicsTransitionWindowPilot(DWORD index)
{
    void* graphics = GetGraphicsControllerSourcePilot();
    return GraphicsClipSlotPilot<GraphicsTransitionWindowPilot>(graphics, 4)(graphics, index);
}

// 20126346. Record the graphics parent index before temporarily moving the HWND.
static void MoveTransitionWindowsToMainPilot()
{
    ClipWindowObservedPilot** windows = NULL;
    DWORD count = 0;
    ReadTransitionWindowsBoundaryPilot(&windows, &count, 0);
    for (DWORD i = 0; i < count; ++i) {
        ClipWindowObservedPilot* window = windows[i];
        if (!window || !window->link) continue;
        ClipWindowLinkObservedPilot& link = *window->link;
        link.contextState = 0xffffffffu;
        if ((window->flags & 0x40000000) == 0 && window->cachedGraphicsWindow &&
            GetParent(link.window) != g_mainWindowHandlePilot) {
            void* graphicsWindow = window->cachedGraphicsWindow;
            link.contextState = GraphicsClipSlotPilot<GraphicsClipIndexPilot>(graphicsWindow, 0x30)(graphicsWindow);
            SetParent(link.window, g_mainWindowHandlePilot);
        }
    }
    ReleaseTransitionWindowsBoundaryPilot(&windows);
}

// 201263e4. Release the window array before sending any synchronous refresh.
static void RestoreTransitionWindowParentsPilot()
{
    std::vector<HWND> restoredWindows;
    ClipWindowObservedPilot** windows = NULL;
    DWORD count = 0;
    ReadTransitionWindowsBoundaryPilot(&windows, &count, 0);
    for (DWORD i = 0; i < count; ++i) {
        ClipWindowObservedPilot* window = windows[i];
        if (!window || !window->link || (window->flags & 0x40000000) != 0) continue;
        ClipWindowLinkObservedPilot& link = *window->link;
        if (link.contextState == 0xffffffffu) continue;
        HWND parent = ReadGraphicsTransitionWindowPilot(link.contextState);
        if (parent) {
            SetParent(link.window, parent);
            restoredWindows.push_back(link.window);
        }
        link.contextState = 0xffffffffu;
    }
    ReleaseTransitionWindowsBoundaryPilot(&windows);
    for (size_t i = 0; i < restoredWindows.size(); ++i)
        SendMessageA(restoredWindows[i], 0x467, 0, 0);
}

// 201252a6.
static BOOL CALLBACK RefreshGraphicsTransitionChildPilot(HWND window, LPARAM)
{
    SendMessageA(window, 0x46c, 0, 0);
    return TRUE;
}

static void ApplyGraphicsTransitionPilot()
{
    void* graphics = GetGraphicsControllerSourcePilot();
    if (GraphicsClipSlotPilot<GraphicsClipIndexPilot>(graphics, 0x10)(graphics) != 0)
        HandleMainWindowGraphicsFailureSourcePilot();
}

static void CompleteGraphicsTransitionPilot()
{
    void* graphics = GetGraphicsControllerSourcePilot();
    GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(graphics, 0x38)(graphics);
}

// 2012f4f7. DefWindowProcA supplies the result on all branches.
static LRESULT HandleMainWindowGraphicsTransitionPilot(IdleControllerBoundaryPilot* controller,
    WPARAM wParam, LPARAM lParam)
{
    MainWindowSizingStatePilot& state = ReadMainWindowSizingStatePilot(controller);
    ControllerModeStatePilot& mode = ReadControllerModeStatePilot(controller);
    // Slot +0x0c, target 2012543c. The import's low byte is also stored at +0x39.
    mode.clipRefreshEnabled = static_cast<BYTE>(QueryGraphicsTransitionReadyBoundaryPilot());
    if (!mode.clipRefreshEnabled) {
        if (GetGraphicsControllerSourcePilot()) {
            if (mode.modeCallbacksEnabled) MoveTransitionWindowsToMainPilot();
            void* graphics = GetGraphicsControllerSourcePilot();
            GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(graphics, 0x14)(graphics);
        }
        state.graphicsTransitionFlag = 1;
    } else if (!mode.clipRefreshOverride) {
        state.graphicsTransitionFlag = 1;
    } else {
        if (mode.modeCallbacksEnabled) {
            bool suspendedModal = mode.mode == 2;
            if (suspendedModal) controller->LeaveModalWindowPilot();
            if (g_mainWindowEventPilot) g_mainWindowEventPilot(0x20025, wParam, 0);
            MoveTransitionWindowsToMainPilot();
            state.controllerOperationDepth = static_cast<LONG>(static_cast<DWORD>(state.controllerOperationDepth) + 1);
            ApplyGraphicsTransitionPilot();
            state.controllerOperationDepth = static_cast<LONG>(static_cast<DWORD>(state.controllerOperationDepth) - 1);
            CompleteGraphicsTransitionPilot();
            RestoreTransitionWindowParentsPilot();
            if (g_mainWindowEventPilot) g_mainWindowEventPilot(0x20026, wParam, 0);
            SetCursorOverridePilot(1);
            void* graphics = GetGraphicsControllerSourcePilot();
            DWORD count = GraphicsClipSlotPilot<GraphicsClipIndexPilot>(graphics, 0x0c)(graphics);
            for (DWORD i = 0; i < count; ++i) {
                HWND window = ReadGraphicsTransitionWindowPilot(i);
                if (window) PostMessageA(window, WM_SIZE, 0, 0);
            }
            ReorderClipWindowsPilot();
            SetCursorOverridePilot(0);
            if (suspendedModal) controller->EnterModalWindowPilot();
        } else {
            ApplyGraphicsTransitionPilot();
            CompleteGraphicsTransitionPilot();
            EnumChildWindows(state.window, RefreshGraphicsTransitionChildPilot, 0);
        }
        state.graphicsTransitionFlag = 0;
        controller->RefreshVisibleWindows();
    }
    return DefWindowProcA(state.window, 0x46f, wParam, lParam);
}

// 2012f694 and the 0x470 dispatch gate at 2012fccc.
static LRESULT HandleMainWindowGraphicsCursorPilot(IdleControllerBoundaryPilot* controller)
{
    if (!ReadMainWindowSizingStatePilot(controller).modeCallbacksEnabled ||
        !GetGraphicsControllerSourcePilot()) return 0;
    void* graphics = GetGraphicsControllerSourcePilot();
    void* window = GraphicsClipSlotPilot<GraphicsTransitionLookupPilot>(graphics, 0x1c)(
        graphics, g_mainWindowHandlePilot);
    if (window) {
        SetCursor(LoadCursorA(NULL, IDC_WAIT));
        GraphicsClipSlotPilot<GraphicsClipWordPilot>(window, 0x88)(window, 1);
    }
    return 0;
}
