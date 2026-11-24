static void RestorePanelChildGeometryPilot(ClipWindowObservedPilot* window)
{
    window->left = PanelGeometryFieldPilot(window, 0x40);
    window->top = PanelGeometryFieldPilot(window, 0x44);
    window->width = PanelGeometryFieldPilot(window, 0x48);
    window->height = PanelGeometryFieldPilot(window, 0x4c);
}

// 2012befe. Convert saved reference-grid geometry to this graphics frame.
static void ScalePanelChildToParentPilot(PanelUpdateContextObservedPilot* context, HWND parent)
{
    ClipWindowObservedPilot* window = context->window;
    if ((window->flags & 0x100) != 0 && window->width != 0) return;
    void* graphics = GetGraphicsControllerSourcePilot();
    void* entry = GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0x1c)(
        graphics, reinterpret_cast<DWORD>(parent));
    if (entry == NULL) return;
    RECT bounds;
    GraphicsClipSlotPilot<GraphicsClipRectPilot>(entry, 0xc)(entry, &bounds);
    LONG width = bounds.right - bounds.left, height = bounds.bottom - bounds.top;
    LONG xScale = (width * 0x10000) / 8192, yScale = (height * 0x10000) / 6144;
    window = context->window;
    if ((window->flags & 0x40000000) == 0) {
        window->left = (PanelGeometryFieldPilot(window, 0x20) * xScale + 0x8000) >> 16;
        window->top = (PanelGeometryFieldPilot(window, 0x24) * yScale + 0x8000) >> 16;
    }
    window->width = (PanelGeometryFieldPilot(window, 0x28) * xScale + 0x8000) >> 16;
    window->height = (PanelGeometryFieldPilot(window, 0x2c) * yScale + 0x8000) >> 16;
    LONG minWidth = PanelGeometryFieldPilot(window, 0x60), minHeight = PanelGeometryFieldPilot(window, 0x64);
    bool fixedWidth = minWidth != 0 && minWidth == PanelGeometryFieldPilot(window, 0x68);
    bool fixedHeight = minHeight != 0 && minHeight == PanelGeometryFieldPilot(window, 0x6c);
    if ((window->flags & 0x80000) == 0) {
        if (fixedWidth) {
            window->width = minWidth;
            if (window->left < 0 || window->left > width) window->left = 0;
        }
        if (fixedHeight) {
            window->height = minHeight;
            if (window->top < 0 || window->top > height) window->top = 0;
        }
    } else if (ReadMainWindowSizingStatePilot(g_mainWindowControllerPilot).inSizeMove == 0) {
        LONG widthDelta = window->width - minWidth, heightDelta = window->height - minHeight;
        if (widthDelta >= -16 && widthDelta <= 16 && heightDelta >= -32 && heightDelta <= 32) {
            if (fixedWidth) window->width = minWidth;
            if (fixedHeight) {
                LONG bottom = window->top + window->height;
                window->height = minHeight;
                LONG spare = bounds.bottom - bottom;
                if ((window->flags & 0x6000000) == 0x4000000 || (spare >= -16 && spare <= 16))
                    window->top = bounds.bottom - minHeight;
                else if (spare < -16)
                    window->top += heightDelta;
            }
        }
    }
}

// 2012c2bb.
static void AnchorPanelChildPilot(PanelUpdateContextObservedPilot* context)
{
    RECT bounds = {0, 0, 0, 0};
    void* entry = context->window->cachedGraphicsWindow;
    if (entry != NULL) GraphicsClipSlotPilot<GraphicsClipRectPilot>(entry, 0xc)(entry, &bounds);
    ClipWindowObservedPilot* window = context->window;
    LONG spareX = bounds.right - bounds.left - window->width;
    LONG spareY = bounds.bottom - bounds.top - window->height;
    switch (window->flags & 0x6000000) {
    case 0x2000000: window->top = 0; break;
    case 0x4000000: window->top = spareY; break;
    case 0x6000000: window->top = spareY / 2; break;
    }
    if ((window->flags & 0x6000000) != 0x2000000 && window->top < 20) window->top = 20;
    switch (window->flags & 0x1800000) {
    case 0x800000: window->left = 0; break;
    case 0x1000000: window->left = spareX; break;
    case 0x1800000: window->left = spareX / 2; break;
    }
    if ((window->flags & 0x1800000) != 0x800000 && window->left < 0) window->left = 0;
}

static void SetPanelParentPilot(HWND hwnd, HWND parent)
{
    if (IsWindow(hwnd)) {
        if (parent == NULL || parent == GetDesktopWindow()) GetParent(hwnd);
        else SetParent(hwnd, parent);
    }
}

