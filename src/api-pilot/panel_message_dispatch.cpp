#include <imm.h>

// API handler boundaries grouped by the subsystem that owns their bodies.
extern "C" LRESULT __stdcall PanelChildKeyboardSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 2013ff37
extern "C" LRESULT __stdcall PanelChildCommandSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 20141853
extern "C" LRESULT __stdcall PanelChildSystemCommandSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 20141095
extern "C" LRESULT __stdcall PanelChildMenuSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 20141195
extern "C" LRESULT __stdcall PanelChildNotifySourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 2013ff66
extern "C" LRESULT __stdcall PanelChildMoveSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 20140f65
extern "C" LRESULT __stdcall PanelChildSizeSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 20140dc1
extern "C" LRESULT __stdcall PanelChildMinMaxSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 20141131
extern "C" LRESULT __stdcall PanelChildExitSizeMoveSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 201410e0
extern "C" LRESULT __stdcall PanelChildRefreshSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 201401d3
extern "C" LRESULT __stdcall PanelChildAuxiliaryMessageSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 20140153
extern "C" LRESULT __stdcall PanelChildRelatedSizeSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 20141fd6
extern "C" LRESULT __stdcall PanelDialogKeyboardSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 2013f4fd
extern "C" LRESULT __stdcall PanelDialogMoveSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 2013f5ed
extern "C" LRESULT __stdcall PanelDialogAuxiliaryMessageSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 2013f499
extern "C" LRESULT __stdcall PanelPointerMessageSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 20141d13
extern "C" LRESULT __stdcall PanelGeneralPointerSourcePilot(PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM); // 20141a0b
extern "C" LRESULT __stdcall PanelPointerAtClientPointSourcePilot(
    PanelUpdateContextObservedPilot*, UINT, WPARAM, LPARAM, const POINT*); // 2012cb9d
extern "C" void __stdcall PreparePanelLeftButtonSourcePilot(HWND, WPARAM, LONG, LONG); // 2013d488
extern "C" void __stdcall RefreshPanelSurfaceSourcePilot(PanelUpdateContextObservedPilot*); // 2012b9af

extern "C" void __stdcall SetPanelControlsCoordinatesPilot(float, float); // controls.dll 46
extern "C" void __stdcall PressPanelControlsButtonPilot(DWORD, DWORD); // controls.dll 47
extern "C" void __stdcall ReleasePanelControlsButtonPilot(DWORD, DWORD); // controls.dll 48
extern DWORD g_panelPointerDispatchBlockedPilot; // 2020de48
extern DWORD g_panelForwardInputEnabledPilot; // 201f7168
extern BYTE g_panelPointerClickStatePilot; // 201f7299
extern DWORD g_panelPendingPointerStatePilot; // 2020d128
extern void* g_mainWindowCommandCallbacksPilot; // 2020bb70

static POINT UnpackPanelPointPilot(LPARAM lParam)
{
    POINT point = { static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)) };
    return point;
}

static LRESULT DefaultPanelMessagePilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProcA(context->windowHandle, message, wParam, lParam);
}

static bool SendPanelControlsPointPilot()
{
    IdleControllerBoundaryPilot* controller = g_mainWindowControllerPilot;
    if (controller == NULL) return true;
    POINT position;
    if (!GetCursorPos(&position) ||
        !ScreenToClient(ReadMainWindowSizingStatePilot(controller).window, &position)) return false;
    RECT bounds;
    GraphicsClipSlotPilot<GraphicsClipRectPilot>(controller, 0x34)(controller, &bounds);
    SetPanelControlsCoordinatesPilot(
        static_cast<float>(position.x * 2 - bounds.right) / bounds.right,
        static_cast<float>(position.y * 2 - bounds.bottom) / bounds.bottom);
    return true;
}

static bool PanelPointerEnabledPilot()
{
    return GraphicsClipSlotPilot<GraphicsClipFlagPilot>(g_idleControllerPilot, 0x20)(g_idleControllerPilot);
}

static bool DispatchInlinePanelInputPilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam, POINT point)
{
    if (!PanelPointerEnabledPilot() || g_panelPointerDispatchBlockedPilot != 0) return false;
    PanelInputMessagePilot input = { context->windowHandle, message, wParam, lParam, point };
    bool handled = DispatchPanelInputPilot(context->window, &input, NULL, context->window) != 0;
    if (context->link->hoverNotificationWindow != NULL && g_panelForwardInputEnabledPilot != 0) {
        MSG copy;
        copy.hwnd = context->windowHandle;
        copy.message = message;
        copy.wParam = wParam;
        copy.lParam = lParam;
        copy.time = GetMessageTime();
        copy.pt = UnpackPanelPointPilot(GetMessagePos());
        SendMessageA(context->link->hoverNotificationWindow, 0x407, 0, reinterpret_cast<LPARAM>(&copy));
    }
    return handled;
}

static LRESULT HandleChildNonclientButtonPilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    g_cursorSuppressedPilot = 100;
    if (message == 0xa1 || message == 0xa4) RaiseClipWindowInGroupPilot(context->window);
    POINT point = UnpackPanelPointPilot(lParam);
    ScreenToClient(context->windowHandle, &point);
    int controls = QueryControlsCursorOverridePilot();
    if (controls != 0 && message != 0xa4 && message != 0xa5) {
        if (!SendPanelControlsPointPilot()) return 0;
        if (message == 0xa1) PressPanelControlsButtonPilot(0, 0);
        if (message == 0xa2) ReleasePanelControlsButtonPilot(0, 0);
        return DefaultPanelMessagePilot(context, message, wParam, lParam);
    }
    if (message == 0xa1 && wParam == 0x14) {
        if (g_mainWindowEventPilot != NULL)
            g_mainWindowEventPilot(0x2000a, reinterpret_cast<DWORD>(context->window), 0);
        return 0;
    }
    if (message == 0xa1) PreparePanelLeftButtonSourcePilot(context->windowHandle, wParam, point.x, point.y);
    ReconcileTimerInteractionPilot(0);
    if (message == 0xa1 || message == 0xa4) g_panelPointerClickStatePilot = 2;
    PanelPointerEnabledPilot();
    bool handled = DispatchInlinePanelInputPilot(context, message, wParam, lParam, point);
    ReconcileTimerInteractionPilot(1);
    return handled ? 0 : DefaultPanelMessagePilot(context, message, wParam, lParam);
}

static LRESULT HandleDialogInlinePointerPilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_MOUSEMOVE) g_cursorSuppressedPilot = 100;
    POINT point = UnpackPanelPointPilot(lParam);
    if (QueryControlsCursorOverridePilot() != 0)
        return SendPanelControlsPointPilot() ? DefaultPanelMessagePilot(context, message, wParam, lParam) : 0;
    ReconcileTimerInteractionPilot(0);
    bool mode = PanelPointerEnabledPilot();
    if (message == 0x46a && mode) {
        if (g_panelPendingPointerStatePilot != 0) {
            DispatchCallbackListSourcePilot(&g_mainWindowCommandCallbacksPilot, 400, 0);
            g_panelPendingPointerStatePilot = 0;
        }
        return 0;
    }
    bool handled = DispatchInlinePanelInputPilot(context, message, wParam, lParam, point);
    if (message == 0x46a) ReconcileTimerInteractionPilot(1);
    return handled ? 0 : DefaultPanelMessagePilot(context, message, wParam, lParam);
}

// 20141f7c.
static LRESULT RefreshPanelMessageSurfacePilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (context->window != NULL && *reinterpret_cast<DWORD*>(
        reinterpret_cast<BYTE*>(context->window) + 0x18) != 0)
        RefreshPanelSurfaceSourcePilot(context);
    return DefaultPanelMessagePilot(context, message, wParam, lParam);
}

struct PanelOwnedInputPacketPilot {
    PanelInputMessagePilot input;
    DWORD mode;
    DWORD group;
};
C_ASSERT(offsetof(PanelOwnedInputPacketPilot, mode) == 0x18);

// 20141fae. ESI originally carries the owned packet, EAX the panel context.
static LRESULT DispatchOwnedPanelInputPacketPilot(PanelUpdateContextObservedPilot* context, LPARAM lParam)
{
    PanelOwnedInputPacketPilot* packet = reinterpret_cast<PanelOwnedInputPacketPilot*>(lParam);
    if (packet != NULL) {
        DispatchPanelContextClickPilot(reinterpret_cast<ClipGeometryLinkObservedPilot*>(context),
            &packet->input, packet->group, packet->mode);
        HeapFree(g_privateHeapPilot, 0, packet);
    }
    return 0;
}

