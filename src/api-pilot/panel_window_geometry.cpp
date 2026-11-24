// API import and graphics-table boundaries.
extern "C" void __stdcall ReadPanelGraphicsLimitsPilot(ClipWindowObservedPilot*, DWORD*, DWORD*, DWORD*); // ordinal 66
extern "C" void __stdcall EnumeratePanelWindowsSourcePilot(ClipWindowObservedPilot***, DWORD*, DWORD); // ordinal 15
extern "C" void __stdcall ReleasePanelWindowEnumerationSourcePilot(ClipWindowObservedPilot***); // ordinal 16

static LONG& PanelGeometryFieldPilot(void* storage, unsigned int offset)
{
    return *reinterpret_cast<LONG*>(static_cast<BYTE*>(storage) + offset);
}

static void* FindPanelGeometryGraphicsPilot(HWND hwnd)
{
    void* graphics = GetGraphicsControllerSourcePilot();
    return GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0x20)(graphics, reinterpret_cast<DWORD>(hwnd));
}

static void ReadPanelGraphicsRectPilot(DWORD index, RECT* bounds)
{
    typedef void (__thiscall *ReadRectPilot)(void*, DWORD, RECT*);
    void* graphics = GetGraphicsControllerSourcePilot();
    GraphicsClipSlotPilot<ReadRectPilot>(graphics, 0x18)(graphics, index, bounds);
}

// 2012b83a.
static void ReadPanelSizeLimitsPilot(PanelUpdateContextObservedPilot* context)
{
    RECT client;
    GetClientRect(context->windowHandle, &client);
    if (client.right < 0 && client.bottom < 0) return;
    ZeroMemory(context->unknown10, 0x28);
    ClipWindowObservedPilot* window = context->window;
    DWORD maximumWidth = 0, maximumHeight = 0;
    PanelGeometryFieldPilot(context, 0x28) = PanelGeometryFieldPilot(window, 0x60);
    PanelGeometryFieldPilot(context, 0x2c) = PanelGeometryFieldPilot(window, 0x64);
    if (window->regionUnionFlag != 0) {
        RECT bounds = { 0, 0, 0, 0 };
        DWORD ignored;
        ReadPanelGraphicsLimitsPilot(window, &maximumWidth, &maximumHeight, &ignored);
        DWORD index = ReadClipWindowGraphicsIndexPilot(context->window);
        if (g_idleControllerPilot->QueryField38() && static_cast<LONG>(index) < 0) {
            void* entry = FindPanelGeometryGraphicsPilot(context->windowHandle);
            if (entry != NULL) index = GraphicsClipSlotPilot<GraphicsClipIndexPilot>(entry, 0x30)(entry);
        }
        if (static_cast<LONG>(index) >= 0) ReadPanelGraphicsRectPilot(index, &bounds);
        DWORD width = bounds.right - bounds.left, height = bounds.bottom - bounds.top;
        PanelGeometryFieldPilot(context, 0x18) = width;
        PanelGeometryFieldPilot(context, 0x1c) = height;
        if (g_idleControllerPilot->QueryField38() || (context->window->flags & 0x48000000) == 0) {
            if (width <= maximumWidth) maximumWidth = width;
            if (height <= maximumHeight) maximumHeight = height;
        }
    }
    window = context->window;
    DWORD requestedWidth = PanelGeometryFieldPilot(window, 0x68);
    DWORD requestedHeight = PanelGeometryFieldPilot(window, 0x6c);
    if (requestedWidth == 0 ||
        (requestedWidth != static_cast<DWORD>(PanelGeometryFieldPilot(window, 0x60)) && maximumWidth <= requestedWidth))
        requestedWidth = maximumWidth;
    if (requestedHeight == 0 ||
        (requestedHeight != static_cast<DWORD>(PanelGeometryFieldPilot(window, 0x64)) && maximumHeight <= requestedHeight))
        requestedHeight = maximumHeight;
    PanelGeometryFieldPilot(context, 0x30) = requestedWidth;
    PanelGeometryFieldPilot(context, 0x34) = requestedHeight;
}

// 2012c129. Store coordinates in the panel's 8192-by-6144 reference grid.
static void SavePanelScaledGeometryPilot(PanelUpdateContextObservedPilot* context)
{
    ClipWindowObservedPilot* window = context->window;
    if ((window->flags & 0x100) != 0 || window->width <= 1 || window->height <= 1) return;
    void* entry = FindPanelGeometryGraphicsPilot(context->windowHandle);
    if (entry == NULL) return;
    RECT bounds;
    GraphicsClipSlotPilot<GraphicsClipRectPilot>(entry, 0xc)(entry, &bounds);
    LONG xScale = (bounds.right - bounds.left) * 0x10000 / 0x2000;
    LONG yScale = (bounds.bottom - bounds.top) * 0x10000 / 0x1800;
    window = context->window;
    if ((window->flags & 0x40000000) == 0) {
        PanelGeometryFieldPilot(window, 0x20) = (xScale / 2 + window->left * 0x10000) / xScale;
        PanelGeometryFieldPilot(window, 0x24) = (yScale / 2 + window->top * 0x10000) / yScale;
    }
    PanelGeometryFieldPilot(window, 0x28) = (xScale / 2 + window->width * 0x10000) / xScale;
    PanelGeometryFieldPilot(window, 0x2c) = (yScale / 2 + window->height * 0x10000) / yScale;
}

