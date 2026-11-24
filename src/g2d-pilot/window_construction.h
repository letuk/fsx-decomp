#pragma once

#include <new>
#include <stdexcept>
#include <string>
#include <string.h>

extern int g_g2dHeapZeroInitializePilot; // 20704eb8
extern void* g_g2dBaseWindowTablePilot[]; // 20669928
extern void* g_g2dLegacyWindowTablePilot[]; // 20668890
extern void* g_g2dLegacyWindowSecondaryTablePilot[]; // 20668968
extern void* g_g2dDxgiWindowTablePilot[]; // 20668c20
extern DWORD g_g2dLegacyWindowDefaultsPilot; // 20701c90
extern DWORD g_g2dDxgiWindowDefaultsPilot; // 20701c94
extern "C" int __stdcall AllocateG2dHeapBoundaryPilot(int, SIZE_T, void**, int); // api.dll #106
extern "C" int __stdcall FreeG2dHeapSlotBoundaryPilot(void**); // api.dll #108

C_ASSERT(sizeof(std::string) == 0x1c);
C_ASSERT(offsetof(G2dControllerObservedPilot, windowsEnd) == 0x0c);
C_ASSERT(offsetof(G2dControllerObservedPilot, windowsCapacityEnd) == 0x10);

template<class Field>
static Field& G2dWindowFieldPilot(void* window, size_t offset)
{
    return *reinterpret_cast<Field*>(static_cast<BYTE*>(window) + offset);
}

static void* AllocateG2dWindowStoragePilot(size_t bytes)
{
    void* storage = NULL;
    AllocateG2dHeapBoundaryPilot(1, bytes, &storage, g_g2dHeapZeroInitializePilot);
    return storage;
}

static void ZeroG2dWindowDwordsPilot(void* window, const unsigned int* offsets, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        G2dWindowFieldPilot<DWORD>(window, offsets[i]) = 0;
}

// 20696ba0. Two VC8 strings occupy +0x54 and +0x70.
static void ConstructG2dBaseWindowPilot(void* window, UINT windowIndex, UINT adapterIndex, UINT outputIndex)
{
    static const unsigned int zeroFields[] = {
        0x08, 0x0c, 0x10, 0x18, 0x1c, 0x20, 0x28, 0x2c, 0x30, 0x34,
        0x98, 0x9c, 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc,
        0xc0, 0xc4, 0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4, 0xec, 0xf0, 0xf4, 0xf8
    };
    G2dWindowFieldPilot<void**>(window, 0) = g_g2dBaseWindowTablePilot;
    ZeroG2dWindowDwordsPilot(window, zeroFields, _countof(zeroFields));
    std::string* first = new (static_cast<BYTE*>(window) + 0x54) std::string;
    try {
        new (static_cast<BYTE*>(window) + 0x70) std::string;
    } catch (...) {
        first->~basic_string();
        throw;
    }
    G2dWindowFieldPilot<UINT>(window, 0x8c) = windowIndex;
    G2dWindowFieldPilot<UINT>(window, 0x90) = adapterIndex;
    G2dWindowFieldPilot<UINT>(window, 0x94) = outputIndex;
    memset(static_cast<BYTE*>(window) + 0xc8, 0, 6);
    G2dWindowFieldPilot<BYTE>(window, 0xcb) = 1;
    G2dWindowFieldPilot<DWORD>(window, 0xe8) = 4;
}

static void InitializeG2dWindowDefaultsPilot(void* window, DWORD* defaults)
{
    memset(static_cast<BYTE*>(window) + 0x34, 0, 0x20);
    G2dWindowFieldPilot<DWORD*>(window, 0x34) = defaults;
}

// Allocation at 20689c47, construction at 206857d0.
void* CreateG2dLegacyWindowSourcePilot(UINT adapterIndex)
{
    void* window = AllocateG2dWindowStoragePilot(0x810);
    if (!window) return NULL;
    try {
        ConstructG2dBaseWindowPilot(window, adapterIndex, adapterIndex, 0);
    } catch (...) {
        FreeG2dHeapSlotBoundaryPilot(&window);
        throw;
    }
    static const unsigned int zeroFields[] = {
        0x100, 0x104, 0x108, 0x10c, 0x110, 0x118, 0x158, 0x15c, 0x290, 0x294, 0x298,
        0x6ec, 0x6f0, 0x6f4, 0x6f8, 0x7fc, 0x800, 0x804, 0x808, 0x80c
    };
    G2dWindowFieldPilot<void**>(window, 0) = g_g2dLegacyWindowTablePilot;
    G2dWindowFieldPilot<void**>(window, 0xfc) = g_g2dLegacyWindowSecondaryTablePilot;
    ZeroG2dWindowDwordsPilot(window, zeroFields, _countof(zeroFields));
    G2dWindowFieldPilot<DWORD>(window, 0x114) = 1;
    G2dWindowFieldPilot<BYTE>(window, 0x155) = 0;
    G2dWindowFieldPilot<BYTE>(window, 0x6fc) = 0;
    memset(static_cast<BYTE*>(window) + 0x11c, 0, 0x38);
    memset(static_cast<BYTE*>(window) + 0x160, 0, 0x130);
    memset(static_cast<BYTE*>(window) + 0x29c, 0, 0x450);
    InitializeG2dWindowDefaultsPilot(window, &g_g2dLegacyWindowDefaultsPilot);
    return window;
}

