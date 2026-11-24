// The original table and remaining message dispatcher are source boundaries.
extern void* g_mainWindowControllerTablePilot[]; // 200f2ce0
extern "C" LRESULT __stdcall DispatchRemainingMainWindowMessagePilot(
    UINT message, WPARAM wParam, LPARAM lParam); // 2012f93d

struct MainWindowControllerStoragePilot {
    void** table;
    MainWindowSizingStatePilot state;
};
C_ASSERT(sizeof(MainWindowControllerStoragePilot) == 0x6c);
C_ASSERT(offsetof(MainWindowSizingStatePilot, ownedBlock) == 0x60 - 4);
C_ASSERT(offsetof(MainWindowSizingStatePilot, allowRemoteSession) == 0x40 - 4);

static void ReleaseMainWindowOwnedBlockPilot(MainWindowSizingStatePilot& state)
{
    if (state.ownedBlock != NULL) HeapFree(g_privateHeapPilot, 0, state.ownedBlock);
    state.ownedBlock = NULL;
    state.unknown64 = 0;
    state.unknown68 = 0;
}

// 20125350. Initialization order preserves the original configuration call boundary.
extern "C" IdleControllerBoundaryPilot* __stdcall InitializeMainWindowControllerPilot(
    MainWindowControllerStoragePilot* storage, HWND window)
{
    storage->table = g_mainWindowControllerTablePilot;
    MainWindowSizingStatePilot& state = storage->state;
    state.window = window;
    state.snapshot = NULL;
    state.controllerOperationDepth = 0;
    state.modeCallbacksEnabled = 0;
    ZeroMemory(state.unknown39, sizeof(state.unknown39));
    state.graphicsTransitionFlag = 0;
    state.inSizeMove = 0;
    state.modalWindowFlag = 0;
    state.unknown3f = 0;
    state.allowRemoteSession = 0;
    ZeroMemory(state.unknown44, sizeof(state.unknown44));
    state.modalDepth = 0;
    state.closeOverride = 0;
    state.modalSavedFocus = NULL;
    state.unknown58 = 0;
    state.ownedBlock = NULL;
    state.unknown64 = 0;
    state.unknown68 = 0;
    __try {
        // The original signed >= 0 or <= 2 test accepts every returned value.
        g_savedControllerWindowModePilot = GetPrivateProfileIntA(
            "MAIN", "Maximized", g_savedControllerWindowModePilot, g_configurationFilePilot);
        ReadSelectedConfigurationIntPilot("MAIN", "ALLOW_REMOTE_SESSION", &state.allowRemoteSession, NULL, 0);
        ZeroMemory(&state.clientBounds, sizeof(state.clientBounds));
        ZeroMemory(state.unknown18, sizeof(state.unknown18));
        state.snapshotWidth = 0;
        state.snapshotHeight = 0;
    } __finally {
        if (AbnormalTermination()) ReleaseMainWindowOwnedBlockPilot(state);
    }
    return reinterpret_cast<IdleControllerBoundaryPilot*>(storage);
}

// 2012e506. The original receiver is in ESI and is returned after free.
extern "C" IdleControllerBoundaryPilot* __stdcall DestroyMainWindowControllerPilot(
    IdleControllerBoundaryPilot* controller)
{
    MainWindowControllerStoragePilot* storage = reinterpret_cast<MainWindowControllerStoragePilot*>(controller);
    storage->table = g_mainWindowControllerTablePilot;
    DeleteMainWindowSnapshotPilot(storage->state);
    ReleaseMainWindowOwnedBlockPilot(storage->state);
    HeapFree(g_privateHeapPilot, 0, storage);
    return controller;
}

// 2012e413. Publish both aliases only after the constructor completes.
extern "C" LRESULT CALLBACK MainWindowProcedurePilot(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    IdleControllerBoundaryPilot* controller = g_mainWindowControllerPilot;
    if (message == WM_NCDESTROY) {
        if (controller != NULL) DestroyMainWindowControllerPilot(controller);
        g_mainWindowControllerPilot = NULL;
        g_idleControllerPilot = NULL;
        controller = NULL;
    }
    if (controller == NULL && g_messageQuitRequestedPilot == 0) {
        void* memory;
        AllocatePrivateHeapMemoryPilot(1, sizeof(MainWindowControllerStoragePilot),
            &memory, g_factoryZeroInitializeBoundaryPilot);
        __try {
            if (memory != NULL)
                controller = InitializeMainWindowControllerPilot(
                    static_cast<MainWindowControllerStoragePilot*>(memory), window);
        } __finally {
            if (AbnormalTermination() && memory != NULL) HeapFree(g_privateHeapPilot, 0, memory);
        }
        g_mainWindowControllerPilot = controller;
        g_idleControllerPilot = controller;
    }
    if (controller == NULL) return DefWindowProcA(window, message, wParam, lParam);

    switch (message) {
    case WM_SETCURSOR: return HandleMainWindowSetCursorPilot(controller, wParam, lParam);
    case WM_CLOSE: return CloseMainWindowPilot(controller, wParam, lParam);
    case WM_COMMAND: return HandleMainWindowCommandPilot(controller, wParam, lParam);
    case WM_PAINT: return PaintMainWindowSizeMovePilot(controller, wParam, lParam);
    case WM_ENTERSIZEMOVE: return EnterMainWindowSizeMovePilot(controller, wParam, lParam);
    case WM_EXITSIZEMOVE: return ExitMainWindowSizeMovePilot(controller, wParam, lParam);
    case 0x465: return HandleMainWindowTimerMessagePilot(controller, wParam, lParam);
    case 0x46b: return RefreshFloatingWindowsMessagePilot(controller, wParam, lParam);
    case 0x46f: return HandleMainWindowGraphicsTransitionPilot(controller, wParam, lParam);
    case 0x470: return HandleMainWindowGraphicsCursorPilot(controller);
    default: return DispatchRemainingMainWindowMessagePilot(message, wParam, lParam);
    }
}
