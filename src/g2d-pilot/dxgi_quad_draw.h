#pragma once

#include "dxgi_quad_draw_state.h"

// 2068d500. The original updates the CPU snapshot before attempting Map.
static int UploadG2dQuadConstantPilot(void* constant)
{
    typedef HRESULT (WINAPI *MapMethod)(void*, UINT, UINT, void**);
    typedef void (WINAPI *UnmapMethod)(void*);
    const UINT size = G2dWindowFieldPilot<UINT>(constant, 0x2c);
    const void* current = G2dWindowFieldPilot<void*>(constant, 0x30);
    void* previous = G2dWindowFieldPilot<void*>(constant, 0x34);
    if (memcmp(current, previous, size) == 0) return 0;

    memcpy(previous, current, size);
    void* buffer = G2dWindowFieldPilot<void*>(constant, 0x38);
    void* mapped = NULL;
    const UINT writeDiscard = 4; // D3D10_MAP_WRITE_DISCARD
    if (FAILED(G2dSlotPilot<MapMethod>(buffer, 0x28)(buffer, writeDiscard, 0, &mapped))) return -1;
    memcpy(mapped, previous, size);
    G2dSlotPilot<UnmapMethod>(buffer, 0x2c)(buffer);
    return 0;
}

// 2067f500. Clear only the pipeline state selected by the entry.
static void ClearG2dQuadPipelineStatePilot(void* entry)
{
    typedef void (WINAPI *SetPointerMethod)(void*, void*);
    typedef void (WINAPI *SetDepthStencilMethod)(void*, void*, UINT);
    typedef void (WINAPI *SetBlendMethod)(void*, void*, const FLOAT*, UINT);
    const BYTE flags = G2dWindowFieldPilot<BYTE>(entry, 0);
    void* device = G2dWindowFieldPilot<void*>(entry, 8);
    if (flags & 1) G2dSlotPilot<SetPointerMethod>(device, 0x74)(device, NULL); // RSSetState
    if (flags & 2) G2dSlotPilot<SetDepthStencilMethod>(device, 0x68)(device, NULL, 0);
    if (flags & 4) G2dSlotPilot<SetBlendMethod>(device, 0x64)(device, NULL, NULL, 0xffffffffu);
    if (flags & 8) G2dSlotPilot<SetPointerMethod>(device, 0x2c)(device, NULL); // IASetInputLayout
}

// 2067ea50. The original carries holder and stateArgument in EAX and ECX.
int DrawG2dQuadSourcePilot(void* holder, DWORD stateArgument, UINT entryIndex,
                           const FLOAT* quadValues, FLOAT extraValue)
{
    typedef void (WINAPI *SetLayoutMethod)(void*, void*);
    typedef void (WINAPI *SetTopologyMethod)(void*, UINT);
    typedef void (WINAPI *DrawMethod)(void*, UINT, UINT);
    typedef void (__thiscall *BindConstantMethod)(void*, void*);
    void* window = G2dWindowFieldPilot<void*>(holder, 8);
    void* device = G2dWindowFieldPilot<void*>(window, 0x104);
    const int prepareStatus = PrepareG2dQuadDrawStatePilot(holder, entryIndex, stateArgument);
    if (prepareStatus) return prepareStatus;

    const UINT pointList = 1; // D3D10_PRIMITIVE_TOPOLOGY_POINTLIST
    G2dSlotPilot<SetLayoutMethod>(device, 0x2c)(device, NULL);
    G2dSlotPilot<SetTopologyMethod>(device, 0x48)(device, pointList);

    void* shaders[] = {
        G2dWindowFieldPilot<void*>(holder, 0x10),
        G2dWindowFieldPilot<void*>(holder, 0x14),
        G2dWindowFieldPilot<void*>(holder, 0x18)
    };
    for (unsigned int i = 0; i < _countof(shaders); ++i)
        G2dSlotPilot<G2dNotifyMethodPilot>(shaders[i], 8)(shaders[i]);

    memcpy(static_cast<BYTE*>(holder) + 0x5c, quadValues, 4 * sizeof(*quadValues));
    G2dWindowFieldPilot<FLOAT>(holder, 0x6c) = extraValue;
    void* constant = static_cast<BYTE*>(holder) + 0x20;
    const int uploadStatus = UploadG2dQuadConstantPilot(constant);
    if (uploadStatus) return uploadStatus;

    const DWORD constantStage = G2dWindowFieldPilot<DWORD>(constant, 4);
    if (constantStage < 3)
        G2dSlotPilot<BindConstantMethod>(shaders[1], 0x14 + constantStage * 4)(shaders[1], constant);
    G2dSlotPilot<DrawMethod>(device, 0x24)(device, 1, 0);
    ClearG2dQuadPipelineStatePilot(G2dWindowFieldPilot<void*>(holder, 0x84 + entryIndex * sizeof(void*)));
    for (unsigned int i = 0; i < _countof(shaders); ++i)
        G2dSlotPilot<G2dNotifyMethodPilot>(shaders[i], 0x0c)(shaders[i]);
    return 0;
}