// 2068f710 and 2068f8f0. Both store the output and retain the adapter.
void* CreateG2dDxgiWindowSourcePilot(UINT adapterIndex, UINT windowIndex,
    UINT outputIndex, void* adapter, void* output, void* firstOutputWindow)
{
    void* window = AllocateG2dWindowStoragePilot(0x1b0);
    if (!window) return NULL;
    void* treeHead = NULL;
    try {
        treeHead = AllocateG2dWindowStoragePilot(0x18); // 206a8540
        if (!treeHead) {
            FreeG2dHeapSlotBoundaryPilot(&window);
            return NULL;
        }
        ConstructG2dBaseWindowPilot(window, windowIndex, adapterIndex, outputIndex);
    } catch (...) {
        if (treeHead) FreeG2dHeapSlotBoundaryPilot(&treeHead);
        FreeG2dHeapSlotBoundaryPilot(&window);
        throw;
    }
    static const unsigned int zeroFields[] = {
        0x104, 0x10c, 0x158, 0x15c, 0x160, 0x164, 0x168, 0x16c,
        0x17c, 0x180, 0x184, 0x18c, 0x190, 0x194, 0x19c, 0x1a0, 0x1a4
    };
    G2dWindowFieldPilot<void**>(window, 0) = g_g2dDxgiWindowTablePilot;
    G2dWindowFieldPilot<void*>(window, 0xfc) = adapter;
    if (adapter) {
        typedef ULONG (WINAPI *AddRefPilot)(void*);
        G2dSlotPilot<AddRefPilot>(adapter, 4)(adapter);
    }
    G2dWindowFieldPilot<void*>(window, 0x100) = output;
    G2dWindowFieldPilot<void*>(window, 0x108) = outputIndex == 0 ? NULL : firstOutputWindow;
    G2dWindowFieldPilot<void*>(window, 0x110) = window;
    ZeroG2dWindowDwordsPilot(window, zeroFields, _countof(zeroFields));
    memset(static_cast<BYTE*>(window) + 0x114, 0, 0x3c);
    memset(static_cast<BYTE*>(window) + 0x170, 0, 5);
    G2dWindowFieldPilot<void*>(window, 0x154) = treeHead;
    G2dWindowFieldPilot<void*>(treeHead, 0) = treeHead;
    G2dWindowFieldPilot<void*>(treeHead, 4) = treeHead;
    G2dWindowFieldPilot<void*>(treeHead, 8) = treeHead;
    G2dWindowFieldPilot<BYTE>(treeHead, 0x14) = 1;
    G2dWindowFieldPilot<BYTE>(treeHead, 0x15) = 1;
    if (outputIndex == 0) {
        G2dWindowFieldPilot<DWORD>(window, 0x1a8) = 1;
        G2dWindowFieldPilot<DWORD>(window, 0x1ac) = 0;
    }
    InitializeG2dWindowDefaultsPilot(window, &g_g2dDxgiWindowDefaultsPilot);
    return window;
}

// End insertion through 20689d80 or 20689df0. Publish only after the copy.
void AppendG2dBackendWindowSourcePilot(G2dControllerObservedPilot* controller, void* window)
{
    if (controller->windowsEnd != controller->windowsCapacityEnd) {
        *controller->windowsEnd++ = window;
        return;
    }
    const size_t maxCount = 0x3fffffff;
    const size_t count = controller->windowsBegin ? controller->windowsEnd - controller->windowsBegin : 0;
    if (count >= maxCount) throw std::length_error("G2D window list");
    const size_t grown = count <= maxCount - count / 2 ? count + count / 2 : 0;
    const size_t capacity = grown > count ? grown : count + 1;
    void* replacement = AllocateG2dWindowStoragePilot(capacity * sizeof(void*));
    if (!replacement) throw std::bad_alloc();
    void** entries = static_cast<void**>(replacement);
    if (count) memcpy(entries, controller->windowsBegin, count * sizeof(void*));
    entries[count] = window;
    if (controller->windowsBegin) {
        void* previous = controller->windowsBegin;
        FreeG2dHeapSlotBoundaryPilot(&previous);
    }
    controller->windowsBegin = entries;
    controller->windowsEnd = entries + count + 1;
    controller->windowsCapacityEnd = entries + capacity;
}
