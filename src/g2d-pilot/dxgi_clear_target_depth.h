#pragma once

extern "C" DWORD __cdecl ReadG2dDepthPoolStampBoundaryPilot(); // main.dll ordinal 61, IAT 2066158c
int CreateG2dDepthTextureBoundaryPilot(const ULONGLONG*, void*, DWORD, DWORD, void**); // 206d87a0
// Insertion boundaries retain a reference for each committed node.
void* InsertG2dDepthByDescriptionBoundaryPilot(void*, const ULONGLONG*, void*); // 206d9ad0
bool InsertG2dDepthByIdBoundaryPilot(void*, DWORD, void*); // 206dbe30
void EraseG2dDepthDescriptionNodeBoundaryPilot(void*, void*); // 206d95f0

// 206d9270. The factory returns one locally owned reference.
static __declspec(noinline) int CreateG2dTargetDepthResourcePilot(
    void* resources, const ULONGLONG* key, void** result)
{
    *result = NULL;
    void* resource = NULL;
    void* descriptionTree = static_cast<BYTE*>(resources) + 0x0c;
    void* descriptionNode = NULL;
    int status;
    try {
        void* window = G2dWindowFieldPilot<void*>(resources, 8);
        status = CreateG2dDepthTextureBoundaryPilot(
            key, G2dWindowFieldPilot<void*>(window, 0x104),
            G2dWindowFieldPilot<DWORD>(window, 0x1a8), G2dWindowFieldPilot<DWORD>(window, 0x1ac), &resource);
        if (!status && resource) {
            G2dWindowFieldPilot<BYTE>(resource, 0x14) = 1;
            descriptionNode = InsertG2dDepthByDescriptionBoundaryPilot(descriptionTree, key, resource);
            if (descriptionNode && InsertG2dDepthByIdBoundaryPilot(
                    static_cast<BYTE*>(resources) + 0x18, G2dWindowFieldPilot<DWORD>(resource, 8), resource)) {
                *result = resource;
            } else {
                status = -1;
            }
        } else if (!status) {
            status = -1;
        }
    } catch (...) {
        if (descriptionNode) EraseG2dDepthDescriptionNodeBoundaryPilot(descriptionTree, descriptionNode);
        ShareG2dDxgiHolderPilot(resource, NULL, 4);
        throw;
    }
    if (status && descriptionNode)
        EraseG2dDepthDescriptionNodeBoundaryPilot(descriptionTree, descriptionNode);
    ShareG2dDxgiHolderPilot(resource, NULL, 4);
    return status;
}

// The original uses the same checked tree lookup at 206d9950 for both maps.
static __declspec(noinline) void* FindG2dTargetTreeValuePilot(void* tree, ULONG_PTR key)
{
    void* sentinel = G2dWindowFieldPilot<void*>(tree, 4);
    void* node = G2dWindowFieldPilot<void*>(sentinel, 4);
    while (node != sentinel) {
        const ULONG_PTR nodeKey = G2dWindowFieldPilot<ULONG_PTR>(node, 0x0c);
        if (key == nodeKey) return G2dWindowFieldPilot<void*>(node, 0x10);
        node = G2dWindowFieldPilot<void*>(node, key < nodeKey ? 0 : 8);
    }
    return NULL;
}

// 206d9190. The secondary map indexes pooled resources by their ID.
static __declspec(noinline) int ReturnG2dTargetDepthResourcePilot(void* resources, DWORD resourceId)
{
    void* resource = FindG2dTargetTreeValuePilot(static_cast<BYTE*>(resources) + 0x18, resourceId);
    if (!resource) return -1;
    G2dWindowFieldPilot<BYTE>(resource, 0x14) = 0;
    G2dWindowFieldPilot<DWORD>(resource, 0x28) = ReadG2dDepthPoolStampBoundaryPilot();
    return 0;
}

static void ReleaseG2dTargetInterfacePilot(void* pointer)
{
    typedef ULONG (WINAPI *ReleaseMethod)(void*);
    if (pointer) G2dSlotPilot<ReleaseMethod>(pointer, 8)(pointer);
}

