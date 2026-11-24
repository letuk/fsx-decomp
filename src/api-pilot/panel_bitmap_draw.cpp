#include <malloc.h>

// 2012d2f1. The original context is in ECX and output rectangle in EAX.
extern "C" void __stdcall ReadPanelSurfaceBoundsPilot(PanelUpdateContextObservedPilot* context, RECT* output)
{
    ClipWindowObservedPilot* window = context->window;
    if ((window->flags & 0x60000000) == 0) {
        output->left = window->left;
        output->top = window->top;
    } else {
        output->left = 0;
        output->top = 0;
    }
    output->right = output->left + window->surfaceWidth;
    output->bottom = output->top + window->surfaceHeight;
}

// 2011ee97. The optional rectangle is in window-local coordinates.
extern "C" void __stdcall RequestPanelGraphicsUpdatePilot(ClipWindowObservedPilot* window, const RECT* bounds)
{
    ClipWindowLinkObservedPilot* link = window->link;
    if (link == NULL || window->cachedGraphicsWindow == NULL) return;
    RECT local, surface;
    ReadPanelSurfaceBoundsPilot(reinterpret_cast<PanelUpdateContextObservedPilot*>(link->geometryLink), &local);
    ReadPanelSurfaceBoundsPilot(reinterpret_cast<PanelUpdateContextObservedPilot*>(link->geometryLink), &surface);
    LONG width = bounds == NULL ? local.right - local.left : bounds->right - bounds->left;
    LONG height = bounds == NULL ? local.bottom - local.top : bounds->bottom - bounds->top;
    if (width <= 0 || height <= 0) return;
    local.left = bounds == NULL ? 0 : bounds->left;
    local.top = bounds == NULL ? 0 : bounds->top;
    local.right = local.left + width;
    local.bottom = local.top + height;
    surface.left += local.left;
    surface.top += local.top;
    surface.right = surface.left + width;
    surface.bottom = surface.top + height;

    HWND root = GetAncestor(link->window, GA_ROOT);
    HWND child = link->window;
    if (root != child) MapWindowPoints(child, root, reinterpret_cast<POINT*>(&local), 2);
    bool wholeWindow = g_idleControllerPilot->QueryField38();
    void* graphics = window->cachedGraphicsWindow;
    typedef void (__thiscall *GraphicsPanelRequestPilot)(void*, const RECT*, const RECT*, HWND, DWORD);
    GraphicsClipSlotPilot<GraphicsPanelRequestPilot>(graphics, 0x84)(
        graphics, wholeWindow ? NULL : &surface, wholeWindow ? NULL : &local, root, 0);
}

// 2012c719. The original handles stack overflow only during the dynamic
// allocation and returns from that handler before restoring the selection.
extern "C" void __fastcall DrawPanelBitmapSourcePilot(PanelUpdateContextObservedPilot* context)
{
    ClipWindowObservedPilot* window = context->window;
    DWORD flags = window->flags;
    if ((flags & 0x380) != 0) return;

    if ((flags & 0x40000000) == 0) {
        context->bitmapDrawPending = 1;
    } else if (window->regionUnionFlag != 0) {
        RequestPanelGraphicsUpdatePilot(window, NULL);
        context->bitmapDrawPending = 0;
    } else {
        RECT client;
        GetClientRect(context->link->window, &client);
        window = context->window;
        if (window->surfaceWidth != client.right - client.left ||
            window->surfaceHeight != client.bottom - client.top) return;

        if ((window->flags & 0x10000000) == 0) {
            HDC destination = GetDC(context->link->window);
            StretchPanelBitmapPilot(context, destination);
            ReleaseDC(context->link->window, destination);
        } else if (context->link->bitmap == NULL) {
            RequestPanelGraphicsUpdatePilot(window, NULL);
            context->bitmapDrawPending = 0;
        } else {
            HGDIOBJ previous = SelectObject(g_panelMemoryDcPilot, context->link->bitmap);
            DWORD bytes = GetRegionData(context->link->updateRegion, 0, NULL);
            if (bytes != 0) {
                RGNDATA* data;
                __try {
                    data = static_cast<RGNDATA*>(_alloca(bytes));
                } __except (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
                    _resetstkoflw();
                    return;
                }
                if (GetRegionData(context->link->updateRegion, bytes, data) != 0 &&
                    data->rdh.dwSize == 0x20 && data->rdh.iType == 1) {
                    HDC destination = GetDC(context->link->window);
                    const RECT* rectangles = reinterpret_cast<const RECT*>(data->Buffer);
                    for (DWORD i = 0; i < data->rdh.nCount; ++i)
                        StretchPanelRectanglePilot(&rectangles[i], destination);
                    ReleaseDC(context->link->window, destination);
                }
            }
            SelectObject(g_panelMemoryDcPilot, previous);
        }
    }
    if ((context->window->flags & 0x10000000) != 0)
        SetRectRgn(context->link->updateRegion, 0, 0, 0, 0);
}

