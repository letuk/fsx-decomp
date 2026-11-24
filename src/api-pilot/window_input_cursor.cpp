extern HINSTANCE g_panelCursorModulePilot; // 2020e024
extern HCURSOR g_controllerCursorCachePilot[6]; // 2020de4c
extern DWORD g_panelHoverResetPilot; // 2020e05c
extern DWORD g_currentPanelHoverPilot; // 2020e064
extern int g_cursorSuppressionEnabledPilot; // 201f7164
extern int g_timerCursorGatePilot; // 2020e03c
extern LONG g_cursorOverrideDepthPilot; // 2020dee8
extern POINT g_lastCursorPositionPilot; // 2020e668
extern "C" int __cdecl QueryControlsCursorOverridePilot(); // controls.dll ordinal 45

static bool UsesControllerCursorCachePilot()
{
    return g_idleControllerPilot->QueryField38() &&
        GraphicsClipSlotPilot<GraphicsClipFlagPilot>(g_idleControllerPilot, 0x20)(g_idleControllerPilot);
}

// 2011fbc1. The original name arrives in EAX.
extern "C" HCURSOR __stdcall CreateControllerCursorPilot(LPCSTR name)
{
    HANDLE original = LoadImageA(NULL, name, IMAGE_CURSOR, 0, 0, 0x8001);
    HICON cursor = static_cast<HICON>(CopyImage(original, IMAGE_CURSOR, 0, 0, 1));
    ICONINFO info;
    BITMAP mask;
    if (!GetIconInfo(cursor, &info) || info.hbmMask == NULL || info.hbmColor == NULL ||
        GetObjectA(info.hbmMask, sizeof(mask), &mask) != sizeof(mask) || mask.bmWidth != mask.bmHeight)
        return reinterpret_cast<HCURSOR>(cursor);

    HDC source = CreateCompatibleDC(NULL);
    if (source == NULL) return reinterpret_cast<HCURSOR>(cursor);
    HDC destination = CreateCompatibleDC(NULL);
    if (destination != NULL) {
        HGDIOBJ previousSource = SelectObject(source, info.hbmMask);
        HBITMAP stackedMask = CreateBitmap(mask.bmWidth, mask.bmHeight * 2,
            mask.bmPlanes, mask.bmBitsPixel, NULL);
        if (stackedMask != NULL) {
            HGDIOBJ previousDestination = SelectObject(destination, stackedMask);
            BitBlt(destination, 0, 0, mask.bmWidth, mask.bmHeight, source, 0, 0, SRCCOPY);
            SelectObject(source, info.hbmColor);
            BitBlt(destination, 0, mask.bmHeight, mask.bmWidth, mask.bmHeight, source, 0, 0, SRCCOPY);
            SelectObject(destination, previousDestination);
            SelectObject(source, previousSource);
            DeleteObject(info.hbmMask);
            info.hbmMask = stackedMask;
            DeleteObject(info.hbmColor);
            info.hbmColor = NULL;
            DestroyIcon(cursor);
            cursor = CreateIconIndirect(&info);
        }
        SelectObject(source, previousSource);
        DeleteDC(destination);
    }
    DeleteDC(source);
    return reinterpret_cast<HCURSOR>(cursor);
}

// 2011fd46. The first cache entry is also the initialization check.
static HCURSOR LoadPanelCursorPilot(HINSTANCE module, LPCSTR name)
{
    if (UsesControllerCursorCachePilot() && module == NULL) {
        static const WORD ids[] = { 0x7f00, 0x7f02, 0x7f85, 0x7f84, 0x7f82, 0x7f83 };
        if (g_controllerCursorCachePilot[0] == NULL) {
            for (unsigned int i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i)
                g_controllerCursorCachePilot[i] = CreateControllerCursorPilot(MAKEINTRESOURCEA(ids[i]));
        }
        for (unsigned int i = 1; i < sizeof(ids) / sizeof(ids[0]); ++i) {
            if (name == MAKEINTRESOURCEA(ids[i])) return g_controllerCursorCachePilot[i];
        }
        return g_controllerCursorCachePilot[0];
    }
    return LoadCursorA(module, name);
}

