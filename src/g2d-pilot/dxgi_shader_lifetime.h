#pragma once

extern void* g_g2dBaseShaderTablePilot[]; // 20668658
#include "dxgi_shader_bindings.h"

struct G2dShaderObjectObservedPilot {
    void** table;
    DWORD stage;
    BYTE initialized;
    std::string path;
    std::string entry;
    void* bytecodeBlob;
    void* device;
    G2dShaderBindingsPilot bindings;
    DWORD profileIndex;
    DWORD defineProxy;
    G2dShaderDefinePilot* definesBegin;
    G2dShaderDefinePilot* definesEnd;
    G2dShaderDefinePilot* definesCapacity;
    void* shaderInterface;
};
C_ASSERT(sizeof(G2dShaderObjectObservedPilot) == 0x74);
C_ASSERT(offsetof(G2dShaderObjectObservedPilot, path) == 0x0c);
C_ASSERT(offsetof(G2dShaderObjectObservedPilot, bytecodeBlob) == 0x44);
C_ASSERT(offsetof(G2dShaderObjectObservedPilot, bindings) == 0x4c);
C_ASSERT(offsetof(G2dShaderObjectObservedPilot, definesBegin) == 0x64);
C_ASSERT(offsetof(G2dShaderObjectObservedPilot, shaderInterface) == 0x70);

// 2067fa30 and the stage-specific table installation in 206d7940.
static G2dShaderObjectObservedPilot* ConstructG2dShaderObjectPilot(
    void* storage, DWORD stage, void** table)
{
    G2dShaderObjectObservedPilot* shader = new (storage) G2dShaderObjectObservedPilot();
    shader->table = table;
    shader->stage = stage;
    shader->bindings.growth = 5;
    shader->bindings.flags = 4;
    return shader;
}

static void FreeG2dShaderDefinesPilot(G2dShaderDefinePilot* begin, G2dShaderDefinePilot* end)
{
    if (!begin) return;
    for (; end != begin; ) (--end)->~G2dShaderDefinePilot();
    void* storage = begin;
    FreeG2dHeapSlotBoundaryPilot(&storage);
}

// 20680210. The catalogue copies into a newly constructed, empty range.
static void CopyG2dShaderDefinesPilot(G2dShaderObjectObservedPilot& shader,
                                     const G2dShaderRecordObservedPilot& record)
{
    if (!record.definesBegin || record.definesBegin == record.definesEnd) return;
    const size_t count = record.definesEnd - record.definesBegin;
    G2dShaderDefinePilot* begin = static_cast<G2dShaderDefinePilot*>(
        AllocateG2dWindowStoragePilot(count * sizeof(G2dShaderDefinePilot)));
    if (!begin) throw std::bad_alloc();
    G2dShaderDefinePilot* end = begin;
    try {
        for (size_t i = 0; i < count; ++i) {
            G2dShaderDefinePilot* define = new (end) G2dShaderDefinePilot(record.definesBegin[i]);
            ++end;
            define->nameText = define->name.c_str();
            define->valueText = define->value.c_str();
        }
    } catch (...) {
        FreeG2dShaderDefinesPilot(begin, end);
        throw;
    }
    shader.definesBegin = begin;
    shader.definesEnd = end;
    shader.definesCapacity = end;
}

static void DeleteG2dShaderObjectPilot(G2dShaderObjectObservedPilot* shader)
{
    typedef ULONG (WINAPI *ReleaseMethod)(void*);
    if (shader->shaderInterface)
        G2dSlotPilot<ReleaseMethod>(shader->shaderInterface, 8)(shader->shaderInterface);

    // 2067f710. The device pointer is borrowed.
    shader->table = g_g2dBaseShaderTablePilot;
    FreeG2dShaderDefinesPilot(shader->definesBegin, shader->definesEnd);
    shader->definesBegin = shader->definesEnd = shader->definesCapacity = NULL;
    ClearG2dShaderBindingsPilot(shader->bindings);
    if (shader->bytecodeBlob)
        G2dSlotPilot<ReleaseMethod>(shader->bytecodeBlob, 8)(shader->bytecodeBlob);
    shader->~G2dShaderObjectObservedPilot();
    void* storage = shader;
    FreeG2dHeapSlotBoundaryPilot(&storage);
}