// 20142211.
extern "C" LRESULT __stdcall DispatchChildPanelMessagePilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        RegisterPanelSurfacePilot(context);
        ImmAssociateContext(context->windowHandle, NULL);
        break;
    case WM_PAINT: return HandlePanelPaintPilot(context);
    case WM_ERASEBKGND: return 1;
    case WM_SETCURSOR: return HandlePanelSetCursorPilot(context, wParam, lParam);
    case 0x22: ReorderClipWindowsPilot(); break;
    case WM_KEYDOWN: return MainWindowProcedurePilot(g_hookMainWindowPilot, message, wParam, lParam);
    case 0x101: case 0x102: case 0x104: case 0x105: case 0x106:
        return PanelChildKeyboardSourcePilot(context, message, wParam, lParam);
    case WM_COMMAND: return PanelChildCommandSourcePilot(context, message, wParam, lParam);
    case WM_SYSCOMMAND: return PanelChildSystemCommandSourcePilot(context, message, wParam, lParam);
    case 0x116: return PanelChildMenuSourcePilot(context, message, wParam, lParam);
    case WM_NOTIFY: return PanelChildNotifySourcePilot(context, message, wParam, lParam);
    case WM_MOVE: return PanelChildMoveSourcePilot(context, message, wParam, lParam);
    case WM_SIZE: return PanelChildSizeSourcePilot(context, message, wParam, lParam);
    case WM_GETMINMAXINFO: return PanelChildMinMaxSourcePilot(context, message, wParam, lParam);
    case WM_EXITSIZEMOVE: return PanelChildExitSizeMoveSourcePilot(context, message, wParam, lParam);
    case 0x85: case 0x86: return 0;
    case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6:
        return HandleChildNonclientButtonPilot(context, message, wParam, lParam);
    case 0xab:
        g_cursorSuppressedPilot = 100;
        RaiseClipWindowInGroupPilot(context->window);
        return PanelGeneralPointerSourcePilot(context, message, wParam, lParam);
    case 0xa7: case 0xa8: case 0xa9: case 0xac: case 0xad: case 0x20a:
        g_cursorSuppressedPilot = 100;
        // fall through
    case 0x84: case 0xa0:
        return PanelGeneralPointerSourcePilot(context, message, wParam, lParam);
    case 0x201: case 0x202: case 0x203: case 0x204: case 0x205: case 0x206:
    case 0x207: case 0x208: case 0x209: case 0x20b: case 0x20c: case 0x20d:
        g_cursorSuppressedPilot = 100;
        // fall through
    case 0x200: case 0x46a:
        return PanelPointerMessageSourcePilot(context, message, wParam, lParam);
    case 0x466: return HandlePanelBitmapMessagePilot(context, wParam, lParam);
    case 0x467: return PanelChildRefreshSourcePilot(context, message, wParam, lParam);
    case 0x46c: return RefreshPanelMessageSurfacePilot(context, message, wParam, lParam);
    case 0x475: case 0x47a: return PanelChildAuxiliaryMessageSourcePilot(context, message, wParam, lParam);
    case 0x478: return DispatchOwnedPanelInputPacketPilot(context, lParam);
    case 0x479:
        if ((context->window->flags & 0x100) == 0) return 0;
        return PanelChildRelatedSizeSourcePilot(context, message, wParam, lParam);
    case 0x47c: context->skipNextPaint = 1; break;
    }
    return DefaultPanelMessagePilot(context, message, wParam, lParam);
}

