#include "heap_boundary.h"
#include <string.h>

HDC g_panelMemoryDcPilot; // 2020d134
ATOM g_simAWindowPropertyAtomPilot; // 2020e2e0

struct PanelSurfaceGroupObservedPilot {
    DWORD key;
    PanelPixelsObservedPilot pixels;
    DWORD trailingSurfaceWord;
    CallbackArrayObservedPilot windows;
};
C_ASSERT(sizeof(PanelSurfaceGroupObservedPilot) == 0x40);
C_ASSERT(offsetof(PanelSurfaceGroupObservedPilot, pixels) == 4);
C_ASSERT(offsetof(PanelSurfaceGroupObservedPilot, windows) == 0x24);
static CallbackArrayObservedPilot g_panelSurfaceGroupsPilot = {
    0, 0, 32, sizeof(PanelSurfaceGroupObservedPilot), 0, 0, NULL
}; // 201f94e0

static PanelSurfaceGroupObservedPilot* FindPanelSurfaceGroupPilot(DWORD key)
{
    for (int i = 0; i < g_panelSurfaceGroupsPilot.count; ++i) {
        PanelSurfaceGroupObservedPilot* group = reinterpret_cast<PanelSurfaceGroupObservedPilot*>(
            CallbackArrayRecordPilot(&g_panelSurfaceGroupsPilot, i));
        if (group->key == key) return group;
    }
    return NULL;
}

static ClipWindowObservedPilot** FindPanelSurfaceWindowPilot(
    CallbackArrayObservedPilot* windows, ClipWindowObservedPilot* window)
{
    for (int i = 0; i < windows->count; ++i) {
        ClipWindowObservedPilot** entry = reinterpret_cast<ClipWindowObservedPilot**>(
            CallbackArrayRecordPilot(windows, i));
        if (*entry == window) return entry;
    }
    return NULL;
}

// 2012c92c. DC creation is independent of whether the window has a shared key.
extern "C" void __stdcall RegisterPanelSurfacePilot(PanelUpdateContextObservedPilot* context)
{
    if (g_panelMemoryDcPilot == NULL) g_panelMemoryDcPilot = CreateCompatibleDC(NULL);
    DWORD key = context->window->regionUnionFlag;
    if (key == 0) return;
    PanelSurfaceGroupObservedPilot* group = FindPanelSurfaceGroupPilot(key);
    if (group == NULL) {
        group = static_cast<PanelSurfaceGroupObservedPilot*>(AppendCallbackArrayRecordPilot(&g_panelSurfaceGroupsPilot));
        group->key = context->window->regionUnionFlag;
        memcpy(reinterpret_cast<BYTE*>(group) + 4,
            reinterpret_cast<const BYTE*>(context->window) + 0x78, 32);
        group->windows.growth = 32;
        group->windows.stride = sizeof(ClipWindowObservedPilot*);
    }
    if (FindPanelSurfaceWindowPilot(&group->windows, context->window) == NULL) {
        ClipWindowObservedPilot** entry = static_cast<ClipWindowObservedPilot**>(
            AppendCallbackArrayRecordPilot(&group->windows));
        *entry = context->window;
    }
}

// 2012c9f7. The original context is in EAX.
extern "C" void __stdcall UnregisterPanelSurfacePilot(PanelUpdateContextObservedPilot* context)
{
    ClipWindowObservedPilot* window = context->window;
    if (window->regionUnionFlag == 0) {
        typedef void (__stdcall *PanelSurfaceCallbackPilot)(int, ClipWindowObservedPilot*, DWORD, DWORD);
        PanelSurfaceCallbackPilot callback = *reinterpret_cast<PanelSurfaceCallbackPilot*>(
            reinterpret_cast<BYTE*>(window) + 0x18);
        if (callback != NULL) callback(6, window, 0, 0);
        HGDIOBJ bitmap = context->link->bitmap;
        if (bitmap != NULL) {
            DeleteObject(bitmap);
            context->link->bitmap = NULL;
        }
        return;
    }
    PanelSurfaceGroupObservedPilot* group = FindPanelSurfaceGroupPilot(window->regionUnionFlag);
    if (group == NULL) return;
    ClipWindowObservedPilot** entry = FindPanelSurfaceWindowPilot(&group->windows, window);
    if (entry != NULL) RemoveCallbackArrayRecordPilot(&group->windows, entry);
    if (group->windows.count == 0) {
        FreePrivateHeapSlotPilot(&group->windows.records);
        group->windows.capacity = 0;
        RemoveCallbackArrayRecordPilot(&g_panelSurfaceGroupsPilot, group);
    }
}

