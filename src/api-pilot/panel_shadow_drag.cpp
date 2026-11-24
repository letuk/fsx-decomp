extern "C" int __cdecl RunApplicationIdlePilot();
extern DWORD g_shadowTargetGraphicsIndexPilot; // 201f7094
extern RECT g_shadowTargetClientBoundsPilot; // 2020e0b0

struct PanelShadowDragPilot {
    MINMAXINFO limits;
    DWORD references;
    HWND shadow;
    HWND panel;
    HWND targetParent;
    RECT targetBounds;
    POINT anchor;
    RECT initialBounds;
    UINT hit;
};
C_ASSERT(sizeof(PanelShadowDragPilot) == 0x64);
C_ASSERT(offsetof(PanelShadowDragPilot, references) == 0x28);
C_ASSERT(offsetof(PanelShadowDragPilot, hit) == 0x60);

struct PanelShadowSiblingListPilot {
    DWORD unknown00;
    HWND* begin;
    HWND* end;
    HWND* capacity;
};
extern "C" void __stdcall AppendPanelShadowSiblingSourcePilot(PanelShadowSiblingListPilot*, const HWND*); // 201265b6

static RECT ReadPanelScreenBoundsPilot(HWND hwnd)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    ClientToScreen(hwnd, reinterpret_cast<POINT*>(&rect.left));
    ClientToScreen(hwnd, reinterpret_cast<POINT*>(&rect.right));
    return rect;
}

static BOOL CALLBACK CollectPanelShadowSiblingPilot(HWND hwnd, LPARAM argument)
{
    char name[0xff];
    GetClassNameA(hwnd, name, sizeof(name));
    if (_stricmp(name, "FS98CHILD") == 0) {
        ClipWindowObservedPilot* window = static_cast<ClipWindowObservedPilot*>(GetPropA(hwnd, PanelWindowPropertyPilot()));
        if (window != NULL && (window->flags & 0xa0000000) == 0x80000000 &&
            (window->panelKind == 2 || window->panelKind == 3 || window->panelKind == 4))
            AppendPanelShadowSiblingSourcePilot(reinterpret_cast<PanelShadowSiblingListPilot*>(argument), &hwnd);
    }
    return TRUE;
}

static bool ShadowResizesLeftPilot(UINT hit) { return hit == 10 || hit == 13 || hit == 16; }
static bool ShadowResizesRightPilot(UINT hit) { return hit == 11 || hit == 14 || hit == 17; }
static bool ShadowResizesTopPilot(UINT hit) { return hit == 12 || hit == 13 || hit == 14; }
static bool ShadowResizesBottomPilot(UINT hit) { return hit == 15 || hit == 16 || hit == 17; }
static bool ShadowNearEdgePilot(LONG delta) { return delta > -15 && delta < 15; }

