#pragma once

// Shader slot +4 targets 206800b0, 20680100 and 20680150.
static int CreateG2dShaderPilot(void* shader, unsigned int stage)
{
    typedef SIZE_T (WINAPI *BlobSizeMethod)(void*);
    typedef void* (WINAPI *BlobPointerMethod)(void*);
    typedef HRESULT (WINAPI *CreateShaderMethod)(void*, const void*, SIZE_T, void**);
    static const unsigned int deviceSlots[] = {
        0x13c, // ID3D10Device::CreateVertexShader
        0x140, // ID3D10Device::CreateGeometryShader
        0x148  // ID3D10Device::CreatePixelShader
    };

    void* blob = G2dWindowFieldPilot<void*>(shader, 0x44);
    void* device = G2dWindowFieldPilot<void*>(shader, 0x48);
    const SIZE_T bytecodeSize = G2dSlotPilot<BlobSizeMethod>(blob, 0x10)(blob);
    const void* bytecode = G2dSlotPilot<BlobPointerMethod>(blob, 0x0c)(blob);
    void** shaderInterface = &G2dWindowFieldPilot<void*>(shader, 0x70);
    const HRESULT result = G2dSlotPilot<CreateShaderMethod>(device, deviceSlots[stage])(
        device, bytecode, bytecodeSize, shaderInterface);
    return FAILED(result) ? -1 : 0;
}
