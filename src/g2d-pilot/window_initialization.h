#pragma once

// Register-based original helpers use explicit source interfaces here.
extern "C" void __cdecl SetG2dD3dPerfOptionsBoundaryPilot(DWORD); // 206ee9dc
extern "C" void __cdecl ReadG2dModeTextDimensionsBoundaryPilot(const char*, DWORD*, DWORD*, DWORD*); // 20699be0
extern "C" DWORD __cdecl ReadG2dLegacyAdapterKindBoundaryPilot(UINT); // 206edd10
extern "C" DWORD __cdecl ReadG2dLegacyAdapterMemoryBoundaryPilot(UINT); // 206edd90
extern "C" void __cdecl ReadG2dLegacyDriverVersionBoundaryPilot(void*); // 20685cf7–20685da3
extern "C" void __cdecl EnumerateG2dLegacyModesBoundaryPilot(void*); // 20688ef0
extern "C" void __cdecl ReadG2dLegacyMemoryOverrideBoundaryPilot(void*, DWORD*); // 20689080
extern "C" void __cdecl NotifyG2dLowGraphicsMemoryBoundaryPilot(); // api.dll #228
extern "C" void __stdcall ReportG2dUnsupportedAdapterBoundaryPilot(const char*, DWORD, DWORD); // 20699890
extern int g_g2dAllowShaderModel3Pilot; // 20704dbc
extern int g_g2dAllowShaderModel2Pilot; // 207018fc

extern "C" void __cdecl RecordG2dDxgiAdapterLabelBoundaryPilot(void*, const WCHAR*); // 2068fd4f–2068fe50
extern "C" int __stdcall EnumerateG2dDxgiModesBoundaryPilot(void*); // 206908d0
extern "C" DWORD __cdecl CountG2dWindowModesBoundaryPilot(void*); // 206856a0
extern "C" void __cdecl SelectG2dDxgiModeBoundaryPilot(void*, DWORD, DWORD*, DWORD*); // 2068fec4–206901a2
extern "C" void __stdcall ApplyG2dWindowModeBoundaryPilot(void*, DWORD, DWORD); // 206978f0
extern "C" void __cdecl FinishG2dDxgiWindowModeBoundaryPilot(void*); // 20690220
extern "C" void __cdecl RefreshG2dDxgiDeviceStateBoundaryPilot(void*); // 20691030
extern "C" void __cdecl PublishG2dDxgiWindowStateBoundaryPilot(void*); // 20691d30

#include "dxgi_window_device.h"

static UINT ReadG2dWindowAdapterIndexPilot(void* window)
{
    return G2dSlotPilot<UINT (__thiscall *)(void*)>(window, 4)(window);
}

static int FailG2dWindowInitializationPilot(void* window)
{
    G2dSlotPilot<G2dWordMethodPilot>(window, 0x8c)(window, 1);
    G2dWindowFieldPilot<DWORD>(window, 0x98) = 0;
    return -1;
}

static void ReadG2dLegacyAdapterMemoryPilot(void* window)
{
    G2dWindowFieldPilot<DWORD>(window, 0x290) =
        ReadG2dLegacyAdapterKindBoundaryPilot(ReadG2dWindowAdapterIndexPilot(window));
    G2dWindowFieldPilot<DWORD>(window, 0x294) =
        ReadG2dLegacyAdapterMemoryBoundaryPilot(ReadG2dWindowAdapterIndexPilot(window));
}

