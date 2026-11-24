#pragma once

extern DWORD g_g2dDxgiDeviceFlagsPilot; // 20704ca4
typedef HRESULT (WINAPI *G2dCreateDevicePilot)(void*, DWORD, HMODULE, UINT, UINT, void**);
extern G2dCreateDevicePilot g_g2dCreateD3d10DevicePilot; // 20704c78
extern "C" const DWORD* __cdecl FindG2dOutputModeBoundaryPilot(void*, DWORD, DWORD, DWORD); // 20697080

// 20691100. The original receives the destination in ESI and window on the stack.
int ConfigureG2dDxgiOutputSourcePilot(void* window, DWORD* mode)
{
    void* output = G2dWindowFieldPilot<void*>(window, 0x100);
    DWORD description[0x60 / sizeof(DWORD)];
    typedef HRESULT (WINAPI *GetOutputDescriptionPilot)(void*, void*);
    if (FAILED(G2dSlotPilot<GetOutputDescriptionPilot>(output, 0x1c)(output, description)))
        return -1;
    mode[1] = description[18] - description[16];
    mode[2] = description[19] - description[17];
    mode[3] = 0;
    mode[4] = 0;
    mode[5] = 0x1c;
    mode[6] = 0;
    mode[7] = 0;
    return 0;
}

// 20690e90. The first pair counts as changed only if both words differ.
static bool G2dDxgiDescriptorChangedPilot(const DWORD* next, const DWORD* previous)
{
    if (next[2] != previous[2] && next[3] != previous[3]) return true;
    static const unsigned int comparedWords[] = {4, 5, 7, 8, 10, 13, 14};
    for (size_t i = 0; i < _countof(comparedWords); ++i) {
        unsigned int word = comparedWords[i];
        if (next[word] != previous[word]) return true;
    }
    return false;
}

// 20690d40. The optional device-flags output originally arrives in EAX.
int PrepareG2dDxgiWindowSourcePilot(void* window, BYTE* descriptorChanged, BYTE* flagsChanged = NULL)
{
    if (descriptorChanged) *descriptorChanged = 0;
    DWORD& deviceFlags = G2dWindowFieldPilot<DWORD>(window, 0x10c);
    if (flagsChanged) *flagsChanged = deviceFlags != g_g2dDxgiDeviceFlagsPilot;
    deviceFlags = g_g2dDxgiDeviceFlagsPilot;

    DWORD* published = &G2dWindowFieldPilot<DWORD>(window, 0x114);
    DWORD previous[15];
    memcpy(previous, published, sizeof(previous));
    DWORD descriptor[15] = {0};
    descriptor[9] = 0x60;
    descriptor[10] = 1;
    descriptor[12] = 1;
    descriptor[14] = 2;
    DWORD* mode = &G2dWindowFieldPilot<DWORD>(window, 0x34);
    void* apiController = ReadApiWindowControllerBoundaryPilot();
    G2dWindowFieldPilot<DWORD>(window, 0xc4) =
        G2dSlotPilot<G2dStatusMethodPilot>(apiController, 0x30)(apiController);
    if (G2dWindowFieldPilot<BYTE>(window, 0xcd)) {
        descriptor[11] = G2dSlotPilot<G2dStatusMethodPilot>(window, 0x10)(window);
        DWORD modeValue = G2dWindowFieldPilot<DWORD>(window, 0xd8);
        DWORD modeKey = G2dSlotPilot<G2dStatusMethodPilot>(window, 0x20)(window);
        DWORD modeIndex = G2dSlotPilot<G2dStatusMethodPilot>(window, 0x38)(window);
        const DWORD* selected = FindG2dOutputModeBoundaryPilot(window, modeIndex, modeKey, modeValue);
        if (!selected) return -1;
        memcpy(mode, selected, 8 * sizeof(DWORD));
    } else {
        int status = ConfigureG2dDxgiOutputSourcePilot(window, mode);
        if (status) return status;
    }
    memcpy(descriptor, mode + 1, 7 * sizeof(DWORD));
    descriptor[7] = 1;
    descriptor[8] = 0;
    if (descriptorChanged) *descriptorChanged = G2dDxgiDescriptorChangedPilot(descriptor, previous);
    memcpy(published, descriptor, sizeof(descriptor));
    return 0;
}

static bool ShareG2dDxgiDevicePilot(void*& destination, void* source)
{
    if (destination != source) {
        typedef ULONG (WINAPI *ComReferencePilot)(void*);
        if (source) G2dSlotPilot<ComReferencePilot>(source, 4)(source);
        if (destination) G2dSlotPilot<ComReferencePilot>(destination, 8)(destination);
        destination = source;
    }
    return destination != NULL;
}

// 20692610 and 20692660. These holders have different reference-count offsets.
typedef void* (__fastcall *G2dDeletingHolderSourcePilot)(void*, void*, DWORD);
void* __fastcall DeleteG2dWindowSharedStateSourcePilot(void*, void*, DWORD);

static bool ShareG2dDxgiHolderPilot(void*& destination, void* source, unsigned int offset,
                                    G2dDeletingHolderSourcePilot deleteHolder = NULL)
{
    if (source) InterlockedIncrement(&G2dWindowFieldPilot<LONG>(source, offset));
    if (destination && InterlockedDecrement(&G2dWindowFieldPilot<LONG>(destination, offset)) == 0) {
        if (deleteHolder) deleteHolder(destination, NULL, 1);
        else G2dSlotPilot<G2dWordMethodPilot>(destination, 0)(destination, 1);
    }
    destination = source;
    return destination != NULL;
}

#include "dxgi_window_resources.h"

// 20690f00. The original carries the window in EDI.
int ConfigureG2dDxgiDeviceSourcePilot(void* window)
{
    void* primary = G2dWindowFieldPilot<void*>(window, 0x108);
    void*& device = G2dWindowFieldPilot<void*>(window, 0x104);
    void*& windowResources = G2dWindowFieldPilot<void*>(window, 0x164);
    void*& deviceState = G2dWindowFieldPilot<void*>(window, 0x168);
    void*& windowState = G2dWindowFieldPilot<void*>(window, 0x160);
    if (primary) {
        if (!ShareG2dDxgiDevicePilot(device, G2dWindowFieldPilot<void*>(primary, 0x104)) ||
            !ShareG2dDxgiHolderPilot(windowResources, G2dWindowFieldPilot<void*>(primary, 0x164), 4) ||
            !ShareG2dDxgiHolderPilot(deviceState, G2dWindowFieldPilot<void*>(primary, 0x168), 0x2c) ||
            !ShareG2dDxgiHolderPilot(windowState, G2dWindowFieldPilot<void*>(primary, 0x160), 4,
                                     DeleteG2dWindowSharedStateSourcePilot)) return -1;
        return 0;
    }

    SetG2dD3dPerfOptionsBoundaryPilot(1);
    HRESULT result = g_g2dCreateD3d10DevicePilot(G2dWindowFieldPilot<void*>(window, 0xfc), 0, NULL,
        G2dWindowFieldPilot<DWORD>(window, 0x10c) | 1, 0x1d, &device);
    if (FAILED(result)) return result == E_FAIL ? -1 : 3;
    int status = CreateG2dWindowDeviceResourcesSourcePilot(window, &windowResources);
    if (status) return status;
    status = CreateG2dDeviceSharedStateSourcePilot(device, &deviceState);
    if (status) return status;
    return CreateG2dWindowSharedStateSourcePilot(window, &windowState);
}
