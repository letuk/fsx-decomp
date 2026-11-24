#include "heap_boundary.h"
#include "window_controller.h"
#include <malloc.h>
#include <stddef.h>
#include <string.h>

// API-side observations. The graphics receivers remain external.
extern "C" void* __cdecl GetGraphicsControllerSourcePilot(); // 200e1338, g2d.dll ordinal 4

template<class Method>
static Method GraphicsClipSlotPilot(void* receiver, unsigned int byteOffset)
{
    void** table = *static_cast<void***>(receiver);
    return reinterpret_cast<Method>(table[byteOffset / sizeof(void*)]);
}

struct ClipGeometryObservedPilot {
    BYTE unknown00[8];
    DWORD flags;
    BYTE unknown0c[0x24];
    LONG left, top;
    BYTE unknown38[0x44];
    LONG width, height;
};
struct ClipGeometryLinkObservedPilot {
    BYTE unknown00[8];
    ClipGeometryObservedPilot* geometry;
};
struct ClipWindowLinkObservedPilot {
    HWND window;
    HWND auxiliaryWindow;
    HWND hoverNotificationWindow;
    ClipGeometryLinkObservedPilot* geometryLink;
    DWORD contextState;
    DWORD hoverRecord;
    HGDIOBJ bitmap;
    DWORD bitmapGraphicsIndex;
    DWORD inputCursor;
    HRGN updateRegion;
    HRGN overlapRegion;
    DWORD field2c, field30;
    BYTE unknown34[0x10];
    void* allocation44;
};
struct ClipWindowObservedPilot {
    DWORD unknown00;
    DWORD panelKind;
    DWORD flags;
    BYTE unknown0c[0x24];
    LONG left, top, width, height;
    BYTE unknown40[0x30];
    DWORD groupKey;
    DWORD regionUnionFlag;
    BYTE unknown78[4];
    LONG surfaceWidth, surfaceHeight;
    BYTE unknown84[0x14];
    ClipWindowLinkObservedPilot* link;
    BYTE unknown9c[4];
    void* cachedGraphicsWindow;
    LONG relatedWidth, relatedHeight;
};
struct ClipWindowGroupObservedPilot {
    ClipWindowObservedPilot** entries;
    DWORD capacity;
    DWORD count;
    DWORD key;
};
C_ASSERT(offsetof(ClipGeometryObservedPilot, left) == 0x30);
C_ASSERT(offsetof(ClipGeometryObservedPilot, width) == 0x7c);
C_ASSERT(offsetof(ClipGeometryLinkObservedPilot, geometry) == 8);
C_ASSERT(offsetof(ClipWindowLinkObservedPilot, geometryLink) == 0xc);
C_ASSERT(offsetof(ClipWindowLinkObservedPilot, bitmap) == 0x18);
C_ASSERT(offsetof(ClipWindowLinkObservedPilot, inputCursor) == 0x20);
C_ASSERT(offsetof(ClipWindowLinkObservedPilot, hoverRecord) == 0x14);
C_ASSERT(offsetof(ClipWindowLinkObservedPilot, updateRegion) == 0x24);
C_ASSERT(offsetof(ClipWindowLinkObservedPilot, overlapRegion) == 0x28);
C_ASSERT(offsetof(ClipWindowLinkObservedPilot, allocation44) == 0x44);
C_ASSERT(offsetof(ClipWindowObservedPilot, groupKey) == 0x70);
C_ASSERT(offsetof(ClipWindowObservedPilot, width) == 0x38);
C_ASSERT(offsetof(ClipWindowObservedPilot, surfaceWidth) == 0x7c);
C_ASSERT(offsetof(ClipWindowObservedPilot, regionUnionFlag) == 0x74);
C_ASSERT(offsetof(ClipWindowObservedPilot, link) == 0x98);
C_ASSERT(offsetof(ClipWindowObservedPilot, cachedGraphicsWindow) == 0xa0);
C_ASSERT(offsetof(ClipWindowObservedPilot, relatedWidth) == 0xa4);
C_ASSERT(sizeof(ClipWindowGroupObservedPilot) == 0x10);