// 2013cfbb. Uses the same ordered sibling scan as the original.
static void ResolvePanelShadowTargetPilot(PanelShadowDragPilot* drag, const RECT& requested,
    RECT* result, HWND* parent, void** target)
{
    RECT search = requested;
    void* graphics;
    if (g_idleControllerPilot->QueryField38()) {
        typedef void* (__thiscall *LookupRectPilot)(void*, RECT*);
        graphics = GetGraphicsControllerSourcePilot();
        *target = GraphicsClipSlotPilot<LookupRectPilot>(graphics, 0x24)(graphics, &search);
        if (*target != NULL) *parent = GraphicsClipSlotPilot<GraphicsClipWindowPilot>(*target, 0x10)(*target);
    } else {
        graphics = GetGraphicsControllerSourcePilot();
        *target = GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0x1c)(
            graphics, reinterpret_cast<DWORD>(*parent));
    }
    RECT candidate = search;
    RECT parentBounds = ReadPanelScreenBoundsPilot(*parent);
    ClipWindowObservedPilot* window = static_cast<ClipWindowObservedPilot*>(GetPropA(drag->panel, PanelWindowPropertyPilot()));
    bool contain = window->regionUnionFlag != 0 || (window->flags & 0x400000) != 0;
    PanelShadowSiblingListPilot siblings;
    siblings.begin = siblings.end = siblings.capacity = NULL;
    EnumChildWindows(*parent, CollectPanelShadowSiblingPilot, reinterpret_cast<LPARAM>(&siblings));
    for (HWND* current = siblings.begin; current != siblings.end; ++current) {
        if (*current == drag->panel) continue;
        RECT sibling = ReadPanelScreenBoundsPilot(*current);
        RECT expanded = sibling, overlap;
        InflateRect(&expanded, 15, 15);
        IntersectRect(&overlap, &expanded, &candidate);
        if (IsRectEmpty(&overlap)) continue;
        LONG delta = sibling.right - candidate.left;
        if (ShadowNearEdgePilot(delta) && sibling.right < candidate.right) OffsetRect(&candidate, delta, 0);
        else {
            delta = sibling.left - candidate.right;
            if (ShadowNearEdgePilot(delta) && candidate.left < sibling.left) OffsetRect(&candidate, delta, 0);
        }
        delta = sibling.bottom - candidate.top;
        if (ShadowNearEdgePilot(delta) && sibling.bottom < candidate.bottom) OffsetRect(&candidate, 0, delta);
        else {
            delta = sibling.top - candidate.bottom;
            if (ShadowNearEdgePilot(delta) && candidate.top < sibling.top) OffsetRect(&candidate, 0, delta);
        }
    }
    LONG delta = parentBounds.left - candidate.left;
    if (ShadowNearEdgePilot(delta) && candidate.right < parentBounds.right) OffsetRect(&candidate, delta, 0);
    else {
        delta = parentBounds.right - candidate.right;
        if (ShadowNearEdgePilot(delta) && parentBounds.left < candidate.left) OffsetRect(&candidate, delta, 0);
    }
    delta = parentBounds.top - candidate.top;
    if (ShadowNearEdgePilot(delta) && candidate.bottom < parentBounds.bottom) OffsetRect(&candidate, 0, delta);
    else {
        delta = parentBounds.bottom - candidate.bottom;
        if (ShadowNearEdgePilot(delta) && parentBounds.top < candidate.top) OffsetRect(&candidate, 0, delta);
    }
    if (siblings.begin != NULL) HeapFree(g_privateHeapPilot, 0, siblings.begin);

    RECT inset = parentBounds;
    InflateRect(&inset, -15, -15);
    if (!contain) {
        if (candidate.right < inset.left) OffsetRect(&candidate, inset.left - candidate.right, 0);
        if (inset.right < candidate.left) OffsetRect(&candidate, inset.right - candidate.left, 0);
        if (candidate.bottom < inset.top) OffsetRect(&candidate, 0, inset.top - candidate.bottom);
        if (inset.bottom < candidate.top) OffsetRect(&candidate, 0, inset.bottom - candidate.top);
    } else {
        if (candidate.left < inset.left) OffsetRect(&candidate, parentBounds.left - candidate.left, 0);
        if (inset.right < candidate.right) OffsetRect(&candidate, parentBounds.right - candidate.right, 0);
        if (candidate.top < inset.top) OffsetRect(&candidate, 0, parentBounds.top - candidate.top);
        if (inset.bottom < candidate.bottom) OffsetRect(&candidate, 0, parentBounds.bottom - candidate.bottom);
        if (ShadowResizesLeftPilot(drag->hit)) candidate.right = drag->initialBounds.right;
        if (ShadowResizesRightPilot(drag->hit)) candidate.left = drag->initialBounds.left;
        if (ShadowResizesTopPilot(drag->hit)) candidate.bottom = drag->initialBounds.bottom;
        if (ShadowResizesBottomPilot(drag->hit)) candidate.top = drag->initialBounds.top;
        IntersectRect(&candidate, &candidate, &parentBounds);
    }
    *result = candidate;
}

static LONG ClampShadowSizePilot(LONG size, LONG minimum, LONG maximum)
{
    if (size < minimum) return minimum;
    if (maximum != 0 && maximum < size) return maximum;
    return size;
}

