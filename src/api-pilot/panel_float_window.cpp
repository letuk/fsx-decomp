static DWORD g_floatingPanelGeometryPendingPilot; // 2020e444
static HWND g_floatingPanelSizingWindowPilot; // 2020e448

// 2012b579.
static bool CanResizeFloatingPanelPilot(ClipWindowObservedPilot* window)
{
    return window != NULL && ((window->flags & 0x80400) != 0 ||
        PanelGeometryFieldPilot(window, 0x60) != PanelGeometryFieldPilot(window, 0x68) ||
        PanelGeometryFieldPilot(window, 0x64) != PanelGeometryFieldPilot(window, 0x6c));
}

// 2013f199 and 2013f0d4.
static void UpdateFloatingPanelGeometryPilot(HWND hwnd, ClipWindowObservedPilot* window, UINT message)
{
    if (window == NULL || window->link == NULL || GetParent(window->link->window) != hwnd) return;
    ClipWindowLinkObservedPilot* link = window->link;
    RECT bounds;
    if (message == WM_SIZE) {
        GetClientRect(hwnd, &bounds);
        MoveWindow(link->window, 0, 0, bounds.right, bounds.bottom, TRUE);
        GetWindowRect(hwnd, &bounds);
        PanelGeometryFieldPilot(window, 0x58) = bounds.right - bounds.left;
        PanelGeometryFieldPilot(window, 0x5c) = bounds.bottom - bounds.top;
    } else {
        GetWindowRect(hwnd, &bounds);
        PanelGeometryFieldPilot(window, 0x50) = bounds.left;
        PanelGeometryFieldPilot(window, 0x54) = bounds.top;
    }
    SendMessageA(link->window, message, 0, 0);
    GetClientRect(hwnd, &bounds);
    MapWindowPoints(hwnd, g_mainWindowHandlePilot, reinterpret_cast<POINT*>(&bounds), 2);
    RECT controllerBounds;
    GraphicsClipSlotPilot<GraphicsClipRectPilot>(g_idleControllerPilot, 0x34)(
        g_idleControllerPilot, &controllerBounds);
    if (message == WM_MOVE) {
        window->left = bounds.left - controllerBounds.left;
        window->top = bounds.top - controllerBounds.top;
    }
    window->width = bounds.right - bounds.left;
    window->height = bounds.bottom - bounds.top;
}

static void UpdateFloatingPanelAndHostPilot(HWND hwnd, ClipWindowObservedPilot* window, UINT message)
{
    UpdateFloatingPanelGeometryPilot(hwnd, window, message);
    void* menu = QueryPanelMenuHostSourcePilot(hwnd, &g_panelMenuInterfaceIdPilot);
    if (menu != NULL) {
        typedef ClipWindowObservedPilot* (__stdcall *ReadHostRecordPilot)(void*);
        ClipWindowObservedPilot* record = GraphicsClipSlotPilot<ReadHostRecordPilot>(menu, 0x40)(menu);
        UpdateFloatingPanelGeometryPilot(hwnd, record, message);
    }
}

static void ForwardFloatingPanelSizeMovePilot(HWND hwnd, ClipWindowLinkObservedPilot* link,
    UINT message, WPARAM wParam, LPARAM lParam)
{
    g_floatingPanelGeometryPendingPilot = 0;
    if (link != NULL) SendMessageA(link->window, message, wParam, lParam);
    void* menu = QueryPanelMenuHostSourcePilot(hwnd, &g_panelMenuInterfaceIdPilot);
    if (menu != NULL) {
        typedef HWND (__stdcall *ReadHostWindowPilot)(void*);
        HWND host = GraphicsClipSlotPilot<ReadHostWindowPilot>(menu, 0x44)(menu);
        SendMessageA(host, message, wParam, lParam);
    }
}

static void AdjustFloatingPanelTrackSizePilot(POINT& point, DWORD style, DWORD exStyle)
{
    RECT bounds = {0, 0, point.x, point.y};
    AdjustWindowRectEx(&bounds, style, FALSE, exStyle);
    point.x = bounds.right - bounds.left;
    point.y = bounds.bottom - bounds.top;
}