// 206db540. GetBuffer(0, IID_ID3D10Texture2D) returns a temporary interface.
static __declspec(noinline) int ReadG2dTargetBackBufferDescPilot(void* target, DWORD* desc)
{
    typedef HRESULT (WINAPI *GetBufferMethod)(void*, UINT, const GUID&, void**);
    typedef void (WINAPI *GetTextureDescMethod)(void*, DWORD*);
    static const GUID texture2d = {
        0x9b7e4c04, 0x342c, 0x4106, {0xa1, 0x9f, 0x4f, 0x27, 0x04, 0xf6, 0x89, 0xf0}
    };
    void* swapChain = G2dWindowFieldPilot<void*>(target, 0x10);
    void* texture = NULL;
    const HRESULT status = G2dSlotPilot<GetBufferMethod>(swapChain, 0x24)(swapChain, 0, texture2d, &texture);
    if (FAILED(status)) {
        ReleaseG2dTargetInterfacePilot(texture);
        return -1;
    }
    G2dSlotPilot<GetTextureDescMethod>(texture, 0x30)(texture, desc);
    ReleaseG2dTargetInterfacePilot(texture);
    return 0;
}

static ULONGLONG ReadG2dDepthKeyPilot(void* object)
{
    return (static_cast<ULONGLONG>(G2dWindowFieldPilot<DWORD>(object, 0x10)) << 32)
        | G2dWindowFieldPilot<DWORD>(object, 0x0c);
}

static void* NextG2dDepthPoolNodePilot(void* node, void* sentinel)
{
    void* next = G2dWindowFieldPilot<void*>(node, 8);
    if (next != sentinel) {
        while (G2dWindowFieldPilot<void*>(next, 0) != sentinel)
            next = G2dWindowFieldPilot<void*>(next, 0);
        return next;
    }
    next = G2dWindowFieldPilot<void*>(node, 4);
    while (next != sentinel && node == G2dWindowFieldPilot<void*>(next, 8)) {
        node = next;
        next = G2dWindowFieldPilot<void*>(next, 4);
    }
    return next;
}

// 206d9060. Multiple entries can share the same resource key.
static __declspec(noinline) int AcquireG2dTargetDepthResourcePilot(
    void* resources, const ULONGLONG* key, void** result)
{
    void* sentinel = G2dWindowFieldPilot<void*>(resources, 0x10);
    void* node = G2dWindowFieldPilot<void*>(sentinel, 4);
    void* first = sentinel;
    while (node != sentinel) {
        if (ReadG2dDepthKeyPilot(node) < *key) {
            node = G2dWindowFieldPilot<void*>(node, 8);
        } else {
            first = node;
            node = G2dWindowFieldPilot<void*>(node, 0);
        }
    }

    for (node = first; node != sentinel; node = NextG2dDepthPoolNodePilot(node, sentinel)) {
        void* resource = G2dWindowFieldPilot<void*>(node, 0x14);
        if (ReadG2dDepthKeyPilot(resource) != *key) break;
        BYTE& reserved = G2dWindowFieldPilot<BYTE>(resource, 0x14);
        if (!reserved) {
            reserved = 1;
            *result = resource;
            return 0;
        }
    }
    return CreateG2dTargetDepthResourcePilot(resources, key, result);
}

// 206d8c40. A matching descriptor keeps the existing view.
static __declspec(noinline) int ConfigureG2dTargetDepthViewPilot(void* resource, const DWORD* desc)
{
    typedef HRESULT (WINAPI *CreateViewMethod)(void*, void*, const DWORD*, void**);
    void*& view = G2dWindowFieldPilot<void*>(resource, 0x24);
    DWORD& tag = G2dWindowFieldPilot<DWORD>(resource, 0x20);
    DWORD* savedDesc = &G2dWindowFieldPilot<DWORD>(resource, 0x40);
    const size_t descBytes = 5 * sizeof(DWORD);
    if (view && tag == 2 && memcmp(savedDesc, desc, descBytes) == 0) return 0;
    memcpy(savedDesc, desc, descBytes);
    void* previous = view;
    view = NULL;
    ReleaseG2dTargetInterfacePilot(previous);
    void* device = G2dWindowFieldPilot<void*>(resource, 0x18);
    void* texture = G2dWindowFieldPilot<void*>(resource, 0x1c);
    const HRESULT status = G2dSlotPilot<CreateViewMethod>(device, 0x134)(device, texture, savedDesc, &view);
    if (FAILED(status)) {
        tag = 0;
        memset(savedDesc, 0, descBytes);
        return -1;
    }
    tag = 2;
    return 0;
}
