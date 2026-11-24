#pragma once

static void ReleaseG2dWindowStateInterfacePilot(void* pointer)
{
    typedef ULONG (WINAPI *ReleaseMethod)(void*);
    if (pointer) G2dSlotPilot<ReleaseMethod>(pointer, 8)(pointer);
}

// 2067f3f0. Clear flagged slots before releasing them.
static void ResetG2dWindowStateEntryPilot(void* entry)
{
    BYTE& initialized = G2dWindowFieldPilot<BYTE>(entry, 4);
    if (!initialized) return;
    for (unsigned int i = 0; i < 4; ++i) {
        if (G2dWindowFieldPilot<BYTE>(entry, 0) & (1u << i)) {
            void*& slot = G2dWindowFieldPilot<void*>(entry, 0x288 + i * sizeof(void*));
            void* pointer = slot;
            slot = NULL;
            ReleaseG2dWindowStateInterfacePilot(pointer);
        }
    }
    initialized = 0;
}

// 2067f260. Release remaining interfaces in reverse order.
static void DestroyG2dWindowStateEntryPilot(void* entry)
{
    if (G2dWindowFieldPilot<BYTE>(entry, 5)) {
        G2dWindowFieldPilot<void*>(entry, 8) = NULL;
        G2dWindowFieldPilot<BYTE>(entry, 5) = 0;
    }
    for (unsigned int offset = 0x294; offset >= 0x288; offset -= sizeof(void*))
        ReleaseG2dWindowStateInterfacePilot(G2dWindowFieldPilot<void*>(entry, offset));
}

// 2067e9c0. The eight entries are owned. The catalogue shaders are borrowed.
static void ResetG2dWindowSharedStatePilot(void* holder)
{
    for (unsigned int i = 0; i < 8; ++i) {
        void*& slot = G2dWindowFieldPilot<void*>(holder, 0x84 + i * sizeof(void*));
        void* entry = slot;
        if (!entry) continue;
        slot = NULL;
        ResetG2dWindowStateEntryPilot(entry);
        G2dWindowFieldPilot<void*>(entry, 8) = NULL;
        G2dWindowFieldPilot<BYTE>(entry, 5) = 0;
        DestroyG2dWindowStateEntryPilot(entry);
        FreeG2dHeapSlotBoundaryPilot(&entry);
    }

    void*& quadBuffer = G2dWindowFieldPilot<void*>(holder, 0x58);
    void* buffer = quadBuffer;
    quadBuffer = NULL;
    ReleaseG2dWindowStateInterfacePilot(buffer);
    G2dWindowFieldPilot<BYTE>(holder, 0x48) = 0;
    G2dWindowFieldPilot<void*>(holder, 0x10) = NULL;
    G2dWindowFieldPilot<void*>(holder, 0x14) = NULL;
    G2dWindowFieldPilot<void*>(holder, 0x18) = NULL;
    G2dWindowFieldPilot<void*>(holder, 8) = NULL;
    G2dWindowFieldPilot<BYTE>(holder, 0x0c) = 0;
}
