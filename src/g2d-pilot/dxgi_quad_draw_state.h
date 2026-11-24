#pragma once

// 2067f1e0. The original allocation is 0x298 bytes.
static __declspec(noinline) void ConstructG2dQuadDrawStatePilot(void* entry)
{
    G2dWindowFieldPilot<DWORD>(entry, 0) = 0;
    G2dWindowFieldPilot<BYTE>(entry, 4) = 0;
    G2dWindowFieldPilot<BYTE>(entry, 5) = 0;
    G2dWindowFieldPilot<void*>(entry, 8) = NULL;
    G2dWindowFieldPilot<DWORD>(entry, 0x0c) = 0;
    G2dWindowFieldPilot<DWORD>(entry, 0x270) = 0;
    for (unsigned int i = 0; i < 4; ++i) {
        G2dWindowFieldPilot<FLOAT>(entry, 0x274 + 4 * i) = 1.0f;
        G2dWindowFieldPilot<void*>(entry, 0x288 + 4 * i) = NULL;
    }
    G2dWindowFieldPilot<DWORD>(entry, 0x284) = 0xffffffffu;
}

// 2067f560, scoped to the quad path's rasterizer, depth/stencil and blend flags.
static __declspec(noinline) void DefaultG2dQuadDrawDescriptorsPilot(void* entry)
{
    const DWORD rasterizer[] = {3, 3, 1, 0, 0, 0, 0, 1, 0, 0};
    const DWORD depthStencil[] = {1, 1, 2, 0, 0xffff, 1, 1, 1, 8, 1, 1, 1, 8};
    const DWORD blend[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 1, 1, 0x0f0f0f0f, 0x0f0f0f0f};
    BYTE* data = static_cast<BYTE*>(entry);
    memcpy(data + 0x10, rasterizer, sizeof(rasterizer));
    memcpy(data + 0x38, depthStencil, sizeof(depthStencil));
    memcpy(data + 0x6c, blend, sizeof(blend));
}

// 2067f2d0, scoped to the fixed quad mask 7.
static __declspec(noinline) void InitializeG2dQuadDrawStatePilot(void* entry, void* device)
{
    G2dWindowFieldPilot<void*>(entry, 8) = device;
    G2dWindowFieldPilot<DWORD>(entry, 0) = 7;
    G2dWindowFieldPilot<BYTE>(entry, 5) = 1;
    DefaultG2dQuadDrawDescriptorsPilot(entry);
}

// 2067f360 records partial creation so cleanup can release successful slots.
static __declspec(noinline) int CreateG2dQuadDrawStatesPilot(void* entry)
{
    typedef HRESULT (WINAPI *CreateStateMethod)(void*, const void*, void**);
    BYTE& attempted = G2dWindowFieldPilot<BYTE>(entry, 4);
    if (attempted) return -1;
    attempted = 1;
    const unsigned int slots[] = {0x154, 0x150, 0x14c};
    const unsigned int descriptors[] = {0x10, 0x38, 0x6c};
    void* device = G2dWindowFieldPilot<void*>(entry, 8);
    for (unsigned int i = 0; i < _countof(slots); ++i) {
        if (!(G2dWindowFieldPilot<BYTE>(entry, 0) & (1u << i))) continue;
        void** output = &G2dWindowFieldPilot<void*>(entry, 0x288 + 4 * i);
        if (FAILED(G2dSlotPilot<CreateStateMethod>(device, slots[i])(
                device, static_cast<BYTE*>(entry) + descriptors[i], output))) return -1;
    }
    return 0;
}

// 2067f480. Apply enabled states in the original order.
static __declspec(noinline) void ApplyG2dQuadDrawStatePilot(void* entry)
{
    typedef void (WINAPI *SetPointerMethod)(void*, void*);
    typedef void (WINAPI *SetDepthStencilMethod)(void*, void*, UINT);
    typedef void (WINAPI *SetBlendMethod)(void*, void*, const FLOAT*, UINT);
    if (!G2dWindowFieldPilot<BYTE>(entry, 4)) return;
    const BYTE flags = G2dWindowFieldPilot<BYTE>(entry, 0);
    void* device = G2dWindowFieldPilot<void*>(entry, 8);
    if (flags & 1) G2dSlotPilot<SetPointerMethod>(device, 0x74)(device, G2dWindowFieldPilot<void*>(entry, 0x288));
    if (flags & 2) G2dSlotPilot<SetDepthStencilMethod>(device, 0x68)(
        device, G2dWindowFieldPilot<void*>(entry, 0x28c), G2dWindowFieldPilot<UINT>(entry, 0x270));
    if (flags & 4) G2dSlotPilot<SetBlendMethod>(device, 0x64)(
        device, G2dWindowFieldPilot<void*>(entry, 0x290),
        &G2dWindowFieldPilot<FLOAT>(entry, 0x274), G2dWindowFieldPilot<UINT>(entry, 0x284));
    if (flags & 8) G2dSlotPilot<SetPointerMethod>(device, 0x2c)(device, G2dWindowFieldPilot<void*>(entry, 0x294));
}

// 2067eb50. Each index represents one combination of color, depth and stencil.
static int PrepareG2dQuadDrawStatePilot(void* holder, UINT index, DWORD stencilReference)
{
    if (index >= 8) return -1;
    void*& cached = G2dWindowFieldPilot<void*>(holder, 0x84 + index * 4);
    if (!cached) {
        void* entry = AllocateG2dWindowStoragePilot(0x298);
        if (!entry) return -1;
        ConstructG2dQuadDrawStatePilot(entry);
        void* window = G2dWindowFieldPilot<void*>(holder, 8);
        InitializeG2dQuadDrawStatePilot(entry, G2dWindowFieldPilot<void*>(window, 0x104));

        G2dWindowFieldPilot<BYTE>(entry, 0xa8) = (index & 1) ? 0x0f : 0;
        const bool depth = (index & 2) != 0;
        G2dWindowFieldPilot<DWORD>(entry, 0x38) = depth;
        G2dWindowFieldPilot<DWORD>(entry, 0x3c) = depth;
        G2dWindowFieldPilot<DWORD>(entry, 0x40) = depth ? 8 : 1;
        const bool stencil = (index & 4) != 0;
        G2dWindowFieldPilot<DWORD>(entry, 0x44) = stencil;
        G2dWindowFieldPilot<BYTE>(entry, 0x49) = stencil ? 0xff : 0;
        const DWORD frontOperation = stencil ? 3 : 1;
        for (UINT i = 0; i < 3; ++i)
            G2dWindowFieldPilot<DWORD>(entry, 0x4c + i * 4) = frontOperation;
        G2dWindowFieldPilot<DWORD>(entry, 0x58) = stencil ? 8 : 1;

        const int status = CreateG2dQuadDrawStatesPilot(entry);
        if (status) {
            ResetG2dWindowStateEntryPilot(entry);
            DestroyG2dWindowStateEntryPilot(entry);
            FreeG2dHeapSlotBoundaryPilot(&entry);
            return status;
        }
        cached = entry;
    }
    G2dWindowFieldPilot<DWORD>(cached, 0x270) = stencilReference;
    ApplyG2dQuadDrawStatePilot(cached);
    return 0;
}