// 20685a60. Window allocation and publication are handled by the controller.
int InitializeG2dLegacyWindowSourcePilot(void* window)
{
    G2dSlotPilot<G2dWordMethodPilot>(window, 0x8c)(window, 0);
    void* controllerInterface = reinterpret_cast<BYTE*>(g_g2dControllerPilot) + 0x20;
    typedef void* (__thiscall *ReadD3dPilot)(void*);
    void* d3d = G2dSlotPilot<ReadD3dPilot>(controllerInterface, 0)(controllerInterface);
    SetG2dD3dPerfOptionsBoundaryPilot(1);

    typedef HRESULT (WINAPI *ReadDisplayModePilot)(void*, UINT, void*);
    G2dSlotPilot<ReadDisplayModePilot>(d3d, 0x20)(d3d, ReadG2dWindowAdapterIndexPilot(window),
        &G2dWindowFieldPilot<DWORD>(window, 0x38));
    std::string& modeText = G2dWindowFieldPilot<std::string>(window, 0x70);
    modeText = "1024x720x32";
    ReadG2dModeTextDimensionsBoundaryPilot(modeText.c_str(),
        &G2dWindowFieldPilot<DWORD>(window, 0xdc), &G2dWindowFieldPilot<DWORD>(window, 0xe0),
        &G2dWindowFieldPilot<DWORD>(window, 0xe4));
    typedef HMONITOR (WINAPI *ReadMonitorPilot)(void*, UINT);
    G2dWindowFieldPilot<HMONITOR>(window, 0x98) =
        G2dSlotPilot<ReadMonitorPilot>(d3d, 0x3c)(d3d, ReadG2dWindowAdapterIndexPilot(window));

    ReadG2dLegacyAdapterMemoryPilot(window);
    for (unsigned int attempt = 0; attempt < 25 && !G2dWindowFieldPilot<DWORD>(window, 0x294); ++attempt) {
        int previous = SetApiGraphicsPreparationBoundaryPilot(1);
        PrepareG2dDesktopBoundaryPilot();
        BeginG2dLegacyPreparationBoundaryPilot();
        SetApiGraphicsPreparationBoundaryPilot(previous);
        ReadG2dLegacyAdapterMemoryPilot(window);
    }
    if (!G2dWindowFieldPilot<DWORD>(window, 0x294)) return FailG2dWindowInitializationPilot(window);

    typedef HRESULT (WINAPI *ReadAdapterDetailsPilot)(void*, UINT, DWORD, void*);
    G2dWindowFieldPilot<DWORD>(window, 0x298) = 8;
    if (FAILED(G2dSlotPilot<ReadAdapterDetailsPilot>(d3d, 0x14)(d3d,
        ReadG2dWindowAdapterIndexPilot(window), 0, &G2dWindowFieldPilot<DWORD>(window, 0x29c))))
        return FailG2dWindowInitializationPilot(window);

    char adapterLabel[1024];
    sprintf_s(adapterLabel, sizeof(adapterLabel), "%s.%u",
        &G2dWindowFieldPilot<char>(window, 0x49c), ReadG2dWindowAdapterIndexPilot(window));
    G2dWindowFieldPilot<std::string>(window, 0x54) = adapterLabel;

    ReadAdapterDetailsPilot readCaps = G2dSlotPilot<ReadAdapterDetailsPilot>(d3d, 0x38);
    if (FAILED(readCaps(d3d, ReadG2dWindowAdapterIndexPilot(window), 1,
        &G2dWindowFieldPilot<DWORD>(window, 0x160)))) return FailG2dWindowInitializationPilot(window);
    if (G2dWindowFieldPilot<DWORD>(window, 0x248) != 1) {
        DWORD caps[0x130 / sizeof(DWORD)] = {0};
        readCaps(d3d, G2dWindowFieldPilot<DWORD>(window, 0x240), 1, caps);
        if (caps[0xe8 / sizeof(DWORD)]) G2dWindowFieldPilot<DWORD>(window, 0x294) /= caps[0xe8 / sizeof(DWORD)];
    }
    if (!(G2dWindowFieldPilot<DWORD>(window, 0x6bc) | G2dWindowFieldPilot<DWORD>(window, 0x6c0)))
        ReadG2dLegacyDriverVersionBoundaryPilot(window);
    EnumerateG2dLegacyModesBoundaryPilot(window);
    BYTE* modes = G2dWindowFieldPilot<BYTE*>(window, 8);
    if (!modes || G2dWindowFieldPilot<BYTE*>(window, 0x0c) - modes < 0x20)
        return FailG2dWindowInitializationPilot(window);

    DWORD memoryOverride = 0;
    ReadG2dLegacyMemoryOverrideBoundaryPilot(window, &memoryOverride);
    DWORD caps = G2dWindowFieldPilot<DWORD>(window, 0x158);
    DWORD kind = G2dWindowFieldPilot<DWORD>(window, 0x290);
    if ((caps & 1) || (kind && kind < 7)) {
        G2dWindowFieldPilot<BYTE>(window, 0x155) = 0;
        typedef const char* (__thiscall *ReadAdapterLabelPilot)(void*);
        const char* label = G2dSlotPilot<ReadAdapterLabelPilot>(window, 0x1c)(window);
        ReportG2dUnsupportedAdapterBoundaryPilot(label, G2dWindowFieldPilot<DWORD>(window, 0x6c4),
            G2dWindowFieldPilot<DWORD>(window, 0x6c8));
        return FailG2dWindowInitializationPilot(window);
    }

    DWORD vertex = G2dWindowFieldPilot<DWORD>(window, 0x224);
    DWORD pixel = G2dWindowFieldPilot<DWORD>(window, 0x22c);
    DWORD& shaderLevel = G2dWindowFieldPilot<DWORD>(window, 0x80c);
    if (vertex >= 0xfffe0300u && pixel >= 0xffff0300u && (caps & 0x2000000) && g_g2dAllowShaderModel3Pilot)
        shaderLevel = 0x27;
    else if (vertex >= 0xfffe0200u && pixel >= 0xffff0200u && (caps & 0x2000000) && g_g2dAllowShaderModel2Pilot)
        shaderLevel = 0x1d;
    else if (vertex >= 0xfffe0101u && pixel >= 0xffff0101u)
        shaderLevel = 0x13;
    else {
        shaderLevel = 9;
        return FailG2dWindowInitializationPilot(window);
    }
    DWORD& memory = G2dWindowFieldPilot<DWORD>(window, 0x294);
    if (memoryOverride) memory = memoryOverride;
    if (!ReadG2dWindowAdapterIndexPilot(window) && memory < 0xf00000)
        NotifyG2dLowGraphicsMemoryBoundaryPilot();
    G2dWindowFieldPilot<BYTE>(window, 0xc9) = (caps & 0x800000) != 0 && memory >= 0x1800000 &&
        G2dWindowFieldPilot<DWORD>(window, 0x1bc) >= 0x800 &&
        G2dWindowFieldPilot<DWORD>(window, 0x1b8) >= 0x800;
    return 0;
}