extern "C" LRESULT __stdcall PanelChildMoveSourcePilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (PanelGeometryFieldPilot(context, 0x3c) == 0) {
        DWORD index = ReadClipWindowGraphicsIndexPilot(context->window);
        if (static_cast<LONG>(index) >= 0) {
            RECT rect;
            GetWindowRect(context->windowHandle, &rect);
            if ((context->window->flags & 0x60000000) == 0) {
                POINT position = { rect.left, rect.top };
                ScreenToClient(GetParent(context->windowHandle), &position);
                ReadPanelGraphicsRectPilot(index, &rect);
                ClipWindowObservedPilot* window = context->window;
                window->left = position.x - rect.left;
                PanelGeometryFieldPilot(window, 0x40) = window->left;
                window->top = position.y - rect.top;
                PanelGeometryFieldPilot(window, 0x44) = window->top;
                if ((window->flags & 0x8000000) == 0 && PanelGeometryFieldPilot(context, 0x38) == 0)
                    SavePanelScaledGeometryPilot(context);
            }
            ReorderClipWindowsPilot();
            if ((context->window->flags & 0x40000000) != 0) {
                if (g_panelGraphicsNotificationPendingPilot == 0) {
                    void* entry = FindPanelGeometryGraphicsPilot(context->windowHandle);
                    if (entry != NULL)
                        GraphicsClipSlotPilot<GraphicsClipWordPilot>(entry, 0x54)(
                            entry, reinterpret_cast<DWORD>(context->window));
                }
                NotifyPanelGraphicsControllerPilot();
            }
        }
    }
    g_idleControllerPilot->RefreshVisibleWindows();
    return DefaultPanelMessagePilot(context, message, wParam, lParam);
}

extern "C" LRESULT __stdcall PanelChildSizeSourcePilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    ClipWindowObservedPilot* window = context->window;
    if (window->cachedGraphicsWindow != NULL) {
        RECT bounds;
        GetWindowRect(context->windowHandle, &bounds);
        if ((window->flags & 0x20000000) == 0) {
            window->width = bounds.right - bounds.left;
            window->height = bounds.bottom - bounds.top;
            if ((window->flags & 0x40000000) == 0) {
                PanelGeometryFieldPilot(window, 0x48) = window->width;
                PanelGeometryFieldPilot(window, 0x4c) = window->height;
            } else {
                RECT frame = { 0, 0, window->width, window->height };
                DWORD style = GetWindowLongA(context->link->auxiliaryWindow, GWL_STYLE);
                DWORD exStyle = GetWindowLongA(context->link->auxiliaryWindow, GWL_EXSTYLE);
                BOOL hasMenu = GetMenu(context->link->auxiliaryWindow) != NULL;
                AdjustWindowRectEx(&frame, style, hasMenu, exStyle);
                PanelGeometryFieldPilot(window, 0x58) = frame.right - frame.left;
                PanelGeometryFieldPilot(window, 0x5c) = frame.bottom - frame.top;
            }
            if ((window->flags & 0x8000000) == 0 && PanelGeometryFieldPilot(context, 0x38) == 0)
                SavePanelScaledGeometryPilot(context);
        }
        if (bounds.right != bounds.left && bounds.bottom != bounds.top) RefreshPanelSurfaceSourcePilot(context);
        ReorderClipWindowsPilot();
        typedef void (__stdcall *ResizeCallbackPilot)(DWORD, ClipWindowObservedPilot*, DWORD, RECT*);
        ResizeCallbackPilot callback = *reinterpret_cast<ResizeCallbackPilot*>(
            reinterpret_cast<BYTE*>(context->window) + 0x18);
        if (callback != NULL) {
            GetClientRect(context->windowHandle, &bounds);
            callback(1, context->window, 0x10000 |
                (ReadMainWindowSizingStatePilot(g_mainWindowControllerPilot).inSizeMove == 0), &bounds);
        }
        if ((context->window->flags & 0x40000000) != 0) NotifyPanelGraphicsControllerPilot();
    }
    g_idleControllerPilot->RefreshVisibleWindows();
    return DefaultPanelMessagePilot(context, message, wParam, lParam);
}