ClipWindowGroupObservedPilot* g_clipWindowGroupsPilot; // 2020de34
DWORD g_clipWindowGroupCapacityPilot; // 2020de38
DWORD g_clipWindowGroupCountPilot; // 2020de3c
HRGN g_sharedClipRegionFirstPilot; // 2020de40
HRGN g_sharedClipRegionSecondPilot; // 2020de44
extern int g_messageQuitRequestedPilot; // 2020e044
extern ClipWindowObservedPilot* g_activeClipWindowPilot; // 2020e060
static int g_reorderingClipWindowsPilot; // 2020e2d8

// 2011da64. Original group receiver in ESI.
extern "C" void __stdcall PrependClipWindowInGroupPilot(
    ClipWindowGroupObservedPilot* group, ClipWindowObservedPilot* window)
{
    for (DWORD i = 0; i < group->count; ++i) {
        if (group->entries[i] == window) return;
    }
    if (group->count != group->capacity) {
        for (DWORD i = group->count; i != 0; --i) group->entries[i] = group->entries[i - 1];
        group->entries[0] = window;
        ++group->count;
        return;
    }
    DWORD capacity = group->capacity + 15;
    void* memory;
    AllocatePrivateHeapMemoryPilot(1, capacity * sizeof(*group->entries), &memory, 0);
    ClipWindowObservedPilot** entries = static_cast<ClipWindowObservedPilot**>(memory);
    for (DWORD i = 0; i < group->count; ++i) entries[i + 1] = group->entries[i];
    entries[0] = window;
    if (group->entries != NULL) {
        HeapFree(g_privateHeapPilot, 0, group->entries);
        group->entries = NULL;
    }
    ++group->count;
    group->entries = entries;
    group->capacity = capacity;
}

// 2011db97. Group keys are ordered from largest to smallest.
extern "C" void __stdcall RegisterClipWindowPilot(ClipWindowObservedPilot* window)
{
    DWORD slot = 0;
    for (; slot < g_clipWindowGroupCountPilot; ++slot) {
        DWORD key = g_clipWindowGroupsPilot[slot].key;
        if (key == window->groupKey) {
            PrependClipWindowInGroupPilot(&g_clipWindowGroupsPilot[slot], window);
            return;
        }
        if (key < window->groupKey) break;
    }
    if (g_clipWindowGroupCountPilot == g_clipWindowGroupCapacityPilot) {
        DWORD capacity = g_clipWindowGroupCapacityPilot + 8;
        void* memory;
        AllocatePrivateHeapMemoryPilot(1, capacity * sizeof(ClipWindowGroupObservedPilot), &memory, 0);
        ClipWindowGroupObservedPilot* groups = static_cast<ClipWindowGroupObservedPilot*>(memory);
        for (DWORD i = 0; i < g_clipWindowGroupCountPilot; ++i) groups[i] = g_clipWindowGroupsPilot[i];
        if (g_clipWindowGroupsPilot != NULL) HeapFree(g_privateHeapPilot, 0, g_clipWindowGroupsPilot);
        g_clipWindowGroupCapacityPilot = capacity;
        g_clipWindowGroupsPilot = groups;
    }
    for (DWORD i = g_clipWindowGroupCountPilot; i > slot; --i)
        g_clipWindowGroupsPilot[i] = g_clipWindowGroupsPilot[i - 1];
    ++g_clipWindowGroupCountPilot;
    ClipWindowGroupObservedPilot& group = g_clipWindowGroupsPilot[slot];
    group.entries = NULL;
    group.capacity = group.count = 0;
    group.key = window->groupKey;
    PrependClipWindowInGroupPilot(&group, window);
}