// 2013f6c3.
extern "C" LRESULT __stdcall DispatchDialogPanelMessagePilot(
    PanelUpdateContextObservedPilot* context, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_MOVE: return PanelDialogMoveSourcePilot(context, message, wParam, lParam);
    case WM_SETFOCUS: ImmAssociateContext(context->windowHandle, NULL); break;
    case WM_PAINT: return HandlePanelPaintPilot(context);
    case WM_SETCURSOR: return HandlePanelSetCursorPilot(context, wParam, lParam);
    case WM_GETDLGCODE: return 0x81;
    case WM_KEYDOWN: return PanelDialogKeyboardSourcePilot(context, message, wParam, lParam);
    case WM_CHAR: return 0;
    case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6:
    case 0xa7: case 0xa8: case 0xa9: case 0x20a:
        g_cursorSuppressedPilot = 100;
        RaiseClipWindowInGroupPilot(context->window);
        // fall through
    case 0x84: case 0xa0: {
        POINT point = UnpackPanelPointPilot(lParam);
        ScreenToClient(context->windowHandle, &point);
        return PanelPointerAtClientPointSourcePilot(context, message, wParam, lParam, &point);
    }
    case 0x200: case 0x46a: return HandleDialogInlinePointerPilot(context, message, wParam, lParam);
    case 0x201: case 0x202: case 0x203: case 0x204: case 0x205: case 0x206:
    case 0x207: case 0x208: case 0x209:
        g_cursorSuppressedPilot = 100;
        return PanelPointerMessageSourcePilot(context, message, wParam, lParam);
    case 0x466: return HandleFocusedPanelBitmapMessagePilot(context, wParam, lParam);
    case 0x46c: return RefreshPanelMessageSurfacePilot(context, 0x466, wParam, lParam);
    case 0x474: return PanelDialogAuxiliaryMessageSourcePilot(context, message, wParam, lParam);
    case 0x478: return DispatchOwnedPanelInputPacketPilot(context, lParam);
    }
    return DefaultPanelMessagePilot(context, message, wParam, lParam);
}

extern void* g_panelContextTablePilot[]; // 200f30b0
// Historical source symbol retained. The handle belongs to the menu interface.
extern "C" void __stdcall ReleasePanelContextGraphicsSourcePilot(ClipWindowObservedPilot*, DWORD); // 20138f47 and slot +0x70
extern DWORD g_countedChildPanelWindowsPilot; // 2020dee4
typedef void (__stdcall *RelatedPanelSurfaceCallbackPilot)(void*, ClipWindowObservedPilot*);
extern "C" void __stdcall RegisterRelatedPanelSurfaceCallbackPilot(RelatedPanelSurfaceCallbackPilot); // 200f6451, owner 2020bb94
extern "C" void __stdcall UnregisterRelatedPanelSurfaceCallbackPilot(DWORD, RelatedPanelSurfaceCallbackPilot); // 200f658f

static LPCSTR PanelWindowPropertyPilot()
{
    if (g_simAWindowPropertyAtomPilot == 0) g_simAWindowPropertyAtomPilot = GlobalAddAtomA("SimAWind");
    return MAKEINTATOM(g_simAWindowPropertyAtomPilot);
}

// 2012b5e7 and 2013f246 share the same final field values.
static PanelUpdateContextObservedPilot* AllocatePanelContextPilot(HWND hwnd, ClipWindowObservedPilot* window,
    ClipWindowLinkObservedPilot* link)
{
    void* memory;
    AllocatePrivateHeapMemoryPilot(1, sizeof(PanelUpdateContextObservedPilot), &memory, g_factoryZeroInitializeBoundaryPilot);
    if (memory == NULL) return NULL;
    PanelUpdateContextObservedPilot* context = static_cast<PanelUpdateContextObservedPilot*>(memory);
    context->unknown00 = reinterpret_cast<DWORD>(g_panelContextTablePilot);
    context->windowHandle = hwnd;
    context->window = window;
    context->link = link;
    ZeroMemory(context->unknown10, sizeof(context->unknown10));
    context->bitmapDrawPending = 0;
    context->skipNextPaint = 0;
    context->contextField44 = 0;
    return context;
}

// 2013f296, target of table slot zero. Placement in that table remains external.
extern "C" PanelUpdateContextObservedPilot* __stdcall DestroyPanelContextPilot(
    PanelUpdateContextObservedPilot* context, DWORD flags)
{
    DWORD graphicsHandle = context->contextField44;
    context->unknown00 = reinterpret_cast<DWORD>(g_panelContextTablePilot);
    if (graphicsHandle != 0) ReleasePanelContextGraphicsSourcePilot(context->window, graphicsHandle);
    context->contextField44 = 0;
    if ((flags & 1) != 0) HeapFree(g_privateHeapPilot, 0, context);
    return context;
}

