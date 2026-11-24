typedef DWORD (__stdcall *PanelWindowErrorCallbackPilot)(DWORD, DWORD, const void*, DWORD, DWORD, HMODULE);
extern PanelWindowErrorCallbackPilot g_panelWindowErrorCallbackPilot; // 2020de78
extern BYTE g_panelWindowErrorContextPilot[]; // 2020d468

// 201230da.
extern "C" DWORD __stdcall ReportPanelWindowErrorPilot(DWORD code, DWORD argument, DWORD flags)
{
    if (g_panelWindowErrorCallbackPilot == NULL) return 0;
    return g_panelWindowErrorCallbackPilot(code, argument, g_panelWindowErrorContextPilot,
        flags, 0x65, g_languageModulePilot);
}

static ClipWindowLinkObservedPilot* AllocatePanelWindowLinkPilot()
{
    if (g_heapInitializedPilot == 0) InitializeHeapManagerPilot();
    void* memory = HeapAlloc(g_privateHeapPilot, HEAP_ZERO_MEMORY, sizeof(ClipWindowLinkObservedPilot));
    if (memory == NULL)
        memory = HeapAlloc(g_privateHeapPilot, HEAP_ZERO_MEMORY, sizeof(ClipWindowLinkObservedPilot));
    if (memory == NULL) {
        HMODULE module = g_languageModulePilot;
        if (module == NULL) module = GetModuleHandleA(NULL);
        LoadStringA(module, 8, g_savedWindowLocationPilot, MAX_PATH);
        MessageBoxA(NULL, g_savedWindowLocationPilot, NULL, MB_ICONHAND);
        TerminateProcess(GetCurrentProcess(), 1);
    }
    return static_cast<ClipWindowLinkObservedPilot*>(memory);
}

C_ASSERT(sizeof(ClipWindowLinkObservedPilot) == 0x48);

// 2011cc66. EDI carries the panel record in the original.
extern "C" LONG __stdcall CreatePanelHostWindowRecordSourcePilot(ClipWindowObservedPilot* window, HWND parent)
{
    if (window == NULL) return -1;
    ClipWindowLinkObservedPilot* link = AllocatePanelWindowLinkPilot();
    window->link = link;
    link->updateRegion = CreateRectRgn(0, 0, 0, 0);
    link->overlapRegion = CreateRectRgn(0, 0, 0, 0);
    CallbackArrayObservedPilot* hits = reinterpret_cast<CallbackArrayObservedPilot*>(&link->field2c);
    hits->growth = 0x20;
    hits->stride = 0x14;

    DWORD mode = *reinterpret_cast<DWORD*>(window->unknown0c);
    if ((mode & 1) != 0) window->flags |= 0x40000000;
    else if ((mode & 2) != 0) window->flags &= ~0x40000000u;
    HWND childParent = (mode & 2) != 0 ? parent : g_mainWindowHandlePilot;
    link->window = CreateWindowExA(0, "FS98CHILD", reinterpret_cast<LPCSTR>(window->unknown00),
        WS_CHILD, window->left, window->top, window->width, window->height,
        childParent, NULL, g_panelCursorModulePilot, window);
    if (link->window == NULL) {
        ReportPanelWindowErrorPilot(0xf004, 0, 0);
        FreePrivateHeapSlotPilot(reinterpret_cast<void**>(&window->link));
        return -1;
    }
    ShowWindow(link->window, SW_HIDE);
    UpdateWindow(link->window);
    mode = *reinterpret_cast<DWORD*>(window->unknown0c);
    if ((mode & 1) != 0) {
        link->auxiliaryWindow = parent;
    } else if ((mode & 2) != 0) {
        link->auxiliaryWindow = NULL;
    } else {
        link->auxiliaryWindow = CreateWindowExA(0x80, "FS98FLOAT", reinterpret_cast<LPCSTR>(window->unknown00),
            0x84cc0000, 0, 0, 0, 0, g_mainWindowHandlePilot, NULL, g_panelCursorModulePilot, window);
        if (link->auxiliaryWindow == NULL) {
            ReportPanelWindowErrorPilot(0xf004, 0, 0);
            FreePrivateHeapSlotPilot(reinterpret_cast<void**>(&window->link));
            return -1;
        }
        ShowWindow(link->auxiliaryWindow, SW_HIDE);
        UpdateWindow(link->auxiliaryWindow);
    }
    RegisterClipWindowPilot(window);
    SendMessageA(link->window, 0x467, 0, 0);
    return 0;
}

// 2011d262, API ordinal 25.
extern "C" void __stdcall FinalizePanelHostWindowRecordSourcePilot(ClipWindowObservedPilot* window)
{
    if (window == NULL || g_messageQuitRequestedPilot != 0 || GetGraphicsControllerSourcePilot() == NULL)
        return;
    SendMessageA(window->link->window, 0x467, 0, 0);
    SetWindowTextA(window->link->window, reinterpret_cast<LPCSTR>(window->unknown00));
}