// 2012c22f.
static void ReparentRelatedPanelsPilot(HWND parent)
{
    ClipWindowObservedPilot** windows = NULL;
    DWORD count = 0;
    EnumeratePanelWindowsSourcePilot(&windows, &count, 0);
    for (DWORD i = 0; i < count; ++i) {
        ClipWindowObservedPilot* window = windows[i];
        if (window != NULL && window->link != NULL && (window->flags & 0x100) != 0)
            SetPanelParentPilot(window->link->window, parent);
    }
    ReleasePanelWindowEnumerationSourcePilot(&windows);
}

static void ReparentPanelForRefreshPilot(PanelUpdateContextObservedPilot* context, HWND parent)
{
    if (context->contextField44 != 0)
        ReleasePanelContextGraphicsSourcePilot(context->window, context->contextField44);
    HWND hwnd = context->windowHandle;
    ++PanelGeometryFieldPilot(context, 0x3c);
    context->contextField44 = 0;
    SetPanelParentPilot(hwnd, parent);
    --PanelGeometryFieldPilot(context, 0x3c);
}

static void MovePanelIntoGraphicsFramePilot(PanelUpdateContextObservedPilot* context, HWND parent, const RECT& frame)
{
    HWND auxiliary = context->link->auxiliaryWindow;
    if (auxiliary != NULL) {
        ShowWindow(auxiliary, SW_HIDE);
        UpdateWindow(context->link->auxiliaryWindow);
    }
    ClipWindowObservedPilot* window = context->window;
    if ((window->flags & 0x8000000) == 0) ScalePanelChildToParentPilot(context, parent);
    else if (PanelGeometryFieldPilot(window, 0x48) > 0 && PanelGeometryFieldPilot(window, 0x4c) > 0)
        RestorePanelChildGeometryPilot(window);

    RECT client;
    GetClientRect(context->windowHandle, &client);
    MapWindowPoints(context->windowHandle, parent, reinterpret_cast<POINT*>(&client), 2);
    window = context->window;
    bool inside = client.left >= frame.left && client.top >= frame.top &&
        client.right <= frame.right && client.bottom <= frame.bottom;
    bool sameSize = window->width == PanelGeometryFieldPilot(window, 0x48) &&
        window->height == PanelGeometryFieldPilot(window, 0x4c);
    LONG width = client.right - client.left, height = client.bottom - client.top;
    if (inside && sameSize && (window->flags & 0x1000) == 0) {
        window->left = client.left - frame.left;
        window->top = client.top - frame.top;
        window->width = width;
        window->height = height;
    } else if ((window->flags & 0x8000000) == 0) {
        if (width < 1 || height < 1) {
            if (PanelGeometryFieldPilot(window, 0x48) > 0 && PanelGeometryFieldPilot(window, 0x4c) > 0)
                RestorePanelChildGeometryPilot(window);
        } else {
            window->width = min(width, frame.right - frame.left);
            window->height = min(height, frame.bottom - frame.top);
            if (window->left + window->width > frame.right) window->left = frame.right - window->width;
            if (window->top + window->height > frame.bottom) window->top = frame.bottom - window->height;
        }
    }
    if ((window->flags & 0x8000000) == 0) SavePanelScaledGeometryPilot(context);
    ReparentPanelForRefreshPilot(context, parent);
    window = context->window;
    if ((window->flags & 0x1000) != 0) ReparentRelatedPanelsPilot(parent);
    window = context->window;
    MoveWindow(context->windowHandle, window->left + frame.left, window->top + frame.top,
        window->width, window->height, TRUE);
}