extern "C" LRESULT __stdcall PanelChildMinMaxSourcePilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    ReadPanelSizeLimitsPilot(context);
    MINMAXINFO* limits = reinterpret_cast<MINMAXINFO*>(lParam);
    if ((context->window->flags & 0x80400) != 0) {
        limits->ptMinTrackSize.x = 39;
        limits->ptMinTrackSize.y = 39;
    } else {
        LONG* values[] = { &limits->ptMinTrackSize.x, &limits->ptMinTrackSize.y,
            &limits->ptMaxTrackSize.x, &limits->ptMaxTrackSize.y };
        for (unsigned int i = 0; i < 4; ++i) {
            LONG limit = PanelGeometryFieldPilot(context, 0x28 + i * 4);
            if (limit != 0) *values[i] = limit;
        }
    }
    return DefaultPanelMessagePilot(context, message, wParam, lParam);
}

// 2012c502.
static void NotifyRelatedPanelGeometryPilot()
{
    ClipWindowObservedPilot** windows = NULL;
    DWORD count = 0;
    EnumeratePanelWindowsSourcePilot(&windows, &count, 0);
    for (DWORD i = 0; i < count; ++i) {
        ClipWindowObservedPilot* window = windows[i];
        if (window->link != NULL && window->link->geometryLink != NULL &&
            (window->flags & 0x80000100) == 0x80000100) {
            PanelUpdateContextObservedPilot* related =
                reinterpret_cast<PanelUpdateContextObservedPilot*>(window->link->geometryLink);
            SendMessageA(related->windowHandle, WM_MOVE, 0, 0);
            SendMessageA(related->windowHandle, WM_SIZE, 0, 0);
        }
    }
    ReleasePanelWindowEnumerationSourcePilot(&windows);
}

extern "C" LRESULT __stdcall PanelChildExitSizeMoveSourcePilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    SendMessageA(context->windowHandle, WM_MOVE, 0, 0);
    SendMessageA(context->windowHandle, WM_SIZE, 0, 0);
    if ((context->window->flags & 0x1000) != 0) NotifyRelatedPanelGeometryPilot();
    return DefaultPanelMessagePilot(context, message, wParam, lParam);
}

extern "C" LRESULT __stdcall PanelDialogMoveSourcePilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    if ((context->window->flags & 0x40000000) != 0) NotifyPanelGraphicsControllerPilot();
    return DefaultPanelMessagePilot(context, message, wParam, lParam);
}

static LONG AlignRelatedPanelAxisPilot(LONG origin, LONG size, LONG grid, LONG spare)
{
    if (origin == 0) return 0;
    return origin + size < grid ? spare / 2 : spare - 1;
}

// 2012c3d1.
static void PositionRelatedPanelPilot(PanelUpdateContextObservedPilot* context, ClipWindowObservedPilot* target)
{
    BYTE& positioning = context->unknown10[0x30];
    if (positioning != 0) return;
    positioning = 1;
    ClipWindowObservedPilot* window = context->window;
    LONG x = PanelGeometryFieldPilot(window, 0x20), y = PanelGeometryFieldPilot(window, 0x24);
    LONG width = PanelGeometryFieldPilot(window, 0x28), height = PanelGeometryFieldPilot(window, 0x2c);
    if ((window->flags & 0x80400) == 0) {
        x = AlignRelatedPanelAxisPilot(x, width, 8192, target->surfaceWidth - window->surfaceWidth);
        y = AlignRelatedPanelAxisPilot(y, height, 6144, target->surfaceHeight - window->surfaceHeight);
        SetWindowPos(context->windowHandle, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    } else {
        DWORD targetWidth = target->surfaceWidth, targetHeight = target->surfaceHeight;
        SetWindowPos(context->windowHandle, NULL,
            static_cast<LONG>(x * targetWidth) / 8192, static_cast<LONG>(y * targetHeight) / 6144,
            static_cast<LONG>(width * targetWidth) / 8192, static_cast<LONG>(height * targetHeight) / 6144,
            SWP_NOZORDER);
    }
    context->window->relatedWidth = target->surfaceWidth;
    context->window->relatedHeight = target->surfaceHeight;
    positioning = 0;
}

extern "C" LRESULT __stdcall PanelChildRelatedSizeSourcePilot(
    PanelUpdateContextObservedPilot* context, UINT, WPARAM, LPARAM)
{
    ClipWindowObservedPilot* target = GetAlternatePanelInputWindowPilot();
    if (target != NULL) {
        ClipWindowObservedPilot* window = context->window;
        if (window->width == 0 || window->height == 0 ||
            window->relatedWidth < target->surfaceWidth - 2 || window->relatedWidth > target->surfaceWidth + 2 ||
            window->relatedHeight < target->surfaceHeight - 2 || window->relatedHeight > target->surfaceHeight + 2)
            PositionRelatedPanelPilot(context, target);
    }
    return 0;
}