// 2011dd5d. Unregister the first matching pointer, even if its key changed.
extern "C" void __stdcall UnregisterClipWindowPilot(ClipWindowObservedPilot* window)
{
    for (DWORD slot = 0; slot < g_clipWindowGroupCountPilot; ++slot) {
        ClipWindowGroupObservedPilot& group = g_clipWindowGroupsPilot[slot];
        for (DWORD index = 0; index < group.count; ++index) {
            if (group.entries[index] != window) continue;
            for (DWORD i = index + 1; i < group.count; ++i) group.entries[i - 1] = group.entries[i];
            if (--group.count != 0) return;
            if (group.entries != NULL) {
                HeapFree(g_privateHeapPilot, 0, group.entries);
                group.entries = NULL;
            }
            for (DWORD i = slot + 1; i < g_clipWindowGroupCountPilot; ++i)
                g_clipWindowGroupsPilot[i - 1] = g_clipWindowGroupsPilot[i];
            if (--g_clipWindowGroupCountPilot != 0) return;
            if (g_clipWindowGroupsPilot != NULL) {
                HeapFree(g_privateHeapPilot, 0, g_clipWindowGroupsPilot);
                g_clipWindowGroupsPilot = NULL;
            }
            g_clipWindowGroupCapacityPilot = 0;
            if (g_sharedClipRegionFirstPilot != NULL) {
                DeleteObject(g_sharedClipRegionFirstPilot);
                g_sharedClipRegionFirstPilot = NULL;
            }
            if (g_sharedClipRegionSecondPilot != NULL) {
                DeleteObject(g_sharedClipRegionSecondPilot);
                g_sharedClipRegionSecondPilot = NULL;
            }
            return;
        }
    }
}

typedef DWORD (__thiscall *GraphicsClipIndexPilot)(void*);
typedef void* (__thiscall *GraphicsClipLookupPilot)(void*, DWORD);
typedef void (__thiscall *GraphicsClipWordPilot)(void*, DWORD);
typedef bool (__thiscall *GraphicsClipFlagPilot)(void*);
typedef HRGN (__thiscall *GraphicsClipRegionPilot)(void*);
typedef void (__thiscall *GraphicsClipRectPilot)(void*, RECT*);
typedef HWND (__thiscall *GraphicsClipWindowPilot)(void*);
typedef void (__thiscall *GraphicsClipNotifyPilot)(void*);

// 2011ee18 / 262. Lookup is retried on future calls after a null result.
extern "C" DWORD __stdcall ReadClipWindowGraphicsIndexPilot(ClipWindowObservedPilot* window)
{
    if (window == NULL) return 0xffffffff;
    if (window->cachedGraphicsWindow == NULL) {
        ClipWindowLinkObservedPilot* link = window->link;
        if (link == NULL || GetGraphicsControllerSourcePilot() == NULL) return 0xffffffff;
        void* graphics = GetGraphicsControllerSourcePilot();
        void* entry = GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0x20)(
            graphics, reinterpret_cast<DWORD>(link->window));
        if (entry == NULL) return 0xffffffff;
        GraphicsClipSlotPilot<GraphicsClipWordPilot>(entry, 0x50)(entry, reinterpret_cast<DWORD>(link->window));
        window->cachedGraphicsWindow = entry;
    }
    void* entry = window->cachedGraphicsWindow;
    return GraphicsClipSlotPilot<GraphicsClipIndexPilot>(entry, 0x30)(entry);
}

