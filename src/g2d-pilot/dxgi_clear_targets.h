#pragma once

// Source boundaries for the original api.dll import calls.
extern "C" void* WINAPI GetG2dTargetWindowContextBoundaryPilot(HWND); // ordinal 258
extern "C" HWND WINAPI GetG2dTargetRegistryKeyBoundaryPilot(void*); // ordinal 260
extern "C" HWND WINAPI GetG2dTargetPresentationWindowBoundaryPilot(void*); // ordinal 257
#include "dxgi_clear_target_depth.h"
void ResetG2dTargetCompositorBoundaryPilot(void*); // 2067a320
int BindG2dTargetCompositorBoundaryPilot(void*, HWND); // 20679e60
int FinishG2dTargetCompositorBoundaryPilot(void*); // 2067a020
void DestroyG2dRegisteredClearTargetBoundaryPilot(void*); // 206db090
void DestroyG2dTargetTreeNodesBoundaryPilot(void*, void*); // 206a85a0

static void SetG2dClearTargetViewsPilot(void* device, UINT count, void* const* color, void* depth)
{
    typedef void (WINAPI *SetTargetsMethod)(void*, UINT, void* const*, void*);
    G2dSlotPilot<SetTargetsMethod>(device, 0x60)(device, count, color, depth);
}

// 206db310. The original receives the target in ESI.
int FinishG2dRegisteredClearTargetSourcePilot(void* target)
{
    BYTE& active = G2dWindowFieldPilot<BYTE>(target, 0);
    if (!active) return -1;

    void* window = G2dWindowFieldPilot<void*>(target, 0x0c);
    void*& depth = G2dWindowFieldPilot<void*>(target, 0x18);
    BYTE& compositorRequested = G2dWindowFieldPilot<BYTE>(target, 1);
    if (compositorRequested && G2dWindowFieldPilot<BYTE>(target, 0x24)) {
        void* device = G2dWindowFieldPilot<void*>(window, 0x104);
        SetG2dClearTargetViewsPilot(device, 1, &G2dWindowFieldPilot<void*>(target, 0x14),
                                   GetG2dTaggedClearViewPilot(depth, 2));
        const int status = FinishG2dTargetCompositorBoundaryPilot(static_cast<BYTE*>(target) + 0x1c);
        if (status) return status;
    }

    ReturnG2dTargetDepthResourcePilot(G2dWindowFieldPilot<void*>(window, 0x164),
                                    G2dWindowFieldPilot<DWORD>(depth, 8));
    depth = NULL;
    active = 0;
    compositorRequested = 0;
    return 0;
}

static void UnbindG2dClearTargetsPilot(void* registry)
{
    void* window = G2dWindowFieldPilot<void*>(registry, 0);
    void* device = G2dWindowFieldPilot<void*>(window, 0x104);
    SetG2dClearTargetViewsPilot(device, 0, NULL, NULL);
}

// 206db8f0 and its checked-iterator lookup at 2067a9a0.
static __declspec(noinline) bool HasG2dTargetWindowPilot(void* registry, HWND presentation)
{
    void* sentinel = G2dWindowFieldPilot<void*>(registry, 0x48);
    void* node = G2dWindowFieldPilot<void*>(sentinel, 4);
    const ULONG_PTR key = reinterpret_cast<ULONG_PTR>(presentation);
    while (node != sentinel) {
        const ULONG_PTR nodeKey = G2dWindowFieldPilot<ULONG_PTR>(node, 0x0c);
        if (key == nodeKey) return true;
        node = G2dWindowFieldPilot<void*>(node, key < nodeKey ? 0 : 8);
    }
    return false;
}

