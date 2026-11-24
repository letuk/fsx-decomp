#pragma once

struct G2dShaderDefinePilot {
    std::string name;
    std::string value;
    const char* nameText;
    const char* valueText;
};
C_ASSERT(sizeof(G2dShaderDefinePilot) == 0x40);

// In the original checked VC8 vectors, a proxy word precedes the pointer triple.
struct G2dShaderRecordObservedPilot {
    std::string path;
    std::string entry;
    DWORD options;
    DWORD defineProxy;
    G2dShaderDefinePilot* definesBegin;
    G2dShaderDefinePilot* definesEnd;
    G2dShaderDefinePilot* definesCapacity;
};
C_ASSERT(sizeof(G2dShaderRecordObservedPilot) == 0x4c);

extern void* g_g2dShaderCatalogueTablePilot[]; // 2066f820
extern void* g_g2dVertexShaderTablePilot[]; // 2066f790
extern void* g_g2dGeometryShaderTablePilot[]; // 2066f7c0
extern void* g_g2dPixelShaderTablePilot[]; // 2066f7f0
#include "dxgi_shader_lifetime.h"
#include "dxgi_shader_loading.h"
#include "dxgi_shader_creation.h"

struct G2dPostProcessShaderPilot {
    const char* filename;
    const char* entry;
    const char* define;
};

static const G2dPostProcessShaderPilot g_g2dPostProcessShadersPilot[] = {
    {"Quad.vsh", "VsQuadMain", NULL},
    {"Quad.gsh", "GsQuadMain", NULL},
    {"Quad.gsh", "GsQuadMain", "GSQUAD_EMIT_TEXCOORD"},
    {"Quad.psh", "PsQuadMain", NULL},
    {"Quad.psh", "PsQuadMain", "GSQUAD_EMIT_TEXCOORD"},
    {"BrightPass.psh", "PsBrightPassMain", "GSQUAD_EMIT_TEXCOORD"},
    {"GaussBlur.psh", "PsGaussBlurMain", "GAUSS_BLUR_HORZ"},
    {"GaussBlur.psh", "PsGaussBlurMain", "GAUSS_BLUR_VERT"},
    {"HdrBlend.psh", "PsHdrBlendMain", "GSQUAD_EMIT_TEXCOORD"},
    {"DownSample.psh", "PsDownSampleMain", "GSQUAD_EMIT_TEXCOORD"},
    {"InitLum.psh", "PsInitLumMain", "GSQUAD_EMIT_TEXCOORD"},
    {"FinalLum.psh", "PsFinalLumMain", "GSQUAD_EMIT_TEXCOORD"}
};