// 2011d836. Graphics slots are typed by the API call sites.
extern "C" void __cdecl RefreshControllerClipRegionsSourcePilot()
{
    HRGN scratch = CreateRectRgn(0, 0, 0, 0);
    void* graphics = GetGraphicsControllerSourcePilot();
    DWORD count = GraphicsClipSlotPilot<GraphicsClipIndexPilot>(graphics, 0xc)(graphics);
    for (DWORD index = 0; index < count; ++index) {
        graphics = GetGraphicsControllerSourcePilot();
        void* entry = GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0)(graphics, index);
        if (!GraphicsClipSlotPilot<GraphicsClipFlagPilot>(entry, 0x40)(entry)) continue;

        HRGN output = GraphicsClipSlotPilot<GraphicsClipRegionPilot>(entry, 0x5c)(entry);
        SetRectRgn(output, 0, 0, 0, 0);
        RECT base;
        GraphicsClipSlotPilot<GraphicsClipRectPilot>(entry, 0xc)(entry, &base);
        HRGN remaining = CreateRectRgnIndirect(&base);
        for (LONG groupIndex = static_cast<LONG>(g_clipWindowGroupCountPilot - 1); groupIndex >= 0; --groupIndex) {
            ClipWindowGroupObservedPilot* group = &g_clipWindowGroupsPilot[groupIndex];
            for (LONG itemIndex = static_cast<LONG>(group->count - 1); itemIndex >= 0; --itemIndex) {
                ClipWindowObservedPilot* window = group->entries[itemIndex];
                ClipWindowLinkObservedPilot* link = window->link;
                if (ReadClipWindowGraphicsIndexPilot(window) != index || link == NULL ||
                    link->geometryLink == NULL) continue;

                DWORD flags = window->flags;
                if ((flags & 0x80000000) == 0 || (flags & 0x100) != 0) continue;
                const ClipGeometryObservedPilot* geometry = link->geometryLink->geometry;
                LONG left = 0, top = 0;
                if ((geometry->flags & 0x60000000) == 0) {
                    left = geometry->left;
                    top = geometry->top;
                }
                LONG right = static_cast<LONG>(static_cast<DWORD>(left) + geometry->width);
                LONG bottom = static_cast<LONG>(static_cast<DWORD>(top) + geometry->height);
                if ((flags & 0x800) != 0) left = top = right = bottom = 0;
                SetRectRgn(scratch, left, top, right, bottom);
                if ((window->flags & 0x40004000) == 0)
                    CombineRgn(remaining, remaining, scratch, RGN_DIFF);
                if (window->regionUnionFlag != 0) CombineRgn(output, output, scratch, RGN_OR);
            }
        }
        RECT bounds;
        GetRgnBox(remaining, &bounds);
        GraphicsClipSlotPilot<GraphicsClipRectPilot>(entry, 0x64)(entry, &bounds);
        GraphicsClipSlotPilot<GraphicsClipWordPilot>(entry, 0x68)(entry, 1);
        HRGN boundsRegion = CreateRectRgnIndirect(&bounds);
        CombineRgn(output, output, boundsRegion, RGN_OR);
        DeleteObject(boundsRegion);
        DeleteObject(remaining);
    }
    DeleteObject(scratch);
}