// 2013e9a8.
extern "C" LRESULT CALLBACK FloatingPanelWindowProcedurePilot(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    ClipWindowObservedPilot* window = static_cast<ClipWindowObservedPilot*>(GetPropA(hwnd, PanelWindowPropertyPilot()));
    ClipWindowLinkObservedPilot* link = window != NULL ? window->link : NULL;
    switch (message) {
    case WM_CREATE: {
        CREATESTRUCTA* creation = reinterpret_cast<CREATESTRUCTA*>(lParam);
        if (creation->lpCreateParams == NULL) RemovePropA(hwnd, PanelWindowPropertyPilot());
        else SetPropA(hwnd, PanelWindowPropertyPilot(), creation->lpCreateParams);
        break;
    }
    case WM_DESTROY:
        RemovePropA(hwnd, PanelWindowPropertyPilot());
        break;
    case WM_MOVE:
    case WM_SIZE:
        if (g_floatingPanelSizingWindowPilot == hwnd)
            g_floatingPanelGeometryPendingPilot |= message == WM_MOVE ? 1 : 2;
        else UpdateFloatingPanelAndHostPilot(hwnd, window, message);
        break;
    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE && g_mainWindowEventPilot != NULL)
            g_mainWindowEventPilot(0x20007, reinterpret_cast<DWORD>(window), 0);
        break;
    case WM_SETFOCUS:
        if (link != NULL && GetParent(link->window) == hwnd && GetFocus() != link->window)
            SetFocus(link->window);
        break;
    case WM_PAINT: {
        if (link == NULL || GetParent(link->window) != hwnd) return 1;
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(hwnd, &paint);
        RECT outer, child;
        GetClientRect(hwnd, &outer);
        GetClientRect(link->window, &child);
        PatBlt(dc, child.right, 0, outer.right - child.right, outer.bottom, BLACKNESS);
        PatBlt(dc, 0, child.top, child.right, outer.bottom, BLACKNESS);
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_CLOSE:
        if (window != NULL) {
            if (g_mainWindowEventPilot != NULL)
                g_mainWindowEventPilot(0x2000a, reinterpret_cast<DWORD>(window), 0);
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_GETMINMAXINFO:
        if (link != NULL) {
            DWORD style = GetWindowLongA(hwnd, GWL_STYLE), exStyle = GetWindowLongA(hwnd, GWL_EXSTYLE);
            SendMessageA(link->window, message, wParam, lParam);
            MINMAXINFO* limits = reinterpret_cast<MINMAXINFO*>(lParam);
            AdjustFloatingPanelTrackSizePilot(limits->ptMaxTrackSize, style, exStyle);
            AdjustFloatingPanelTrackSizePilot(limits->ptMinTrackSize, style, exStyle);
        }
        break;
    case WM_NCHITTEST: {
        LRESULT hit = DefWindowProcA(hwnd, message, wParam, lParam);
        if (link != NULL && (hit == HTCLIENT || (hit >= HTLEFT && hit <= HTBOTTOMRIGHT &&
            !CanResizeFloatingPanelPilot(window)))) return HTCAPTION;
        return hit;
    }
    case WM_NCACTIVATE:
        if (g_idleControllerPilot->QueryField34()) return 1;
        break;
    case WM_NCRBUTTONDOWN:
        if (link != NULL && wParam == HTCAPTION) return SendMessageA(link->window, message, wParam, lParam);
        break;
    case WM_ENTERSIZEMOVE:
        g_floatingPanelSizingWindowPilot = hwnd;
        ForwardFloatingPanelSizeMovePilot(hwnd, link, message, wParam, lParam);
        break;
    case WM_EXITSIZEMOVE:
        if ((g_floatingPanelGeometryPendingPilot & 2) != 0) UpdateFloatingPanelAndHostPilot(hwnd, window, WM_SIZE);
        if ((g_floatingPanelGeometryPendingPilot & 1) != 0) UpdateFloatingPanelAndHostPilot(hwnd, window, WM_MOVE);
        g_floatingPanelSizingWindowPilot = NULL;
        ForwardFloatingPanelSizeMovePilot(hwnd, link, message, wParam, lParam);
        break;
    case 0x467:
        if (link != NULL) {
            DWORD style = GetWindowLongA(hwnd, GWL_STYLE), exStyle = GetWindowLongA(hwnd, GWL_EXSTYLE);
            if (CanResizeFloatingPanelPilot(window)) {
                if ((exStyle & WS_EX_CLIENTEDGE) != 0) {
                    SetWindowLongA(hwnd, GWL_STYLE, style | WS_THICKFRAME);
                    SetWindowLongA(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_CLIENTEDGE);
                }
            } else if ((exStyle & WS_EX_CLIENTEDGE) == 0) {
                SetWindowLongA(hwnd, GWL_STYLE, style & ~WS_THICKFRAME);
                SetWindowLongA(hwnd, GWL_EXSTYLE, exStyle | WS_EX_CLIENTEDGE);
            }
            if (g_idleUnthrottledPilot == 0 && g_idleControllerPilot->QueryMode() != 2) {
                bool visible = GetParent(link->window) == hwnd && (window->flags & 0x80000000) != 0;
                ShowWindow(hwnd, visible ? SW_SHOWNA : SW_HIDE);
                UpdateWindow(hwnd);
            }
        }
        break;
    case 0x475:
    case 0x47a:
        if (link != NULL) SendMessageA(link->window, message, wParam, lParam);
        break;
    }
    return DefWindowProcA(hwnd, message, wParam, lParam);
}