// 2012d3fb. GetWindow starts at the last sibling and walks backwards.
extern "C" void __stdcall UpdateRelatedPanelSurfacesPilot(PanelUpdateContextObservedPilot* context)
{
    ClipWindowLinkObservedPilot* link = context->window->link;
    if (link == NULL || link->window == NULL) return;
    for (HWND sibling = GetWindow(link->window, GW_HWNDLAST); sibling != NULL;
        sibling = GetWindow(sibling, GW_HWNDPREV)) {
        if (g_simAWindowPropertyAtomPilot == 0) g_simAWindowPropertyAtomPilot = GlobalAddAtomA("SimAWind");
        ClipWindowObservedPilot* target = static_cast<ClipWindowObservedPilot*>(
            GetPropA(sibling, MAKEINTATOM(g_simAWindowPropertyAtomPilot)));
        if (target == NULL) continue;
        ClipWindowObservedPilot* owner = context->window;
        if (target == owner || (target->flags & 0x80000100) != 0x80000100) continue;
        LONG width = owner->surfaceWidth, height = owner->surfaceHeight;
        if (target->relatedWidth < width - 2 || target->relatedWidth > width + 2 ||
            target->relatedHeight < height - 2 || target->relatedHeight > height + 2) {
            PostMessageA(sibling, 0x479, 0, 0);
            continue;
        }
        DWORD destinationState[9]; // Full observed output span of ordinal 68.
        if (BeginGraphicsPanelUpdateSourcePilot(target, 0, destinationState) != 0) return;
        owner = context->window;
        LONG left = target->left, top = target->top;
        if ((owner->flags & 0x60000000) == 0) {
            left += owner->left;
            top += owner->top;
        }
        RECT destinationRect = {left, top, left + target->width, top + target->height};
        RECT sourceRect = {0, 0, target->width, target->height};
        const PanelPixelsObservedPilot* source = reinterpret_cast<const PanelPixelsObservedPilot*>(
            reinterpret_cast<const BYTE*>(target) + 0x78);
        TransferPanelUpdateSourcePilot(target, reinterpret_cast<PanelPixelsObservedPilot*>(destinationState),
            source, &destinationRect, &sourceRect);
        EndGraphicsPanelUpdateSourcePilot(target, 0);
    }
}

// 2012d2cd. Its first stack argument is unused by the original.
extern "C" void __stdcall NotifyRelatedPanelSurfacesPilot(void*, ClipWindowObservedPilot* window)
{
    if ((window->flags & 0x1000) != 0)
        UpdateRelatedPanelSurfacesPilot(reinterpret_cast<PanelUpdateContextObservedPilot*>(window->link->geometryLink));
}

// 2012d335. Original context in EAX and destination DC on the stack.
extern "C" void __stdcall StretchPanelBitmapPilot(PanelUpdateContextObservedPilot* context, HDC destination)
{
    RECT client;
    GetClientRect(context->link->window, &client);
    HGDIOBJ previous = SelectObject(g_panelMemoryDcPilot, context->link->bitmap);
    ClipWindowObservedPilot* window = context->window;
    StretchBlt(destination, 0, 0, client.right, client.bottom, g_panelMemoryDcPilot, 0, 0,
        window->surfaceWidth, window->surfaceHeight, SRCCOPY);
    SelectObject(g_panelMemoryDcPilot, previous);
}

// 2012d3b0. Inclusive bounds, with x rounded down to a multiple of four.
extern "C" void __stdcall StretchPanelRectanglePilot(const RECT* rect, HDC destination)
{
    LONG alignedLeft = rect->left & ~3L;
    LONG width = rect->right - alignedLeft + 1;
    LONG height = rect->bottom - rect->top + 1;
    StretchBlt(destination, alignedLeft, rect->top, width, height,
        g_panelMemoryDcPilot, alignedLeft, rect->top, width, height, SRCCOPY);
}