// 2011d580. Each group is visited twice, first with flag 0x100 clear.
extern "C" void __cdecl ReorderClipWindowsPilot()
{
    if (g_messageQuitRequestedPilot != 0 || GetGraphicsControllerSourcePilot() == NULL ||
        g_clipWindowGroupCountPilot == 0 || g_reorderingClipWindowsPilot != 0) return;
    g_reorderingClipWindowsPilot = 1;
    void* graphics = GetGraphicsControllerSourcePilot();
    DWORD count = GraphicsClipSlotPilot<GraphicsClipIndexPilot>(graphics, 0xc)(graphics);
    for (DWORD index = 0; index < count; ++index) {
        graphics = GetGraphicsControllerSourcePilot();
        void* entry = GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0)(graphics, index);
        if (!GraphicsClipSlotPilot<GraphicsClipFlagPilot>(entry, 0x40)(entry)) continue;
        HWND parent = GraphicsClipSlotPilot<GraphicsClipWindowPilot>(entry, 0x10)(entry);
        HDWP batch = BeginDeferWindowPos(1);
        HWND after = g_idleControllerPilot->QueryField38() ? parent : NULL;
        if (g_sharedClipRegionFirstPilot == NULL)
            g_sharedClipRegionFirstPilot = CreateRectRgn(0, 0, 0, 0);
        else
            SetRectRgn(g_sharedClipRegionFirstPilot, 0, 0, 0, 0);

        for (DWORD groupIndex = 0; groupIndex < g_clipWindowGroupCountPilot; ++groupIndex) {
            ClipWindowGroupObservedPilot* group = &g_clipWindowGroupsPilot[groupIndex];
            for (DWORD pass = 0; pass < 2; ++pass) {
                DWORD requiredFlag = pass * 0x100;
                for (DWORD itemIndex = 0; itemIndex < group->count; ++itemIndex) {
                    ClipWindowObservedPilot* window = group->entries[itemIndex];
                    DWORD flags = window->flags;
                    if ((flags & 0x100) != requiredFlag || (flags & 0x80000000) == 0) continue;
                    ClipWindowLinkObservedPilot* link = window->link;
                    if (GetParent(link->window) != parent) continue;
                    HWND child = link->window;
                    batch = DeferWindowPos(batch, child, after, 0, 0, 0, 0,
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                    after = child;
                    if (window->regionUnionFlag != 0 || (window->flags & 0x4100) != 0) continue;

                    CombineRgn(link->overlapRegion, g_sharedClipRegionFirstPilot, NULL, RGN_COPY);
                    RECT rect;
                    GetWindowRect(child, &rect);
                    MapWindowPoints(NULL, parent, reinterpret_cast<POINT*>(&rect), 2);
                    if ((window->flags & 0x800) != 0)
                        SetRectRgn(g_sharedClipRegionFirstPilot, 0, 0, 0, 0);
                    else
                        SetRectRgn(g_sharedClipRegionFirstPilot, rect.left, rect.top, rect.right, rect.bottom);
                    CombineRgn(g_sharedClipRegionFirstPilot, g_sharedClipRegionFirstPilot, link->overlapRegion, RGN_OR);
                }
            }
        }
        EndDeferWindowPos(batch);
    }
    g_reorderingClipWindowsPilot = 0;
}

// 2011d315 / 28. Both region paths share the scratch HRGN at 2020de40.
extern "C" void __stdcall AddClipWindowUpdateRectanglePilot(
    ClipWindowObservedPilot* window, const RECT* rect)
{
    if (window == NULL || (window->flags & 0x80000000) == 0 || window->link == NULL) return;
    ClipWindowLinkObservedPilot* link = window->link;
    if (g_sharedClipRegionFirstPilot == NULL)
        g_sharedClipRegionFirstPilot = CreateRectRgn(0, 0, 0, 0);
    SetRectRgn(g_sharedClipRegionFirstPilot, rect->left, rect->top, rect->right, rect->bottom);
    CombineRgn(link->updateRegion, link->updateRegion, g_sharedClipRegionFirstPilot, RGN_OR);
}

// 2011d37f / 29. An existing entry at index zero still requests reordering.
extern "C" int __stdcall RaiseClipWindowInGroupPilot(ClipWindowObservedPilot* window)
{
    if (window == NULL || (window->flags & 0x1000) != 0) return -1;
    for (DWORD groupIndex = 0; groupIndex < g_clipWindowGroupCountPilot; ++groupIndex) {
        ClipWindowGroupObservedPilot& group = g_clipWindowGroupsPilot[groupIndex];
        if (group.key != window->groupKey) continue;
        for (DWORD index = 0; index < group.count; ++index) {
            if (group.entries[index] != window) continue;
            for (; index > 0; --index) group.entries[index] = group.entries[index - 1];
            group.entries[0] = window;
            ReorderClipWindowsPilot();
            return 0;
        }
    }
    return -1;
}