// 2011fb07 / 230. HWND and WPARAM are carried by the original interface.
extern "C" int __stdcall ApplyControllerHitCursorPilot(HWND, WPARAM, LPARAM lParam)
{
    if (!UsesControllerCursorCachePilot()) return 0;
    WORD resource = 0x7f00;
    switch (LOWORD(lParam)) {
    case 10: case 11: resource = 0x7f84; break;
    case 12: case 15: resource = 0x7f85; break;
    case 13: case 17: resource = 0x7f82; break;
    case 14: case 16: resource = 0x7f83; break;
    }
    SetCursor(LoadPanelCursorPilot(NULL, MAKEINTRESOURCEA(resource)));
    return 1;
}

static void ObserveCursorMovementPilot(bool storeXFirst)
{
    POINT position;
    GetCursorPos(&position);
    if (position.x != g_lastCursorPositionPilot.x || position.y != g_lastCursorPositionPilot.y)
        g_cursorSuppressedPilot = 100;
    if (storeXFirst) {
        g_lastCursorPositionPilot.x = position.x;
        g_lastCursorPositionPilot.y = position.y;
    } else {
        g_lastCursorPositionPilot.y = position.y;
        g_lastCursorPositionPilot.x = position.x;
    }
}

// 2012caa7. The original receives the panel context in ESI.
static bool ApplyPanelMessageCursorPilot(PanelUpdateContextObservedPilot* context, WPARAM wParam, LPARAM lParam)
{
    ObserveCursorMovementPilot(false);
    if (g_idleControllerPilot->QueryMode() == 0 && g_cursorSuppressedPilot == -1 &&
        g_cursorSuppressionEnabledPilot != 0) {
        SetCursor(NULL);
        return true;
    }
    if (QueryControlsCursorOverridePilot() != 0) {
        SetCursor(LoadCursorA(NULL, MAKEINTRESOURCEA(0x7f03)));
        return true;
    }
    if (g_timerCursorGatePilot == 0 && context->link != NULL && context->link->inputCursor != 0 &&
        GraphicsClipSlotPilot<GraphicsClipFlagPilot>(g_idleControllerPilot, 0x20)(g_idleControllerPilot)) {
        SetCursor(reinterpret_cast<HCURSOR>(context->link->inputCursor));
        return true;
    }
    return ApplyControllerHitCursorPilot(context->windowHandle, wParam, lParam) != 0;
}

// 20141f43. Both original panel dispatchers use this handler.
extern "C" LRESULT __stdcall HandlePanelSetCursorPilot(
    PanelUpdateContextObservedPilot* context, WPARAM wParam, LPARAM lParam)
{
    if (ApplyPanelMessageCursorPilot(context, wParam, lParam)) return 1;
    return DefWindowProcA(context->windowHandle, WM_SETCURSOR, wParam, lParam);
}

// 2012f031. The original controller is in ESI.
static LRESULT HandleMainWindowSetCursorPilot(
    IdleControllerBoundaryPilot* controller, WPARAM wParam, LPARAM lParam)
{
    ObserveCursorMovementPilot(true);
    if (g_timerCursorGatePilot != 0) {
        SetCursor(LoadPanelCursorPilot(NULL, MAKEINTRESOURCEA(0x7f02)));
        return 1;
    }
    const ControllerModeStatePilot& state = ReadControllerModeStatePilot(controller);
    if (state.mode == 0 && g_cursorSuppressedPilot == -1 && g_cursorSuppressionEnabledPilot != 0) {
        SetCursor(NULL);
        return 1;
    }
    if (state.modeCallbacksEnabled != 0) {
        if (state.mode == 2) return 1;
        if (g_idleUnthrottledPilot != 0) {
            SetCursor(NULL);
            return 1;
        }
    }
    MainWindowSizingStatePilot& window = ReadMainWindowSizingStatePilot(controller);
    if (ApplyControllerHitCursorPilot(window.window, wParam, lParam) != 0) return 1;
    return DefWindowProcA(window.window, WM_SETCURSOR, wParam, lParam);
}

