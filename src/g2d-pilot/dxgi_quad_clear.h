#pragma once

extern "C" int WINAPI BeginG2dPerfEventBoundaryPilot(DWORD, const WCHAR*); // d3d9.dll, IAT 20661498
extern "C" int WINAPI EndG2dPerfEventBoundaryPilot(); // d3d9.dll, IAT 206614a4

typedef void (WINAPI *G2dClearDepthStencilMethodPilot)(void*, void*, UINT, FLOAT, UINT8);
typedef void (WINAPI *G2dClearColorMethodPilot)(void*, void*, const FLOAT*);

static void* GetG2dTaggedClearViewPilot(void* wrapper, DWORD expectedTag)
{
    if (G2dWindowFieldPilot<DWORD>(wrapper, 0x20) != expectedTag) return NULL;
    return G2dWindowFieldPilot<void*>(wrapper, 0x24);
}

// 2067a270. The compositor supplies its receiver in EAX.
static int ClearG2dCompositorTargetPilot(void* compositor, DWORD flags, const FLOAT* color,
                                        FLOAT depth, DWORD stencil)
{
    if (!G2dWindowFieldPilot<BYTE>(compositor, 8) ||
        !G2dWindowFieldPilot<BYTE>(compositor, 0x0a)) return -1;

    BeginG2dPerfEventBoundaryPilot(0xffff0000u, L"Compositor::Clear");
    void* window = G2dWindowFieldPilot<void*>(compositor, 0x34);
    void* device = G2dWindowFieldPilot<void*>(window, 0x104);
    if (flags & 1) {
        void* view = GetG2dTaggedClearViewPilot(G2dWindowFieldPilot<void*>(compositor, 0x60), 1);
        G2dSlotPilot<G2dClearColorMethodPilot>(device, 0x8c)(device, view, color);
    }
    if (flags & 6) {
        void* view = GetG2dTaggedClearViewPilot(G2dWindowFieldPilot<void*>(compositor, 0x64), 2);
        G2dSlotPilot<G2dClearDepthStencilMethodPilot>(device, 0x90)(
            device, view, (flags >> 1) & 3, depth, static_cast<UINT8>(stencil));
    }
    EndG2dPerfEventBoundaryPilot();
    return 0;
}

// Width and height are the first two fields of the x86 DXGI_SWAP_CHAIN_DESC.
struct G2dClearSwapChainDescPilot {
    UINT width;
    UINT height;
    DWORD remaining[13];
};
C_ASSERT(sizeof(G2dClearSwapChainDescPilot) == 0x3c);

// 206db390. Choose between an alternate target, device clear and scissored draw.
int ClearG2dDxgiTargetSourcePilot(void* target, DWORD flags, const FLOAT* color,
                                  FLOAT depth, DWORD stencil)
{
    typedef void (WINAPI *GetScissorsMethod)(void*, UINT*, RECT*);
    typedef HRESULT (WINAPI *GetDescMethod)(void*, G2dClearSwapChainDescPilot*);

    const bool checkScissor = G2dWindowFieldPilot<BYTE>(target, 0) != 0;
    if (checkScissor && G2dWindowFieldPilot<BYTE>(target, 1) &&
        G2dWindowFieldPilot<BYTE>(target, 0x24))
        return ClearG2dCompositorTargetPilot(static_cast<BYTE*>(target) + 0x1c,
                                            flags, color, depth, stencil);

    void* window = G2dWindowFieldPilot<void*>(target, 0x0c);
    void* device = G2dWindowFieldPilot<void*>(window, 0x104);
    if (checkScissor) {
        UINT count = 1;
        RECT rect = {0};
        G2dSlotPilot<GetScissorsMethod>(device, 0xf8)(device, &count, &rect);
        void* swapChain = G2dWindowFieldPilot<void*>(target, 0x10);
        G2dClearSwapChainDescPilot desc;
        const HRESULT result = G2dSlotPilot<GetDescMethod>(swapChain, 0x30)(swapChain, &desc);
        if (FAILED(result) || !count ||
            (desc.width == static_cast<UINT>(rect.right - rect.left) &&
             desc.height == static_cast<UINT>(rect.bottom - rect.top)))
            flags |= 8;
    } else {
        flags |= 8;
    }

    if (!(flags & 8))
        return DrawG2dQuadSourcePilot(G2dWindowFieldPilot<void*>(window, 0x160),
                                     stencil, flags & 7, color, depth);

    void* depthState = G2dWindowFieldPilot<void*>(target, 0x18);
    if ((flags & 6) && depthState) {
        void* view = GetG2dTaggedClearViewPilot(depthState, 2);
        G2dSlotPilot<G2dClearDepthStencilMethodPilot>(device, 0x90)(
            device, view, (flags >> 1) & 3, depth, static_cast<UINT8>(stencil));
    }
    void* renderTargetView = G2dWindowFieldPilot<void*>(target, 0x14);
    if ((flags & 1) && renderTargetView)
        G2dSlotPilot<G2dClearColorMethodPilot>(device, 0x8c)(device, renderTargetView, color);
    return 0;
}