// 2011cec9 / 20. Releases the native window resources and link allocation.
extern "C" void __stdcall DestroyClipWindowLinksPilot(ClipWindowObservedPilot* window)
{
    if (window == NULL) return;
    if (g_activeClipWindowPilot == window) g_activeClipWindowPilot = NULL;
    ClipWindowLinkObservedPilot* link = window->link;
    UnregisterClipWindowPilot(window);
    if (link != NULL) {
        DestroyWindow(link->window);
        DestroyWindow(link->auxiliaryWindow);
        DeleteObject(link->updateRegion);
        DeleteObject(link->overlapRegion);
        if (link->allocation44 != NULL) {
            HeapFree(g_privateHeapPilot, 0, link->allocation44);
            link->allocation44 = NULL;
        }
        link->field2c = 0;
        link->field30 = 0;
        if (window->link != NULL) {
            HeapFree(g_privateHeapPilot, 0, window->link);
            window->link = NULL;
        }
    }
    if (g_messageQuitRequestedPilot == 0 && GetGraphicsControllerSourcePilot() != NULL) {
        ReorderClipWindowsPilot();
        void* graphics = GetGraphicsControllerSourcePilot();
        GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(graphics, 0x38)(graphics);
    }
}

static void OffsetPanelUpdateRegionPilot(HRGN region, const ClipWindowObservedPilot* window)
{
    OffsetRgn(region, static_cast<LONG>(0u - static_cast<DWORD>(window->left)),
        static_cast<LONG>(0u - static_cast<DWORD>(window->top)));
}

// 2011e6cb. The original window receiver is in ESI.
extern "C" void __stdcall RebuildPanelUpdateRegionPilot(ClipWindowObservedPilot* window)
{
    RECT graphicsBounds = {0, 0, 0, 0};
    if (window == NULL || window->link == NULL) return;
    ClipWindowLinkObservedPilot* link = window->link;
    DWORD index = ReadClipWindowGraphicsIndexPilot(window);
    if (index == 0xffffffff) return;
    void* graphics = GetGraphicsControllerSourcePilot();
    void* entry = GraphicsClipSlotPilot<GraphicsClipLookupPilot>(graphics, 0)(graphics, index);
    if ((window->flags & 0x20000000) != 0)
        GraphicsClipSlotPilot<GraphicsClipRectPilot>(entry, 0xc)(entry, &graphicsBounds);
    if (g_sharedClipRegionFirstPilot == NULL)
        g_sharedClipRegionFirstPilot = CreateRectRgn(0, 0, 0, 0);

    if (GraphicsClipSlotPilot<GraphicsClipFlagPilot>(entry, 0x14)(entry)) {
        LONG width = window->width, height = window->height;
        if ((window->flags & 0x20000000) != 0) {
            width = static_cast<LONG>(static_cast<DWORD>(graphicsBounds.right) - graphicsBounds.left);
            height = static_cast<LONG>(static_cast<DWORD>(graphicsBounds.bottom) - graphicsBounds.top);
        }
        SetRectRgn(link->updateRegion, 0, 0, width, height);
    } else {
        if (g_sharedClipRegionSecondPilot == NULL) {
            g_sharedClipRegionSecondPilot = CreateRectRgn(0, 0, 0, 0);
            if (g_sharedClipRegionSecondPilot == NULL) return;
        }
        HRGN graphicsRegion = GraphicsClipSlotPilot<GraphicsClipRegionPilot>(entry, 0x5c)(entry);
        if (graphicsRegion != NULL) {
            CombineRgn(g_sharedClipRegionFirstPilot, graphicsRegion, NULL, RGN_COPY);
            CombineRgn(g_sharedClipRegionSecondPilot, graphicsRegion, NULL, RGN_COPY);
        }
        if ((window->flags & 0x20000000) == 0) {
            OffsetPanelUpdateRegionPilot(g_sharedClipRegionFirstPilot, window);
            OffsetPanelUpdateRegionPilot(g_sharedClipRegionSecondPilot, window);
        }
        CombineRgn(link->updateRegion, link->updateRegion, g_sharedClipRegionFirstPilot, RGN_OR);
        if ((window->flags & 0x20000000) != 0) {
            LONG width = static_cast<LONG>(static_cast<DWORD>(graphicsBounds.right) - graphicsBounds.left);
            LONG height = static_cast<LONG>(static_cast<DWORD>(graphicsBounds.bottom) - graphicsBounds.top);
            SetRectRgn(g_sharedClipRegionFirstPilot, 0, 0, width, height);
            CombineRgn(link->updateRegion, link->updateRegion, g_sharedClipRegionFirstPilot, RGN_AND);
        } else {
            SetRectRgn(g_sharedClipRegionFirstPilot, 0, 0, window->width, window->height);
            CombineRgn(link->updateRegion, link->updateRegion, g_sharedClipRegionFirstPilot, RGN_AND);
            CombineRgn(link->updateRegion, g_sharedClipRegionSecondPilot, link->updateRegion, RGN_AND);
        }
    }
    CombineRgn(g_sharedClipRegionFirstPilot, link->overlapRegion, NULL, RGN_COPY);
    if ((window->flags & 0x20000000) == 0)
        OffsetPanelUpdateRegionPilot(g_sharedClipRegionFirstPilot, window);
    CombineRgn(link->updateRegion, link->updateRegion, g_sharedClipRegionFirstPilot, RGN_DIFF);
}