// 2012e1dd / 12. ShowCursor is invoked on every call.
extern "C" HCURSOR __stdcall SetCursorOverridePilot(int enable)
{
    DWORD depth = static_cast<DWORD>(g_cursorOverrideDepthPilot);
    g_cursorOverrideDepthPilot = static_cast<LONG>(depth + (enable != 0 ? 1u : 0xffffffffu));
    if (g_cursorOverrideDepthPilot > 0) {
        g_timerCursorGatePilot = 1;
        HCURSOR cursor = LoadPanelCursorPilot(NULL, MAKEINTRESOURCEA(0x7f02));
        ShowCursor(TRUE);
        return SetCursor(cursor);
    }
    g_timerCursorGatePilot = 0;
    ShowCursor(FALSE);
    if (g_idleControllerPilot->QueryMode() == 0 && g_cursorSuppressedPilot == -1 &&
        g_cursorSuppressionEnabledPilot != 0) return SetCursor(NULL);
    return SetCursor(LoadPanelCursorPilot(NULL, MAKEINTRESOURCEA(0x7f00)));
}

// 20137c26. Outputs already written survive a failed client conversion.
static bool ReadPanelPointerStatePilot(HWND* target, POINT* screen, POINT* client, DWORD* keys)
{
    if (target != NULL || screen != NULL || client != NULL) {
        POINT position;
        if (!GetCursorPos(&position)) return false;
        HWND window = WindowFromPoint(position);
        if (target != NULL) *target = window;
        if (screen != NULL) *screen = position;
        if (client != NULL) {
            if (window != NULL && !ScreenToClient(window, &position)) return false;
            *client = position;
        }
    }
    if (keys != NULL) *keys = ReadPanelPointerKeysPilot();
    return true;
}

// 20137cf8 / 253.
extern "C" int __stdcall QueryPanelPointerPilot(ClipWindowObservedPilot* window, POINT* output, DWORD* actions)
{
    POINT position;
    HWND target;
    DWORD keys;
    if (!ReadPanelPointerStatePilot(&target, &position, NULL, actions != NULL ? &keys : NULL)) return -1;
    if (actions != NULL) {
        DWORD flags = 0;
        if ((keys & 1) != 0) flags |= 0x20000000;
        if ((keys & 0x10) != 0) flags |= 0x40000000;
        if ((keys & 2) != 0) flags |= 0x80000000;
        if ((keys & 4) != 0) flags |= 0x200;
        if ((keys & 8) != 0) flags |= 0x100;
        *actions = flags;
    }
    if (output != NULL) *output = position;
    if (window == NULL) return 0;
    ClipWindowLinkObservedPilot* link = window->link;
    if (link == NULL) return -1;
    if (output != NULL && link->window != NULL) {
        ScreenToClient(link->window, &position);
        *output = position;
    }
    if (link->window != target) return 0x30001;
    if (g_idleControllerPilot->QueryMode() == 0 && g_cursorSuppressedPilot == -1 &&
        g_cursorSuppressionEnabledPilot != 0) return 0x30002;
    return 0;
}

// 20136dec. Cursor kind 5 depends on whether capture is active.
extern "C" DWORD __stdcall ReadPanelInteractionCursorPilot(TimerInteractionObservedPilot* record)
{
    static const WORD resources[] = { 0, 0x67, 0x68, 0x68, 0x67, 0x66, 0x66, 0x69 };
    DWORD kind = record->cursorKind;
    if (kind == 0 || kind >= sizeof(resources) / sizeof(resources[0])) return 0;
    WORD resource = kind == 5 && g_timerActivityFirstPilot != NULL ? 0x69 : resources[kind];
    return reinterpret_cast<DWORD>(LoadPanelCursorPilot(g_panelCursorModulePilot, MAKEINTRESOURCEA(resource)));
}

struct PanelHoverPacketPilot {
    DWORD size, field04;
    HWND window;
    DWORD marker;
    RECT bounds;
    HMODULE module;
    const char* text;
    DWORD unknown28[2];
};
C_ASSERT(sizeof(PanelHoverPacketPilot) == 0x30);
C_ASSERT(offsetof(PanelHoverPacketPilot, text) == 0x24);