// 2013d7f5.
static LRESULT MovePanelShadowPilot(PanelShadowDragPilot* drag, WPARAM keys, LPARAM coordinates)
{
    if ((keys & MK_LBUTTON) != 0) {
        POINT position = UnpackPanelPointPilot(coordinates);
        ClientToScreen(drag->shadow, &position);
        HWND parent = GetParent(drag->panel);
        UINT flags = 0x1c;
        RECT requested, preview;
        if (drag->hit == HTCAPTION) {
            GetClientRect(drag->shadow, &requested);
            OffsetRect(&requested, position.x - drag->anchor.x, position.y - drag->anchor.y);
            preview.left = position.x;
            preview.top = position.y;
            preview.right = preview.bottom = 0;
            flags = 0x1d;
        } else {
            RECT parentBounds = ReadPanelScreenBoundsPilot(parent);
            if (position.x < parentBounds.left) position.x = parentBounds.left;
            else if (position.x > parentBounds.right) position.x = parentBounds.right;
            if (position.y < parentBounds.top) position.y = parentBounds.top;
            else if (position.y > parentBounds.bottom) position.y = parentBounds.bottom;
            GetPropA(drag->panel, PanelWindowPropertyPilot());
            requested = drag->initialBounds;
            if (ShadowResizesLeftPilot(drag->hit)) {
                LONG width = requested.right - (position.x - drag->anchor.x) + 1;
                requested.left = requested.right + 1 - ClampShadowSizePilot(width,
                    drag->limits.ptMinTrackSize.x, drag->limits.ptMaxTrackSize.x);
            } else if (ShadowResizesRightPilot(drag->hit)) {
                LONG width = requested.right - drag->anchor.x - 1 + position.x - 2 * requested.left + 1;
                requested.right = requested.left - 1 + ClampShadowSizePilot(width,
                    drag->limits.ptMinTrackSize.x, drag->limits.ptMaxTrackSize.x);
            }
            if (ShadowResizesTopPilot(drag->hit)) {
                LONG height = requested.bottom - (position.y - drag->anchor.y) + 1;
                requested.top = requested.bottom + 1 - ClampShadowSizePilot(height,
                    drag->limits.ptMinTrackSize.y, drag->limits.ptMaxTrackSize.y);
            } else if (ShadowResizesBottomPilot(drag->hit)) {
                LONG height = requested.bottom - drag->anchor.y - 1 + position.y - 2 * requested.top + 1;
                requested.bottom = requested.top - 1 + ClampShadowSizePilot(height,
                    drag->limits.ptMinTrackSize.y, drag->limits.ptMaxTrackSize.y);
            }
            preview = requested;
        }
        void* graphics;
        RECT targetBounds;
        ResolvePanelShadowTargetPilot(drag, requested, &targetBounds, &parent, &graphics);
        ScreenToClient(parent, reinterpret_cast<POINT*>(&targetBounds.left));
        ScreenToClient(parent, reinterpret_cast<POINT*>(&targetBounds.right));
        if (graphics != NULL) {
            g_shadowTargetGraphicsIndexPilot = GraphicsClipSlotPilot<GraphicsClipIndexPilot>(graphics, 0x30)(graphics);
            g_shadowTargetClientBoundsPilot = targetBounds;
        }
        drag->targetParent = parent;
        drag->targetBounds = targetBounds;
        SetWindowPos(drag->shadow, NULL, preview.left, preview.top,
            preview.right - preview.left + 1, preview.bottom - preview.top + 1, flags);
    }
    g_cursorSuppressedPilot = 100;
    RunApplicationIdlePilot();
    return 1;
}

// 2013daee.
static LRESULT CommitPanelShadowPilot(PanelShadowDragPilot* drag)
{
    g_shadowTargetGraphicsIndexPilot = 0xffffffffu;
    HWND parent = drag->targetParent;
    if (parent != NULL && parent != GetParent(drag->panel)) {
        HWND focus = GetFocus();
        if (IsWindow(drag->panel)) {
            if (parent != GetDesktopWindow()) SetParent(drag->panel, parent);
            else GetParent(drag->panel);
        }
        if (focus == drag->panel) SetFocus(focus);
    }
    void* graphics = GetGraphicsControllerSourcePilot();
    if (GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0x1c)(
        graphics, reinterpret_cast<DWORD>(drag->targetParent)) != NULL) {
        const RECT& rect = drag->targetBounds;
        MoveWindow(drag->panel, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, FALSE);
        SendMessageA(drag->panel, 0x467, 0, 0);
    }
    g_cursorSuppressedPilot = 100;
    DestroyWindow(drag->shadow);
    return 0;
}