static void ConstructG2dShaderCataloguePilot(void* holder, void* device)
{
    const size_t count = _countof(g_g2dPostProcessShadersPilot);
    void* shaderStorage = AllocateG2dWindowStoragePilot(count * sizeof(void*));
    if (!shaderStorage) throw std::bad_alloc();
    void* recordStorage = NULL;
    G2dShaderRecordObservedPilot* records = NULL;
    size_t constructed = 0;
    try {
        recordStorage = AllocateG2dWindowStoragePilot(count * sizeof(*records));
        if (!recordStorage) throw std::bad_alloc();
        records = static_cast<G2dShaderRecordObservedPilot*>(recordStorage);
        for (size_t i = 0; i < count; ++i) {
            const G2dPostProcessShaderPilot& spec = g_g2dPostProcessShadersPilot[i];
            G2dShaderRecordObservedPilot& record = *new (records + i) G2dShaderRecordObservedPilot();
            ++constructed;
            record.path = std::string("ShadersHLSL\\PostProcess\\") + spec.filename;
            record.entry = spec.entry;
            if (spec.define) {
                void* defineStorage = AllocateG2dWindowStoragePilot(sizeof(G2dShaderDefinePilot));
                if (!defineStorage) throw std::bad_alloc();
                try {
                    G2dShaderDefinePilot* define = new (defineStorage) G2dShaderDefinePilot();
                    record.definesBegin = define;
                    record.definesEnd = record.definesCapacity = define + 1;
                    define->name = spec.define;
                    define->value = "1";
                    define->nameText = define->name.c_str();
                    define->valueText = define->value.c_str();
                } catch (...) {
                    if (!record.definesBegin) FreeG2dHeapSlotBoundaryPilot(&defineStorage);
                    throw;
                }
            }
        }
    } catch (...) {
        for (size_t i = constructed; i != 0; --i) {
            G2dShaderRecordObservedPilot& record = records[i - 1];
            if (record.definesBegin) {
                record.definesBegin->~G2dShaderDefinePilot();
                void* storage = record.definesBegin;
                FreeG2dHeapSlotBoundaryPilot(&storage);
            }
            record.~G2dShaderRecordObservedPilot();
        }
        if (recordStorage) FreeG2dHeapSlotBoundaryPilot(&recordStorage);
        FreeG2dHeapSlotBoundaryPilot(&shaderStorage);
        throw;
    }

    void** shaders = static_cast<void**>(shaderStorage);
    memset(shaders, 0, count * sizeof(*shaders));
    G2dWindowFieldPilot<void**>(holder, 0) = g_g2dShaderCatalogueTablePilot;
    G2dWindowFieldPilot<BYTE>(holder, 4) = 0;
    G2dWindowFieldPilot<void*>(holder, 8) = device;
    G2dWindowFieldPilot<DWORD>(holder, 0x0c) = 0;
    G2dWindowFieldPilot<DWORD>(holder, 0x1c) = 0;
    G2dWindowFieldPilot<void**>(holder, 0x10) = shaders;
    G2dWindowFieldPilot<void**>(holder, 0x14) = shaders + count;
    G2dWindowFieldPilot<void**>(holder, 0x18) = shaders + count;
    G2dWindowFieldPilot<G2dShaderRecordObservedPilot*>(holder, 0x20) = records;
    G2dWindowFieldPilot<G2dShaderRecordObservedPilot*>(holder, 0x24) = records + count;
    G2dWindowFieldPilot<G2dShaderRecordObservedPilot*>(holder, 0x28) = records + count;
    G2dWindowFieldPilot<LONG>(holder, 0x2c) = 0;
}

static int InitializeG2dShaderCataloguePilot(void* holder)
{
    G2dShaderRecordObservedPilot* record = G2dWindowFieldPilot<G2dShaderRecordObservedPilot*>(holder, 0x20);
    G2dShaderRecordObservedPilot* end = G2dWindowFieldPilot<G2dShaderRecordObservedPilot*>(holder, 0x24);
    void** shaders = G2dWindowFieldPilot<void**>(holder, 0x10);
    for (; record != end; ++record, ++shaders) {
        static const char* const extensions[] = {".vsh", ".gsh", ".psh"};
        void** tables[] = {g_g2dVertexShaderTablePilot, g_g2dGeometryShaderTablePilot, g_g2dPixelShaderTablePilot};
        unsigned int stage = 0;
        while (stage < _countof(extensions) && record->path.find(extensions[stage]) == std::string::npos) ++stage;
        if (stage == _countof(extensions)) return -1;
        void* storage = AllocateG2dWindowStoragePilot(sizeof(G2dShaderObjectObservedPilot));
        if (!storage) return -1;
        G2dShaderObjectObservedPilot* shader;
        try {
            shader = ConstructG2dShaderObjectPilot(storage, stage, tables[stage]);
        } catch (...) {
            FreeG2dHeapSlotBoundaryPilot(&storage);
            throw;
        }
        int status;
        try {
            G2dWindowFieldPilot<std::string>(shader, 0x0c) = record->path;
            G2dWindowFieldPilot<std::string>(shader, 0x28) = record->entry;
            G2dWindowFieldPilot<void*>(shader, 0x48) = G2dWindowFieldPilot<void*>(holder, 8);
            G2dWindowFieldPilot<DWORD>(shader, 0x5c) = record->options;
            CopyG2dShaderDefinesPilot(*shader, *record);
            status = PrepareG2dShaderPilot(shader);
            if (!status) status = LoadG2dShaderPilot(shader);
            if (!status) status = CreateG2dShaderPilot(shader, stage);
        } catch (...) {
            DeleteG2dShaderObjectPilot(shader);
            throw;
        }
        if (status) {
            DeleteG2dShaderObjectPilot(shader);
            return status;
        }
        G2dWindowFieldPilot<BYTE>(shader, 8) = 1;
        *shaders = shader;
    }
    G2dWindowFieldPilot<BYTE>(holder, 4) = 1;
    return 0;
}