static void MovePanelIntoAuxiliaryFramePilot(PanelUpdateContextObservedPilot* context, HWND parent)
{
    ClipWindowObservedPilot* window = context->window;
    PanelGeometryFieldPilot(window, 0x40) = window->left;
    PanelGeometryFieldPilot(window, 0x44) = window->top;
    PanelGeometryFieldPilot(window, 0x48) = window->width;
    PanelGeometryFieldPilot(window, 0x4c) = window->height;
    HWND auxiliary = context->link->auxiliaryWindow;
    LONG& left = PanelGeometryFieldPilot(window, 0x50), &top = PanelGeometryFieldPilot(window, 0x54);
    LONG& width = PanelGeometryFieldPilot(window, 0x58), &height = PanelGeometryFieldPilot(window, 0x5c);
    if (width > 0 && window->height > 0) {
        RECT frame = {0, 0, window->width, window->height};
        DWORD style = GetWindowLongA(auxiliary, GWL_STYLE), exStyle = GetWindowLongA(auxiliary, GWL_EXSTYLE);
        AdjustWindowRectEx(&frame, style, GetMenu(auxiliary) != NULL, exStyle);
        width = frame.right - frame.left;
        height = frame.bottom - frame.top;
    }
    if (width != 0 && height != 0) {
        HDC dc = GetDC(NULL);
        RECT frame = {left, top, left + width, top + height};
        if (!RectVisible(dc, &frame)) width = height = 0;
        ReleaseDC(NULL, dc);
    }
    if (width == 0 || height == 0) {
        RECT frame;
        HWND client = (PanelGeometryFieldPilot(window, 0xc) & 1) != 0 ? auxiliary : context->windowHandle;
        GetClientRect(client, &frame);
        if (frame.right <= frame.left || frame.bottom <= frame.top) {
            if ((window->flags & 0x8000000) == 0) ScalePanelChildToParentPilot(context, parent);
            left = window->left;
            top = window->top;
            width = window->width;
            height = window->height;
        } else {
            MapWindowPoints(context->windowHandle, NULL, reinterpret_cast<POINT*>(&frame), 2);
            DWORD exStyle = GetWindowLongA(auxiliary, GWL_EXSTYLE), style = GetWindowLongA(auxiliary, GWL_STYLE);
            AdjustWindowRectEx(&frame, style, FALSE, exStyle);
            left = frame.left;
            top = frame.top;
            width = frame.right - frame.left;
            height = frame.bottom - frame.top;
        }
        HDC dc = GetDC(NULL);
        if (!PtVisible(dc, left, top) && !PtVisible(dc, left + width / 2, top + GetSystemMetrics(SM_CYCAPTION) / 2))
            left = top = 0;
        ReleaseDC(NULL, dc);
    }
    ReparentPanelForRefreshPilot(context, context->link->auxiliaryWindow);
    if ((context->window->flags & 0x1000) != 0) ReparentRelatedPanelsPilot(context->link->auxiliaryWindow);
    window = context->window;
    if ((PanelGeometryFieldPilot(window, 0xc) & 1) == 0)
        MoveWindow(context->link->auxiliaryWindow, PanelGeometryFieldPilot(window, 0x50),
            PanelGeometryFieldPilot(window, 0x54), PanelGeometryFieldPilot(window, 0x58),
            PanelGeometryFieldPilot(window, 0x5c), TRUE);
}

