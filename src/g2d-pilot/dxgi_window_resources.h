#pragma once

extern void* g_g2dWindowDeviceResourcesTablePilot[]; // 2066f82c
extern void* g_g2dWindowSharedStateTablePilot[]; // 20668514
extern void* g_g2dQuadConstantTablePilot[]; // 2066f51c
extern void** g_g2dWindowResourcesBeginPilot; // 20709b68
extern void** g_g2dWindowResourcesEndPilot; // 20709b6c

extern "C" void __cdecl RegisterG2dResourceCleanupBoundaryPilot(); // api.dll #231, group 5, target 206d9580
extern "C" void __cdecl AppendG2dWindowResourcesBoundaryPilot(void*); // 206d99f0
#include "dxgi_shader_catalogue.h"
#include "dxgi_quad_constant.h"
#include "dxgi_window_state_reset.h"
#include "dxgi_quad_draw.h"
#include "dxgi_quad_clear.h"
#include "dxgi_clear_entrypoints.h"
#include "dxgi_clear_targets.h"

// 2067e860. The inline constant is constructed even if initialization failed.
static void DestroyG2dWindowSharedStatePilot(void* holder)
{
    G2dWindowFieldPilot<void**>(holder, 0) = g_g2dWindowSharedStateTablePilot;
    void* constant = static_cast<BYTE*>(holder) + 0x20;
    if (G2dWindowFieldPilot<BYTE>(holder, 0x0c)) {
        try {
            ResetG2dWindowSharedStatePilot(holder);
        } catch (...) {
            DestroyG2dQuadConstantPilot(constant);
            throw;
        }
    }
    DestroyG2dQuadConstantPilot(constant);
}

// 2067e7b0. EDX is unused; the deletion flags arrive on the stack.
void* __fastcall DeleteG2dWindowSharedStateSourcePilot(void* holder, void*, DWORD flags)
{
    DestroyG2dWindowSharedStatePilot(holder);
    if (flags & 1) {
        void* storage = holder;
        FreeG2dHeapSlotBoundaryPilot(&storage);
    }
    return holder;
}

// 2067fad0. Cache the reflected slot after validating a constant buffer's size.
static bool BindG2dShaderConstantPilot(void* constant, void* shader)
{
    typedef UINT (__thiscall *ConstantByteSizeMethod)(void*);
    UINT& slot = G2dWindowFieldPilot<UINT>(constant, 0x24);
    slot = static_cast<UINT>(-1);
    const G2dShaderBindingsPilot& bindings = G2dWindowFieldPilot<G2dShaderBindingsPilot>(shader, 0x4c);
    const std::string& name = G2dWindowFieldPilot<std::string>(constant, 8);
    const G2dShaderBindingPilot* binding = FindG2dShaderBindingPilot(bindings, name);
    if (!binding) return false;
    if (binding->type == 0 &&
        binding->constantBufferSize != G2dSlotPilot<ConstantByteSizeMethod>(constant, 0)(constant))
        return false;
    slot = binding->bindPoint;
    return true;
}

static void InitializeG2dResourceSentinelPilot(void* node, unsigned int marker)
{
    G2dWindowFieldPilot<void*>(node, 0) = node;
    G2dWindowFieldPilot<void*>(node, 4) = node;
    G2dWindowFieldPilot<void*>(node, 8) = node;
    G2dWindowFieldPilot<BYTE>(node, marker - 1) = 1;
    G2dWindowFieldPilot<BYTE>(node, marker) = 1;
}