struct PanelUpdateContextObservedPilot {
    DWORD unknown00;
    HWND windowHandle;
    ClipWindowObservedPilot* window;
    ClipWindowLinkObservedPilot* link;
    BYTE unknown10[0x31];
    BYTE bitmapDrawPending;
    BYTE skipNextPaint;
    BYTE unknown43;
    DWORD contextField44;
};
C_ASSERT(sizeof(PanelUpdateContextObservedPilot) == 0x48);
C_ASSERT(offsetof(PanelUpdateContextObservedPilot, window) == 8);
C_ASSERT(offsetof(PanelUpdateContextObservedPilot, bitmapDrawPending) == 0x41);
C_ASSERT(offsetof(PanelUpdateContextObservedPilot, skipNextPaint) == 0x42);
struct PanelPixelsObservedPilot {
    short format;
    BYTE unknown02[2];
    LONG width, height, pitch;
    DWORD unknown10;
    DWORD storageBytes;
    BYTE* pixels;
};
C_ASSERT(sizeof(PanelPixelsObservedPilot) == 0x1c);
C_ASSERT(offsetof(PanelPixelsObservedPilot, pixels) == 0x18);
extern "C" char g_configurationFilePilot[];
extern "C" int __stdcall BeginGraphicsPanelUpdateSourcePilot(ClipWindowObservedPilot*, DWORD, void*); // g2d ordinal 68
extern "C" void __stdcall EndGraphicsPanelUpdateSourcePilot(ClipWindowObservedPilot*, DWORD); // g2d ordinal 69
extern "C" void __stdcall TransferPanelUpdateSourcePilot(
    ClipWindowObservedPilot*, PanelPixelsObservedPilot*, const PanelPixelsObservedPilot*,
    const RECT*, const RECT*); // 2011f7d3
static DWORD g_usePartialPanelUpdatePilot = 0xfffffffe; // 201f9958