// 201401d3. The original panel context arrives in EAX.
extern "C" LRESULT __stdcall PanelChildRefreshSourcePilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!GraphicsClipSlotPilot<GraphicsClipFlagPilot>(g_mainWindowControllerPilot, 8)(g_mainWindowControllerPilot))
        return DefaultPanelMessagePilot(context, message, wParam, lParam);
    ++PanelGeometryFieldPilot(context, 0x38);
    if (g_idleControllerPilot->QueryField38()) context->window->flags &= ~0x40000000u;
    for (;;) {
        ClipWindowObservedPilot* window = context->window;
        if (window->width == 0 || window->height == 0) RestorePanelChildGeometryPilot(window);
        DWORD style = GetWindowLongA(context->windowHandle, GWL_STYLE);
        GetWindowLongA(context->windowHandle, GWL_EXSTYLE);
        bool visible = (style & WS_VISIBLE) != 0;
        bool show = (context->window->flags & 0x80000000) != 0;
        typedef void (__stdcall *VisibilityCallbackPilot)(DWORD, ClipWindowObservedPilot*, DWORD, RECT*);
        VisibilityCallbackPilot callback = *reinterpret_cast<VisibilityCallbackPilot*>(
            reinterpret_cast<BYTE*>(context->window) + 0x18);
        if (visible != show && callback != NULL) {
            RECT client;
            GetClientRect(context->windowHandle, &client);
            callback(show ? 3 : 4, context->window, 0, &client);
        }
        if (visible && (context->window->flags & 0x80000000) == 0) {
            context->window->cachedGraphicsWindow = NULL;
            SetWindowLongA(context->windowHandle, GWL_STYLE, style & ~WS_VISIBLE);
            if ((context->window->flags & 0x40000000) != 0 && context->link->auxiliaryWindow != NULL)
                SetWindowPos(context->link->auxiliaryWindow, NULL, 0, 0, 0, 0, 0xb7);
            g_panelGraphicsNotificationPendingPilot = 1;
            break;
        }
        if (g_messageQuitRequestedPilot != 0) break;
        void* previousEntry = context->window->cachedGraphicsWindow;
        HWND parent = context->windowHandle;
        bool useMainWindow = !g_idleControllerPilot->QueryField38() &&
            (context->window->flags & 0x40000100) == 0;
        if (!useMainWindow) useMainWindow = !g_idleControllerPilot->QueryField38() && previousEntry == NULL;
        if (useMainWindow) parent = g_mainWindowHandlePilot;
        void* graphics = GetGraphicsControllerSourcePilot();
        void* entry = GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0x20)(
            graphics, reinterpret_cast<DWORD>(parent));
        if (entry == NULL) {
            g_panelGraphicsNotificationPendingPilot = 1;
            break;
        }
        if (!useMainWindow) parent = GraphicsClipSlotPilot<GraphicsClipWindowPilot>(entry, 0x10)(entry);
        if (entry != previousEntry) {
            GraphicsClipSlotPilot<GraphicsClipWordPilot>(entry, 0x54)(
                entry, reinterpret_cast<DWORD>(context->window));
            g_panelGraphicsNotificationPendingPilot = 1;
        }
        RECT frame;
        GraphicsClipSlotPilot<GraphicsClipRectPilot>(entry, 0xc)(entry, &frame);
        DWORD flags = context->window->flags;
        bool notifyGeometry = visible && (flags & 0x80004000) == 0;
        if ((flags & 0x40000000) != 0 && GetParent(context->windowHandle) == parent &&
            (context->window->flags & 0x100) == 0) {
            MovePanelIntoAuxiliaryFramePilot(context, parent);
            notifyGeometry = true;
            g_panelGraphicsNotificationPendingPilot = 1;
        } else if ((context->window->flags & 0x40000000) == 0 && GetParent(context->windowHandle) != parent &&
            (context->window->flags & 0x100) == 0) {
            MovePanelIntoGraphicsFramePilot(context, parent, frame);
            notifyGeometry = true;
            g_panelGraphicsNotificationPendingPilot = 1;
        }
        flags = context->window->flags;
        UINT positionFlags = (flags & 0x80000000) != 0 ? 0x77 : 0xb7;
        if ((flags & 0x4000) != 0) {
            g_idleControllerPilot->RefreshVisibleWindows();
            positionFlags |= SWP_NOREDRAW;
        }
        SetWindowPos(context->windowHandle, NULL, 0, 0, 0, 0, positionFlags);
        if (visible != ((context->window->flags & 0x80000000) != 0)) g_panelGraphicsNotificationPendingPilot = 1;
        if (notifyGeometry) RedrawWindow(parent, NULL, NULL, 0x181);
        if ((context->window->flags & 0x8000000) == 0) ScalePanelChildToParentPilot(context, parent);
        window = context->window;
        flags = window->flags;
        if ((flags & 0x100000) != 0) window->flags &= ~0x100000u;
        else if ((flags & 0x7800000) != 0 && (flags & 0x80000) == 0) AnchorPanelChildPilot(context);
        window = context->window;
        flags = window->flags;
        if ((flags & 0x40000000) != 0) {
            GetClientRect(GetParent(context->windowHandle), &frame);
            MoveWindow(context->windowHandle, frame.left, frame.top,
                frame.right - frame.left, frame.bottom - frame.top, TRUE);
        } else if ((flags & 0x20000000) != 0) {
            MoveWindow(context->windowHandle, frame.left, frame.top,
                frame.right - frame.left, frame.bottom - frame.top, TRUE);
        } else if ((flags & 0x100) != 0 && (window->width == 0 || window->height == 0)) {
            ClipWindowObservedPilot* target = GetAlternatePanelInputWindowPilot();
            if (target != NULL) PositionRelatedPanelPilot(context, target);
        } else {
            if ((flags & 0x400000) != 0 &&
                (window->width > frame.right - frame.left || window->height > frame.bottom - frame.top) &&
                !g_idleControllerPilot->QueryField38()) {
                context->window->flags |= 0x40000000;
                PanelGeometryFieldPilot(context->window, 0x100) = 1;
                continue;
            }
            if (window->regionUnionFlag != 0 || (flags & 0x400000) != 0) {
                if (window->left + window->width > frame.right) window->left = frame.right - window->width - frame.left;
                if (window->top + window->height > frame.bottom) window->top = frame.bottom - window->height - frame.top;
                if (window->left < 0) window->left = 0;
                if (window->top < 0) window->top = 0;
            }
            MoveWindow(context->windowHandle, frame.left + window->left, frame.top + window->top,
                window->width, window->height, TRUE);
        }
        RefreshPanelSurfaceSourcePilot(context);
        if (notifyGeometry) {
            SendMessageA(context->windowHandle, WM_MOVE, 0, 0);
            SendMessageA(context->windowHandle, WM_SIZE, 0, 0);
            if ((context->window->flags & 0x1000) != 0) NotifyRelatedPanelGeometryPilot();
        }
        if (GetParent(context->windowHandle) != parent)
            SendMessageA(GetParent(context->windowHandle), 0x467, 0, 0);
        ReorderClipWindowsPilot();
        break;
    }
    if (g_messageQuitRequestedPilot == 0 && g_panelGraphicsNotificationPendingPilot != 0)
        NotifyPanelGraphicsControllerPilot();
    --PanelGeometryFieldPilot(context, 0x38);
    return DefaultPanelMessagePilot(context, message, wParam, lParam);
}