extern BYTE g_panelGraphicsNotificationPendingPilot; // 2020de87

// 2012c588. The original DC arrives in EDI and context on the stack.
extern "C" BOOL __stdcall PaintPanelClipPilot(PanelUpdateContextObservedPilot* context, HDC dc)
{
    RECT clip;
    GetClipBox(dc, &clip);
    if (clip.top == clip.bottom || clip.left == clip.right) return TRUE;

    if ((context->window->flags & 0x200) == 0 && g_idleControllerPilot->QueryMode() != 2) {
        ClipWindowObservedPilot* window = context->window;
        if ((window->flags & 0x40000000) != 0) {
            if (window->regionUnionFlag == 0 && context->link->bitmap != NULL) {
                StretchPanelBitmapPilot(context, dc);
                return TRUE;
            }
        } else {
            if ((g_idleControllerPilot->QueryField38() && g_idleControllerPilot->QueryMode() != 0) ||
                g_idleControllerPilot->QueryField34()) return FALSE;

            if (context->bitmapDrawPending != 0) {
                window = context->window;
                if (clip.left < 0) clip.left = 0;
                if (clip.top < 0) clip.top = 0;
                if (clip.right > window->surfaceWidth) clip.right = window->surfaceWidth;
                if (clip.bottom > window->surfaceHeight) clip.bottom = window->surfaceHeight;
                RequestPanelGraphicsUpdatePilot(window, &clip);
                return TRUE;
            }
        }
    }
    PatBlt(dc, clip.left, clip.top, clip.right - clip.left, clip.bottom - clip.top, BLACKNESS);
    return TRUE;
}

// 20140d00. Both original WM_PAINT dispatchers pass context in ESI.
extern "C" LRESULT __stdcall HandlePanelPaintPilot(PanelUpdateContextObservedPilot* context)
{
    LRESULT result = 1;
    g_idleControllerPilot->RefreshVisibleWindows();
    PAINTSTRUCT paint;
    BeginPaint(context->windowHandle, &paint);
    if (ReadMainWindowSizingStatePilot(g_mainWindowControllerPilot).inSizeMove != 0)
        context->skipNextPaint = 1;
    else if (context->skipNextPaint != 0)
        context->skipNextPaint = 0;
    else
        result = PaintPanelClipPilot(context, paint.hdc);
    EndPaint(context->windowHandle, &paint);
    return result;
}

static void NotifyPanelGraphicsControllerPilot()
{
    g_panelGraphicsNotificationPendingPilot = 0;
    void* graphics = GetGraphicsControllerSourcePilot();
    GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(graphics, 0x38)(graphics);
}

// 20140d7b and 2013f3ec. The original message context is in ESI.
extern "C" LRESULT __stdcall HandlePanelBitmapMessagePilot(
    PanelUpdateContextObservedPilot* context, WPARAM wParam, LPARAM lParam)
{
    if (g_panelGraphicsNotificationPendingPilot != 0) NotifyPanelGraphicsControllerPilot();
    DrawPanelBitmapSourcePilot(context);
    return DefWindowProcA(context->windowHandle, 0x466, wParam, lParam);
}

extern "C" LRESULT __stdcall HandleFocusedPanelBitmapMessagePilot(
    PanelUpdateContextObservedPilot* context, WPARAM wParam, LPARAM lParam)
{
    void* entry = context->window->cachedGraphicsWindow;
    if (entry != NULL && GraphicsClipSlotPilot<GraphicsClipFlagPilot>(entry, 0x44)(entry))
        NotifyPanelGraphicsControllerPilot();
    DrawPanelBitmapSourcePilot(context);
    HWND window = context->windowHandle;
    if (GetFocus() == window) {
        HDC dc = GetDC(window);
        RECT rect;
        SetRect(&rect, 0, 0, context->window->surfaceWidth, context->window->surfaceHeight);
        HBRUSH brush = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        FrameRect(dc, &rect, brush);
        ReleaseDC(context->windowHandle, dc);
    }
    return DefWindowProcA(context->windowHandle, 0x466, wParam, lParam);
}