// 2012d7ac. Original context in ECX, optional bounds in EAX and state on stack.
extern "C" void __stdcall PreparePanelUpdateTransferPilot(
    PanelUpdateContextObservedPilot* context, const RECT* bounds, void* state)
{
    ClipWindowObservedPilot* window = context->window;
    RECT destination, source;
    if (bounds == NULL) {
        destination.left = destination.top = source.left = source.top = 0;
        destination.right = window->width;
        destination.bottom = window->height;
        source.right = window->surfaceWidth;
        source.bottom = window->surfaceHeight;
    } else {
        destination.left = bounds->left > 0 ? bounds->left : 0;
        destination.top = bounds->top > 0 ? bounds->top : 0;
        destination.right = bounds->right < window->width ? bounds->right : window->width;
        destination.bottom = bounds->bottom < window->height ? bounds->bottom : window->height;
        source = destination;
    }
    if ((window->flags & 0x40000000) == 0) OffsetRect(&destination, window->left, window->top);
    window = context->window;
    TransferPanelUpdateSourcePilot(window, static_cast<PanelPixelsObservedPilot*>(state),
        reinterpret_cast<const PanelPixelsObservedPilot*>(reinterpret_cast<BYTE*>(window) + 0x78),
        &destination, &source);
}

// 2012d584. The original catches stack overflow only around the stack probe.
extern "C" void __fastcall RunPanelUpdateSourcePilot(PanelUpdateContextObservedPilot* context)
{
    ClipWindowObservedPilot* window = context->window;
    void* entry = window->cachedGraphicsWindow;
    if (entry == NULL) return;
    if (window->width != window->surfaceWidth || window->height != window->surfaceHeight) {
        typedef void (__thiscall *FillPilot)(void*, LONG, LONG, LONG, LONG, DWORD);
        LONG left = window->left, top = window->top, width = window->width, height = window->height;
        LONG right = static_cast<LONG>(static_cast<DWORD>(left) + width - 1);
        LONG bottom = static_cast<LONG>(static_cast<DWORD>(top) + height - 1);
        GraphicsClipSlotPilot<FillPilot>(entry, 0x70)(entry, left, top, width, height, 0xff000000);
        GraphicsClipSlotPilot<FillPilot>(entry, 0x70)(entry, left, top, width, 1, 0xffffffff);
        GraphicsClipSlotPilot<FillPilot>(entry, 0x70)(entry, left, top, 1, height, 0xffffffff);
        GraphicsClipSlotPilot<FillPilot>(entry, 0x70)(entry, left, bottom, width, 1, 0xffffffff);
        GraphicsClipSlotPilot<FillPilot>(entry, 0x70)(entry, right, top, 1, height, 0xffffffff);
        return;
    }

    DWORD updateState[9]; // Opaque storage written through ordinal 68.
    if (BeginGraphicsPanelUpdateSourcePilot(window, 0, updateState) != 0) return;
    window = context->window;
    ClipWindowLinkObservedPilot* link = window->link;
    bool partial = (window->flags & 0x40000000) == 0 && link != NULL;
    if (partial && g_usePartialPanelUpdatePilot == 0xfffffffe) {
        g_usePartialPanelUpdatePilot = GetPrivateProfileIntA("DISPLAY", "UsePartialPanelUpdate",
            -1, g_configurationFilePilot);
    }
    if (partial && g_usePartialPanelUpdatePilot != 0) {
        RebuildPanelUpdateRegionPilot(context->window);
        DWORD bytes = GetRegionData(link->updateRegion, 0, NULL);
        if (bytes != 0) {
            RGNDATA* data;
            __try {
                data = static_cast<RGNDATA*>(_alloca(bytes));
            } __except (GetExceptionCode() == EXCEPTION_STACK_OVERFLOW ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
                _resetstkoflw();
                return;
            }
            if (GetRegionData(link->updateRegion, bytes, data) != 0 &&
                data->rdh.dwSize == 0x20 && data->rdh.iType == 1) {
                RECT bounds = {0, 0, 0, 0};
                GetRgnBox(link->updateRegion, &bounds);
                PreparePanelUpdateTransferPilot(context, &bounds, updateState);
            }
        }
    } else {
        PreparePanelUpdateTransferPilot(context, NULL, updateState);
    }
    EndGraphicsPanelUpdateSourcePilot(context->window, 0);
}