// 2068fc80. Nonzero prerequisite statuses propagate to the caller.
int InitializeG2dDxgiWindowSourcePilot(void* window)
{
    G2dSlotPilot<G2dWordMethodPilot>(window, 0x8c)(window, 0);
    int status = PrepareG2dDxgiWindowSourcePilot(window, NULL);
    if (status) return status;
    status = ConfigureG2dDxgiDeviceSourcePilot(window);
    if (status) return status;
    status = ConfigureG2dDxgiOutputSourcePilot(window, &G2dWindowFieldPilot<DWORD>(window, 0x34));
    if (status) return status;

    typedef HRESULT (WINAPI *ReadDescriptorPilot)(void*, void*);
    DWORD adapter[0x128 / sizeof(DWORD)];
    DWORD output[0x60 / sizeof(DWORD)];
    void* adapterInterface = G2dWindowFieldPilot<void*>(window, 0xfc);
    void* outputInterface = G2dWindowFieldPilot<void*>(window, 0x100);
    if (FAILED(G2dSlotPilot<ReadDescriptorPilot>(adapterInterface, 0x20)(adapterInterface, adapter)) ||
        FAILED(G2dSlotPilot<ReadDescriptorPilot>(outputInterface, 0x1c)(outputInterface, output))) return -1;
    G2dWindowFieldPilot<DWORD>(window, 0x98) = output[0x58 / sizeof(DWORD)];
    RecordG2dDxgiAdapterLabelBoundaryPilot(window, reinterpret_cast<const WCHAR*>(adapter));
    status = EnumerateG2dDxgiModesBoundaryPilot(window);
    if (status) return status;
    DWORD count = CountG2dWindowModesBoundaryPilot(window);
    if (!count) return FailG2dWindowInitializationPilot(window);

    G2dWindowFieldPilot<BYTE>(window, 0xc9) = 1;
    DWORD modeFlags = 0;
    DWORD modeValue = 0;
    SelectG2dDxgiModeBoundaryPilot(window, count, &modeFlags, &modeValue);
    ApplyG2dWindowModeBoundaryPilot(window, modeFlags & ~1u, modeValue);
    FinishG2dDxgiWindowModeBoundaryPilot(window);
    RefreshG2dDxgiDeviceStateBoundaryPilot(window);
    PublishG2dDxgiWindowStateBoundaryPilot(window);
    return 0;
}