// 2013d703 and its WM_DESTROY/WM_PAINT helpers.
static LRESULT DispatchPanelShadowPilot(PanelShadowDragPilot* drag, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_DESTROY: {
        ReleaseCapture();
        ClipWindowObservedPilot* panel = static_cast<ClipWindowObservedPilot*>(
            GetPropA(drag->panel, PanelWindowPropertyPilot()));
        if (panel != NULL) RaiseClipWindowInGroupPilot(panel);
        --drag->references;
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT paint;
        BeginPaint(drag->shadow, &paint);
        EndPaint(drag->shadow, &paint);
        return 0;
    }
    case WM_ERASEBKGND: return 1;
    case WM_NCPAINT: return 0;
    case WM_MOUSEMOVE: return MovePanelShadowPilot(drag, wParam, lParam);
    case WM_LBUTTONUP: return CommitPanelShadowPilot(drag);
    }
    return DefWindowProcA(drag->shadow, message, wParam, lParam);
}

// 2013d683. A dispatch reference keeps the state alive across nested WM_DESTROY.
extern "C" LRESULT CALLBACK PanelShadowWindowProcedurePilot(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PanelShadowDragPilot* drag = reinterpret_cast<PanelShadowDragPilot*>(GetWindowLongA(hwnd, GWL_USERDATA));
    if (message == WM_CREATE) SetCapture(hwnd);
    if (drag == NULL) return DefWindowProcA(hwnd, message, wParam, lParam);
    ++drag->references;
    LRESULT result = DispatchPanelShadowPilot(drag, message, wParam, lParam);
    if (--drag->references == 0) {
        HeapFree(g_privateHeapPilot, 0, drag);
        SetWindowLongA(hwnd, GWL_USERDATA, 0);
    }
    return result;
}

// 2013d488. The original panel HWND arrives in EAX.
extern "C" void __stdcall PreparePanelLeftButtonSourcePilot(HWND panel, WPARAM hit, LONG x, LONG y)
{
    RECT bounds = ReadPanelScreenBoundsPilot(panel);
    HWND shadow = CreateWindowExA(0xa0, "FS98SHADOW", "Child Shadow", WS_POPUP,
        bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top,
        NULL, NULL, g_panelCursorModulePilot, NULL);
    if (shadow == NULL) return;
    void* memory;
    AllocatePrivateHeapMemoryPilot(1, sizeof(PanelShadowDragPilot), &memory, g_factoryZeroInitializeBoundaryPilot);
    if (memory == NULL) return;
    PanelShadowDragPilot* drag = static_cast<PanelShadowDragPilot*>(memory);
    ZeroMemory(drag, sizeof(*drag));
    drag->references = 1;
    drag->shadow = shadow;
    drag->panel = panel;
    drag->anchor.x = x;
    drag->anchor.y = y;
    SendMessageA(panel, WM_GETMINMAXINFO, 0, reinterpret_cast<LPARAM>(&drag->limits));
    if (drag->limits.ptMinTrackSize.x != 0) ++drag->limits.ptMinTrackSize.x;
    if (drag->limits.ptMinTrackSize.y != 0) ++drag->limits.ptMinTrackSize.y;
    drag->initialBounds = bounds;
    POINT pointer = { x, y };
    ClientToScreen(panel, &pointer);
    drag->hit = static_cast<UINT>(hit);
    InflateRect(&bounds, -4, -4);
    if (pointer.x < bounds.left) {
        drag->hit = pointer.y < bounds.top ? 13 : (pointer.y > bounds.bottom ? 16 : 10);
    } else if (pointer.x > bounds.right) {
        drag->hit = pointer.y < bounds.top ? 14 : (pointer.y > bounds.bottom ? 17 : 11);
    } else if (pointer.y < bounds.top) drag->hit = 12;
    else if (pointer.y > bounds.bottom) drag->hit = 15;
    SetWindowLongA(shadow, GWL_USERDATA, reinterpret_cast<LONG>(drag));
    ShowWindow(shadow, SW_SHOWNOACTIVATE);
    UpdateWindow(shadow);
}
