#pragma once

// 2067e570. Quad constant table slot zero receives the object in ECX.
__declspec(naked) UINT __fastcall GetG2dQuadConstantByteWidthSourcePilot(void*)
{
    __asm {
        mov eax, [ecx + 2ch]
        mov ecx, eax
        and ecx, 0fh
        jbe aligned
        sub eax, ecx
        add eax, 10h
    aligned:
        ret
    }
}

extern void* g_g2dConstantBufferBaseTablePilot[]; // 2066850c
extern void* g_g2dShaderResourceBaseTablePilot[]; // 20668de0

// 2067e7e0. The CPU payload pointers refer to storage owned by the holder.
static void DestroyG2dQuadConstantPilot(void* constant)
{
    typedef ULONG (WINAPI *ReleaseMethod)(void*);
    G2dWindowFieldPilot<void**>(constant, 0) = g_g2dConstantBufferBaseTablePilot;
    void* buffer = G2dWindowFieldPilot<void*>(constant, 0x38);
    if (buffer) G2dSlotPilot<ReleaseMethod>(buffer, 8)(buffer);
    G2dWindowFieldPilot<void**>(constant, 0) = g_g2dShaderResourceBaseTablePilot;
    G2dWindowFieldPilot<std::string>(constant, 8).~basic_string();
}

// 2067ec80. Only the standalone deleting entry can free the receiver itself.
void* __fastcall DeleteG2dQuadConstantSourcePilot(void* constant, void*, DWORD flags)
{
    DestroyG2dQuadConstantPilot(constant);
    if (flags & 1) {
        void* storage = constant;
        FreeG2dHeapSlotBoundaryPilot(&storage);
    }
    return constant;
}

static void ZeroG2dQuadConstantPayloadsPilot(void* constant)
{
    const UINT size = G2dWindowFieldPilot<UINT>(constant, 0x2c);
    memset(G2dWindowFieldPilot<void*>(constant, 0x30), 0, size);
    memset(G2dWindowFieldPilot<void*>(constant, 0x34), 0, size);
}

// 2068d3f0 and the final table installed by the holder constructor.
static void ConstructG2dQuadConstantPilot(void* constant, const std::string& name,
                                          UINT size, void* currentData, void* previousData)
{
    new (static_cast<BYTE*>(constant) + 8) std::string(name);
    G2dWindowFieldPilot<void**>(constant, 0) = g_g2dQuadConstantTablePilot;
    G2dWindowFieldPilot<DWORD>(constant, 4) = 0;
    G2dWindowFieldPilot<BYTE>(constant, 0x28) = 0;
    G2dWindowFieldPilot<UINT>(constant, 0x2c) = size;
    G2dWindowFieldPilot<void*>(constant, 0x30) = currentData;
    G2dWindowFieldPilot<void*>(constant, 0x34) = previousData;
    G2dWindowFieldPilot<void*>(constant, 0x38) = NULL;
    ZeroG2dQuadConstantPayloadsPilot(constant);
}

// x86 D3D10_BUFFER_DESC. Keep the XP compiler independent of newer SDK headers.
struct G2dQuadBufferDescPilot {
    UINT byteWidth;
    UINT usage;
    UINT bindFlags;
    UINT cpuAccessFlags;
    UINT miscFlags;
};
C_ASSERT(sizeof(G2dQuadBufferDescPilot) == 0x14);

// 2068d460. ESI and EDI are the original constant and device receivers.
static int InitializeG2dQuadConstantPilot(void* constant, void* device)
{
    typedef HRESULT (WINAPI *CreateBufferMethod)(void*, const G2dQuadBufferDescPilot*, const void*, void**);
    ZeroG2dQuadConstantPayloadsPilot(constant);
    const G2dQuadBufferDescPilot desc = {
        GetG2dQuadConstantByteWidthSourcePilot(constant),
        2,      // D3D10_USAGE_DYNAMIC
        4,      // D3D10_BIND_CONSTANT_BUFFER
        0x10000, // D3D10_CPU_ACCESS_WRITE
        0
    };
    void** buffer = &G2dWindowFieldPilot<void*>(constant, 0x38);
    if (FAILED(G2dSlotPilot<CreateBufferMethod>(device, 0x11c)(device, &desc, NULL, buffer))) return -1;
    G2dWindowFieldPilot<BYTE>(constant, 0x28) = 1;
    return 0;
}