// 206d8e30 and 206d8d70. The holder has two empty trees and a window back-pointer.
int CreateG2dWindowDeviceResourcesSourcePilot(void* window, void** destination)
{
    void* holder = AllocateG2dWindowStoragePilot(0x24);
    if (!holder) {
        ShareG2dDxgiHolderPilot(*destination, NULL, 4);
        return 0;
    }

    void* first = NULL;
    void* second = NULL;
    try {
        first = AllocateG2dWindowStoragePilot(0x1c);
        second = AllocateG2dWindowStoragePilot(0x18);
        if (!first || !second) throw std::bad_alloc();
        G2dWindowFieldPilot<void**>(holder, 0) = g_g2dWindowDeviceResourcesTablePilot;
        G2dWindowFieldPilot<LONG>(holder, 4) = 0;
        G2dWindowFieldPilot<void*>(holder, 8) = window;
        InitializeG2dResourceSentinelPilot(first, 0x19);
        InitializeG2dResourceSentinelPilot(second, 0x15);
        G2dWindowFieldPilot<void*>(holder, 0x10) = first;
        G2dWindowFieldPilot<DWORD>(holder, 0x14) = 0;
        G2dWindowFieldPilot<void*>(holder, 0x1c) = second;
        G2dWindowFieldPilot<DWORD>(holder, 0x20) = 0;
        if (!g_g2dWindowResourcesBeginPilot || g_g2dWindowResourcesBeginPilot == g_g2dWindowResourcesEndPilot)
            RegisterG2dResourceCleanupBoundaryPilot();
        AppendG2dWindowResourcesBoundaryPilot(holder);
    } catch (...) {
        if (second) FreeG2dHeapSlotBoundaryPilot(&second);
        if (first) FreeG2dHeapSlotBoundaryPilot(&first);
        FreeG2dHeapSlotBoundaryPilot(&holder);
        throw;
    }
    ShareG2dDxgiHolderPilot(*destination, holder, 4);
    return 0;
}

// 206d74c0. Publish the counted holder before initializing its shader entries.
int CreateG2dDeviceSharedStateSourcePilot(void* device, void** destination)
{
    void* holder = AllocateG2dWindowStoragePilot(0x30);
    if (holder) {
        try {
            ConstructG2dShaderCataloguePilot(holder, device);
        } catch (...) {
            FreeG2dHeapSlotBoundaryPilot(&holder);
            throw;
        }
    }
    ShareG2dDxgiHolderPilot(*destination, holder, 0x2c);
    return holder ? InitializeG2dShaderCataloguePilot(holder) : -1;
}

// 2067e8d0. Entries 0, 1 and 3 are borrowed from the device's catalogue.
static int InitializeG2dWindowSharedStatePilot(void* holder)
{
    memset(static_cast<BYTE*>(holder) + 0x84, 0, 0x20);
    void* window = G2dWindowFieldPilot<void*>(holder, 8);
    void* catalogue = G2dWindowFieldPilot<void*>(window, 0x168);
    void** entries = G2dWindowFieldPilot<void**>(catalogue, 0x10);
    void** end = G2dWindowFieldPilot<void**>(catalogue, 0x14);
    if (!entries || end - entries < 4) return -1;
    G2dWindowFieldPilot<void*>(holder, 0x10) = entries[0];
    G2dWindowFieldPilot<void*>(holder, 0x14) = entries[1];
    G2dWindowFieldPilot<void*>(holder, 0x18) = entries[3];

    void* constant = static_cast<BYTE*>(holder) + 0x20;
    int status = InitializeG2dQuadConstantPilot(constant, G2dWindowFieldPilot<void*>(window, 0x104));
    if (status) return status;
    if (!BindG2dShaderConstantPilot(constant, entries[1])) return -1;
    G2dWindowFieldPilot<BYTE>(holder, 0x0c) = 1;
    return 0;
}

// 2067e6e0 and 2067e620. The quad constant uses two inline 0x14-byte buffers.
int CreateG2dWindowSharedStateSourcePilot(void* window, void** destination)
{
    void* holder = AllocateG2dWindowStoragePilot(0xa4);
    if (holder) {
        try {
            G2dWindowFieldPilot<void**>(holder, 0) = g_g2dWindowSharedStateTablePilot;
            G2dWindowFieldPilot<LONG>(holder, 4) = 0;
            G2dWindowFieldPilot<void*>(holder, 8) = window;
            G2dWindowFieldPilot<BYTE>(holder, 0x0c) = 0;
            memset(static_cast<BYTE*>(holder) + 0x10, 0, 0x0c);
            void* constant = static_cast<BYTE*>(holder) + 0x20;
            ConstructG2dQuadConstantPilot(constant, "GsQuadCb", 0x14,
                static_cast<BYTE*>(holder) + 0x5c, static_cast<BYTE*>(holder) + 0x70);
            memset(static_cast<BYTE*>(holder) + 0x84, 0, 0x20);
        } catch (...) {
            FreeG2dHeapSlotBoundaryPilot(&holder);
            throw;
        }
    }
    ShareG2dDxgiHolderPilot(*destination, holder, 4, DeleteG2dWindowSharedStateSourcePilot);
    return holder ? InitializeG2dWindowSharedStatePilot(holder) : -1;
}