// 20142031. FS98CHILD is registered with this procedure.
extern "C" LRESULT CALLBACK MainChildWindowProcedurePilot(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PanelUpdateContextObservedPilot* context = NULL;
    if (message == WM_NCCREATE) {
        ClipWindowObservedPilot* window = *reinterpret_cast<ClipWindowObservedPilot**>(lParam);
        ClipWindowLinkObservedPilot* link = window->link;
        link->window = hwnd;
        SetPropA(hwnd, PanelWindowPropertyPilot(), window);
        if (window->panelKind == 3 && ++g_countedChildPanelWindowsPilot == 1)
            RegisterRelatedPanelSurfaceCallbackPilot(NotifyRelatedPanelSurfacesPilot);
        context = reinterpret_cast<PanelUpdateContextObservedPilot*>(link->geometryLink);
        if (context == NULL) {
            context = AllocatePanelContextPilot(hwnd, window, link);
            link->geometryLink = reinterpret_cast<ClipGeometryLinkObservedPilot*>(context);
            link->contextState = 0xffffffffu;
        }
    } else {
        ClipWindowObservedPilot* window = static_cast<ClipWindowObservedPilot*>(GetPropA(hwnd, PanelWindowPropertyPilot()));
        if (window == NULL) return DefWindowProcA(hwnd, message, wParam, lParam);
        if (message == WM_NCDESTROY) {
            if (window->panelKind == 3 && --g_countedChildPanelWindowsPilot == 0)
                UnregisterRelatedPanelSurfaceCallbackPilot(0x19, NotifyRelatedPanelSurfacesPilot);
            ClipWindowLinkObservedPilot* link = window->link;
            if (link != NULL) {
                context = reinterpret_cast<PanelUpdateContextObservedPilot*>(link->geometryLink);
                if (context != NULL) {
                    UnregisterPanelSurfacePilot(context);
                    typedef void (__thiscall *DeletePanelContextPilot)(void*, DWORD);
                    GraphicsClipSlotPilot<DeletePanelContextPilot>(context, 0)(context, 1);
                }
                link->geometryLink = NULL;
            }
            RemovePropA(hwnd, PanelWindowPropertyPilot());
            context = NULL;
        } else if (window->link != NULL) {
            context = reinterpret_cast<PanelUpdateContextObservedPilot*>(window->link->geometryLink);
        }
    }
    if (context != NULL && g_messageQuitRequestedPilot == 0)
        return DispatchChildPanelMessagePilot(context, message, wParam, lParam);
    return DefWindowProcA(hwnd, message, wParam, lParam);
}

// 2013f2e5. 0x467 creates the FS98DLG3D context from the LPARAM window.
extern "C" LRESULT CALLBACK DialogPanelWindowProcedurePilot(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == 0x467) {
        ClipWindowObservedPilot* window = reinterpret_cast<ClipWindowObservedPilot*>(lParam);
        ClipWindowLinkObservedPilot* link = window->link;
        PanelUpdateContextObservedPilot* context = AllocatePanelContextPilot(hwnd, window, link);
        if (context == NULL) return 0;
        link->geometryLink = reinterpret_cast<ClipGeometryLinkObservedPilot*>(context);
        SetPropA(hwnd, PanelWindowPropertyPilot(), window);
        window->flags |= 0xc8000000;
        BYTE* storage = reinterpret_cast<BYTE*>(window);
        *reinterpret_cast<DWORD*>(storage + 0x100) = 1;
        ZeroMemory(storage + 0x60, 0x10);
        window->regionUnionFlag = *reinterpret_cast<DWORD*>(storage + 0x18) != 0;
        RegisterPanelSurfacePilot(context);
        RefreshPanelSurfaceSourcePilot(context);
    } else {
        ClipWindowObservedPilot* window = static_cast<ClipWindowObservedPilot*>(GetPropA(hwnd, PanelWindowPropertyPilot()));
        if (window != NULL) {
            PanelUpdateContextObservedPilot* context =
                reinterpret_cast<PanelUpdateContextObservedPilot*>(window->link->geometryLink);
            if (context != NULL) return DispatchDialogPanelMessagePilot(context, message, wParam, lParam);
        }
    }
    return DefWindowProcA(hwnd, message, wParam, lParam);
}
