#pragma once

struct G2dShaderMacroPilot {
    const char* name;
    const char* definition;
};
C_ASSERT(sizeof(G2dShaderMacroPilot) == 8);

// Source interfaces for the original d3dx10_34.dll imports.
extern "C" HRESULT WINAPI CompileG2dShaderFileBoundaryPilot(
    const char*, const G2dShaderMacroPilot*, void*, const char*, const char*,
    UINT, UINT, void*, void**, void**, HRESULT*); // IAT 206614bc
extern "C" HRESULT WINAPI ReflectG2dShaderBoundaryPilot(
    const void*, SIZE_T, void**); // IAT 206614b0

extern std::string g_g2dShaderProfileSuffixesPilot[]; // 20705370

class G2dShaderInterfacePilot {
public:
    G2dShaderInterfacePilot() : pointer(NULL) {}
    ~G2dShaderInterfacePilot()
    {
        if (pointer) G2dSlotPilot<ULONG (WINAPI*)(void*)>(pointer, 8)(pointer);
    }

    void* pointer;
private:
    G2dShaderInterfacePilot(const G2dShaderInterfacePilot&);
    G2dShaderInterfacePilot& operator=(const G2dShaderInterfacePilot&);
};

// 2067fe30. Slot +0x2c returns the stage prefix in the caller's string storage.
static int PrepareG2dShaderPilot(void* shader)
{
    typedef std::string* (__thiscall *StagePrefixMethod)(void*, void*);
    typedef void* (WINAPI *BlobPointerMethod)(void*);

    const DWORD profileIndex = G2dWindowFieldPilot<DWORD>(shader, 0x5c);
    const std::string suffix(g_g2dShaderProfileSuffixesPilot[profileIndex]);
    std::string profile;
    __declspec(align(4)) BYTE stageStorage[sizeof(std::string)];
    std::string* stage = G2dSlotPilot<StagePrefixMethod>(shader, 0x2c)(shader, stageStorage);
    try {
        profile = *stage + suffix;
    } catch (...) {
        stage->~basic_string();
        throw;
    }
    stage->~basic_string();

    const G2dShaderDefinePilot* begin = G2dWindowFieldPilot<G2dShaderDefinePilot*>(shader, 0x64);
    const G2dShaderDefinePilot* end = G2dWindowFieldPilot<G2dShaderDefinePilot*>(shader, 0x68);
    std::vector<G2dShaderMacroPilot> macros;
    if (begin && begin != end) {
        macros.resize(end - begin + 1);
        for (size_t i = 0; i + 1 < macros.size(); ++i) {
            macros[i].name = begin[i].nameText;
            macros[i].definition = begin[i].valueText;
        }
    }

    G2dShaderInterfacePilot diagnostics;
    HRESULT status = CompileG2dShaderFileBoundaryPilot(
        G2dWindowFieldPilot<std::string>(shader, 0x0c).c_str(),
        macros.empty() ? NULL : &macros[0], NULL,
        G2dWindowFieldPilot<std::string>(shader, 0x28).c_str(), profile.c_str(),
        0x800, 0, NULL, &G2dWindowFieldPilot<void*>(shader, 0x44), &diagnostics.pointer, NULL);
    if (FAILED(status) && diagnostics.pointer)
        G2dSlotPilot<BlobPointerMethod>(diagnostics.pointer, 0x0c)(diagnostics.pointer);
    return FAILED(status) ? -1 : 0;
}

// These x86 SDK descriptor layouts are the outputs of the original reflection slots.
struct G2dReflectedShaderDescPilot {
    DWORD version;
    const char* creator;
    UINT flags;
    UINT constantBuffers;
    UINT boundResources;
    DWORD remaining[23];
};
C_ASSERT(sizeof(G2dReflectedShaderDescPilot) == 0x70);

struct G2dReflectedBindingDescPilot {
    const char* name;
    UINT type;
    UINT bindPoint;
    UINT bindCount;
    DWORD remaining[4];
};
C_ASSERT(sizeof(G2dReflectedBindingDescPilot) == 0x20);

struct G2dReflectedBufferDescPilot {
    const char* name;
    UINT type;
    UINT variables;
    UINT size;
    UINT flags;
};
C_ASSERT(sizeof(G2dReflectedBufferDescPilot) == 0x14);

// 2067fbb0. The constant-buffer view is borrowed from the reflection interface.
static int LoadG2dShaderPilot(void* shader)
{
    typedef void* (WINAPI *BlobPointerMethod)(void*);
    typedef SIZE_T (WINAPI *BlobSizeMethod)(void*);
    typedef HRESULT (WINAPI *ShaderDescMethod)(void*, G2dReflectedShaderDescPilot*);
    typedef HRESULT (WINAPI *BindingDescMethod)(void*, UINT, G2dReflectedBindingDescPilot*);
    typedef void* (WINAPI *FindBufferMethod)(void*, const char*);
    typedef HRESULT (WINAPI *BufferDescMethod)(void*, G2dReflectedBufferDescPilot*);

    void* blob = G2dWindowFieldPilot<void*>(shader, 0x44);
    const SIZE_T bytecodeSize = G2dSlotPilot<BlobSizeMethod>(blob, 0x10)(blob);
    const void* bytecode = G2dSlotPilot<BlobPointerMethod>(blob, 0x0c)(blob);
    G2dShaderInterfacePilot reflection;
    if (FAILED(ReflectG2dShaderBoundaryPilot(bytecode, bytecodeSize, &reflection.pointer))) return -1;

    void* reflector = reflection.pointer;
    G2dReflectedShaderDescPilot desc;
    if (FAILED(G2dSlotPilot<ShaderDescMethod>(reflector, 0x0c)(reflector, &desc))) return -1;

    for (UINT i = 0; i < desc.boundResources; ++i) {
        G2dReflectedBindingDescPilot binding;
        if (FAILED(G2dSlotPilot<BindingDescMethod>(reflector, 0x18)(reflector, i, &binding))) return -1;
        G2dShaderBindingPilot record;
        record.name = binding.name;
        record.bindPoint = binding.bindPoint;
        record.bindCount = binding.bindCount;
        record.type = binding.type;
        record.constantBufferSize = 0;
        if (binding.type == 0) { // D3D10_SIT_CBUFFER
            void* buffer = G2dSlotPilot<FindBufferMethod>(reflector, 0x14)(reflector, binding.name);
            G2dReflectedBufferDescPilot bufferDesc;
            if (!buffer || FAILED(G2dSlotPilot<BufferDescMethod>(buffer, 0)(buffer, &bufferDesc))) return -1;
            record.constantBufferSize = bufferDesc.size;
        }
        InsertG2dShaderBindingPilot(G2dWindowFieldPilot<G2dShaderBindingsPilot>(shader, 0x4c), record);
    }
    return 0;
}
