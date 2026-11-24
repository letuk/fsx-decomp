#pragma once

struct G2dClearViewportPilot {
    INT topLeftX;
    INT topLeftY;
    UINT width;
    UINT height;
    FLOAT minDepth;
    FLOAT maxDepth;
};
C_ASSERT(sizeof(G2dClearViewportPilot) == 0x18);

static void UnpackG2dClearRgbPilot(DWORD packed, FLOAT* color)
{
    color[0] = static_cast<FLOAT>(((packed >> 16) & 0xff) / 255.0);
    color[1] = static_cast<FLOAT>(((packed >> 8) & 0xff) / 255.0);
    color[2] = static_cast<FLOAT>((packed & 0xff) / 255.0);
}

// 20690570. EDX is unused; the rectangle and packed color arrive on the stack.
bool __fastcall SetG2dWindowViewportAndClearSourcePilot(void* window, void*, const RECT* rectangle, DWORD packed)
{
    typedef void (WINAPI *SetViewportsMethod)(void*, UINT, const G2dClearViewportPilot*);
    typedef void (WINAPI *SetScissorsMethod)(void*, UINT, const RECT*);
    void* device = G2dWindowFieldPilot<void*>(window, 0x104);
    if (!device) return true;

    G2dClearViewportPilot viewport = {0, 0, 0, 0, 0.0f, 1.0f};
    viewport.width = G2dSlotPilot<G2dStatusMethodPilot>(window, 0x38)(window);
    viewport.height = G2dSlotPilot<G2dStatusMethodPilot>(window, 0x20)(window);
    G2dSlotPilot<SetViewportsMethod>(device, 0x78)(device, 1, &viewport);
    RECT full = {0, 0, static_cast<LONG>(viewport.width), static_cast<LONG>(viewport.height)};
    G2dSlotPilot<SetScissorsMethod>(device, 0x7c)(device, 1, rectangle ? rectangle : &full);

    void* target = G2dWindowFieldPilot<void*>(window, 0x15c);
    if (!target) return true;
    FLOAT color[4];
    UnpackG2dClearRgbPilot(packed, color);
    color[3] = 1.0f;
    return ClearG2dDxgiTargetSourcePilot(target, 1, color, 1.0f, 0) == 0;
}

// 206b9440. Both original stack arguments are unused.
int __fastcall ClearG2dRendererDefaultSourcePilot(void* renderer, void*, DWORD, DWORD)
{
    if (G2dSlotPilot<G2dStatusMethodPilot>(renderer, 0x194)(renderer)) {
        void* window = G2dWindowFieldPilot<void*>(renderer, 0x227b4);
        void* target = G2dWindowFieldPilot<void*>(window, 0x15c);
        const FLOAT color[] = {0.0f, 0.0f, 0.0f, 1.0f};
        ClearG2dDxgiTargetSourcePilot(target, 7, color, 1.0f, 0);
    }
    return 0;
}

// 206b94c0. The first original stack argument is unused.
int __fastcall ClearG2dRendererSourcePilot(void* renderer, void*, DWORD, DWORD flags,
                                          DWORD packed, FLOAT depth, DWORD stencil)
{
    void* window = G2dWindowFieldPilot<void*>(renderer, 0x227b4);
    void* target = G2dWindowFieldPilot<void*>(window, 0x15c);
    FLOAT color[4];
    UnpackG2dClearRgbPilot(packed, color);
    color[3] = static_cast<FLOAT>((packed >> 24) / 255.0);
    ClearG2dDxgiTargetSourcePilot(target, flags, color, depth, stencil);
    return 0;
}