// 206db1e0. The stack arguments are the presentation window and options.
static __declspec(noinline) int BindG2dRegisteredClearTargetPilot(void* target, HWND presentation, DWORD options)
{
    void* window = G2dWindowFieldPilot<void*>(target, 0x0c);
    void* device = G2dWindowFieldPilot<void*>(window, 0x104);
    BYTE& active = G2dWindowFieldPilot<BYTE>(target, 0);
    BYTE& compositorRequested = G2dWindowFieldPilot<BYTE>(target, 1);
    const BYTE& compositorReady = G2dWindowFieldPilot<BYTE>(target, 0x24);
    void* compositor = static_cast<BYTE*>(target) + 0x1c;
    if (active) {
        active = 0;
        if (compositorRequested & compositorReady) {
            compositorRequested = 0;
            ResetG2dTargetCompositorBoundaryPilot(compositor);
        } else {
            SetG2dClearTargetViewsPilot(device, 0, NULL, NULL);
        }
        return -1;
    }

    compositorRequested = options == 1;
    DWORD textureDesc[11];
    const int readStatus = ReadG2dTargetBackBufferDescPilot(target, textureDesc);
    if (readStatus) return readStatus;
    const ULONGLONG key = (static_cast<ULONGLONG>(0x12d) << 32)
        | ((textureDesc[1] & 0xffffu) << 16) | (textureDesc[0] & 0xffffu);
    void*& depth = G2dWindowFieldPilot<void*>(target, 0x18);
    const int acquireStatus = AcquireG2dTargetDepthResourcePilot(
        G2dWindowFieldPilot<void*>(window, 0x164), &key, &depth);
    if (acquireStatus) return acquireStatus;

    const DWORD depthDesc[] = {0x2d, 3, 0, 0, 0};
    const int viewStatus = ConfigureG2dTargetDepthViewPilot(depth, depthDesc);
    if (viewStatus) return viewStatus;
    if (compositorRequested & compositorReady) {
        const int bindStatus = BindG2dTargetCompositorBoundaryPilot(compositor, presentation);
        if (bindStatus) return bindStatus;
    } else {
        SetG2dClearTargetViewsPilot(device, 1, &G2dWindowFieldPilot<void*>(target, 0x14),
                                   GetG2dTaggedClearViewPilot(depth, 2));
    }
    active = 1;
    return 0;
}

// 206d9950. The tree key comparison is unsigned in the original.
static __declspec(noinline) void* FindG2dRegisteredClearTargetPilot(void* registry, HWND key)
{
    return FindG2dTargetTreeValuePilot(static_cast<BYTE*>(registry) + 0x40,
                                     reinterpret_cast<ULONG_PTR>(key));
}

// 206db9b0. Registry +0x4c aliases the host window's +0x15c field.
int SelectG2dRegisteredClearTargetSourcePilot(void* registry, HWND presentation, HWND key, DWORD options)
{
    void*& active = G2dWindowFieldPilot<void*>(registry, 0x4c);
    if (!presentation) {
        active = NULL;
        UnbindG2dClearTargetsPilot(registry);
        return 0;
    }
    if (!key) {
        void* context = GetG2dTargetWindowContextBoundaryPilot(presentation);
        if (!context) return 0;
        key = GetG2dTargetRegistryKeyBoundaryPilot(context);
        presentation = GetG2dTargetPresentationWindowBoundaryPilot(context);
        if (!key) return 0;
    }

    void* target = FindG2dRegisteredClearTargetPilot(registry, key);
    if (!target) return -1;
    active = target;
    if (G2dWindowFieldPilot<HWND>(target, 8) != presentation || presentation != key) {
        if (!HasG2dTargetWindowPilot(registry, presentation)) return -1;
    }
    return BindG2dRegisteredClearTargetPilot(active, presentation, options);
}

static void DestroyG2dClearTargetValuesPilot(void* node, void* sentinel)
{
    if (node == sentinel) return;
    DestroyG2dClearTargetValuesPilot(G2dWindowFieldPilot<void*>(node, 0), sentinel);
    void*& value = G2dWindowFieldPilot<void*>(node, 0x10);
    if (value) {
        G2dWindowFieldPilot<DWORD>(value, 4) = 0;
        void* target = value;
        DestroyG2dRegisteredClearTargetBoundaryPilot(target);
        FreeG2dHeapSlotBoundaryPilot(&target);
        value = NULL;
    }
    DestroyG2dClearTargetValuesPilot(G2dWindowFieldPilot<void*>(node, 8), sentinel);
}

// 206dbd20. Unbind selected views before destroying registry values.
void ResetG2dClearTargetRegistrySourcePilot(void* registry)
{
    void*& active = G2dWindowFieldPilot<void*>(registry, 0x4c);
    if (active) {
        UnbindG2dClearTargetsPilot(registry);
        active = NULL;
    }

    void* tree = static_cast<BYTE*>(registry) + 0x40;
    void* sentinel = G2dWindowFieldPilot<void*>(tree, 4);
    void* root = G2dWindowFieldPilot<void*>(sentinel, 4);
    DestroyG2dClearTargetValuesPilot(root, sentinel);
    DestroyG2dTargetTreeNodesBoundaryPilot(tree, root);
    G2dWindowFieldPilot<void*>(sentinel, 4) = sentinel;
    G2dWindowFieldPilot<DWORD>(tree, 8) = 0;
    G2dWindowFieldPilot<void*>(sentinel, 0) = sentinel;
    G2dWindowFieldPilot<void*>(sentinel, 8) = sentinel;
}