// 20136c32. The receiver can modify the packet in the synchronous 0x408 call.
static int SendPanelHoverPacketPilot(ClipWindowObservedPilot* window, UINT message,
    LONG width, LONG height, const char* text, UINT resource)
{
    ClipWindowLinkObservedPilot* link = window->link;
    if (link == NULL || link->hoverNotificationWindow == NULL) return -1;
    PanelHoverPacketPilot packet;
    packet.size = sizeof(packet);
    packet.field04 = 0;
    packet.window = link->window;
    packet.marker = 0x0eb1baba;
    packet.text = NULL;
    if (message == 0x409)
        SendMessageA(link->hoverNotificationWindow, 0x408, 0, reinterpret_cast<LPARAM>(&packet));
    char localText[0x51];
    if (message == 0x404 || message == 0x409) {
        SetRect(&packet.bounds, 0, 0, width, height);
        packet.module = g_languageModulePilot;
        packet.text = text;
        if (text == NULL) {
            packet.text = localText;
            LoadSplashStringPilot(resource, localText, sizeof(localText));
        }
    }
    SendMessageA(link->hoverNotificationWindow, message, 0, reinterpret_cast<LPARAM>(&packet));
    return 0;
}

// 2013699b. The original window arrives in EDI.
extern "C" int __stdcall CreatePanelHoverWindowPilot(ClipWindowObservedPilot* window)
{
    ClipWindowLinkObservedPilot* link = window->link;
    if (link == NULL) return -1;
    link->hoverRecord = 0;
    HWND tooltip = CreateWindowExA(WS_EX_TOPMOST, "tooltips_class32", NULL, 1,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, g_panelCursorModulePilot, NULL);
    link->hoverNotificationWindow = tooltip;
    if (tooltip == NULL) return -1;
    return SendPanelHoverPacketPilot(window, 0x404, 0, 0, NULL, 0);
}

// 20136bc9. The original window and record arrive in EAX and EDX.
static int SetPanelHoverRecordPilot(ClipWindowObservedPilot* window, DWORD hover)
{
    if (window == NULL || window->link == NULL) return -1;
    window->link->hoverRecord = hover;
    if (hover == 0) return SendPanelHoverPacketPilot(window, 0x409, 0, 0, NULL, 0);
    TimerInteractionObservedPilot* record = reinterpret_cast<TimerInteractionObservedPilot*>(hover);
    const char* text = record->releaseOwner != NULL ?
        static_cast<const char*>(record->releaseOwner->hoverText) : NULL;
    return SendPanelHoverPacketPilot(window, 0x409, window->surfaceWidth, window->surfaceHeight,
        text, record->inputGroup);
}

// 20136ae6. Original window in ESI and hover record in EDI.
static int RefreshPanelHoverTextPilot(ClipWindowObservedPilot* window, DWORD recordKey)
{
    if (g_currentPanelHoverPilot == recordKey && g_activeClipWindowPilot != NULL)
        SetPanelHoverRecordPilot(g_activeClipWindowPilot, recordKey);
    if (window == NULL) return 0;
    if (window->link == NULL) return -1;
    if (window->link->hoverRecord == recordKey) return SetPanelHoverRecordPilot(window, recordKey);
    return 0;
}

// 20136532. The original window arrives in ESI.
extern "C" void __stdcall UpdatePanelHoverPilot(ClipWindowObservedPilot* window, DWORD hover)
{
    g_activeClipWindowPilot = NULL;
    g_currentPanelHoverPilot = 0;
    if (window == NULL || window->link == NULL) return;
    ClipWindowLinkObservedPilot* link = window->link;
    g_activeClipWindowPilot = window;
    g_currentPanelHoverPilot = hover;
    if (g_panelHoverResetPilot != 0) {
        hover = 0;
        g_panelHoverResetPilot = 0;
    }
    if (g_idleControllerPilot->QueryMode() == 0 && g_cursorSuppressedPilot == -1 &&
        g_cursorSuppressionEnabledPilot != 0 && g_idleControllerPilot->QueryField38()) {
        if (link->hoverRecord != 0) SetPanelHoverRecordPilot(window, 0);
    } else if (link->hoverRecord != hover) {
        if (window->link != NULL)
            SendMessageA(window->link->hoverNotificationWindow, 0x41c, 0, 0);
        SetPanelHoverRecordPilot(window, hover);
    }
}
