#include "cab_stream_records.h"
#include "heap_boundary.h"
#include <stdlib.h>

struct CabPoolSlotStatePilot;
struct CabPoolBlockTailPilot {
    LONG available;
    CabPoolSlotStatePilot* slots;
    BYTE* next;
};
struct CabPoolRegistryNodePilot {
    void* const* vtable;
    CabPoolRegistryNodePilot* next;
};
struct CabPoolOwnerObservedPilot {
    CabPoolRegistryNodePilot registry;
    BYTE* head;
    void* lock;
    DWORD blockCount;
    DWORD peakBlockCount;
};
C_ASSERT(sizeof(CabPoolBlockTailPilot) == 12);
C_ASSERT(offsetof(CabPoolOwnerObservedPilot, lock) == 0xc);
CabPoolOwnerObservedPilot* g_cabFilePoolOwnerPilot; // 2020e194
CabPoolOwnerObservedPilot* g_cabWrapperPoolOwnerPilot; // 2020e1a0
extern int g_alternateAllocationZeroInitializePilot;
CabPoolRegistryNodePilot* g_cabPoolRegistryHeadPilot; // 2020e2bc
CabPoolOwnerObservedPilot g_cabFilePoolStoragePilot; // 2020eaac
CabPoolOwnerObservedPilot g_cabWrapperPoolStoragePilot; // 2020ea90
static DWORD g_cabFilePoolGuardPilot; // 2020eac4
static DWORD g_cabWrapperPoolGuardPilot; // 2020eaa8
extern void* const g_cabFilePoolOwnerVTablePilot[]; // 200f2cb0
extern void* const g_cabWrapperPoolOwnerVTablePilot[]; // 200f2d90
extern void* const g_cabPoolRegistryVTablePilot[]; // 200f0528
extern "C" void* __cdecl CreateCabPoolLockPilot(); // 20147dff
extern "C" DWORD __stdcall DestroyCabPoolLockSourcePilot(void*); // 20147c77

// Explicit source arguments. The original acquire, slot and destroy callees
// receive the object in EDI, EDI and EAX respectively.
extern "C" DWORD __stdcall AcquireCabPoolLockForStreamPoolsPilot(void*, DWORD); // 20146e07, pool subset
extern "C" DWORD __stdcall ReleaseCabPoolLockPilot(void*, int); // 20147aa6
extern "C" void* __stdcall ReserveCabPoolSlotSourcePilot(CabPoolSlotStatePilot*); // 201464c3
extern "C" int __stdcall PublishCabPoolSlotSourcePilot(CabPoolSlotStatePilot*, void*); // 201462fc
extern "C" int __stdcall DestroyCabPoolSlotsSourcePilot(CabPoolSlotStatePilot*); // 201460ff
extern "C" CabPoolSlotStatePilot* __stdcall CreateCabPoolSlotsPilot(LONG, LONG); // 20145fe3
typedef int (__stdcall *CabPoolBlockInitializerPilot)(BYTE*);

static CabPoolBlockTailPilot* CabPoolTail(BYTE* block, DWORD payloadBytes)
{
    return reinterpret_cast<CabPoolBlockTailPilot*>(block + payloadBytes);
}

static int InitializeCabPoolBlock(BYTE* block, DWORD recordBytes, DWORD slotsPerBlock)
{
    CabPoolBlockTailPilot* tail = CabPoolTail(block, recordBytes * slotsPerBlock);
    if (tail->slots == NULL) {
        tail->slots = CreateCabPoolSlotsPilot(slotsPerBlock, slotsPerBlock);
        if (tail->slots == NULL) {
            DebugBreak();
            return 0;
        }
    } else {
        while (ReserveCabPoolSlotSourcePilot(tail->slots) != NULL) {
            // Drain published slots before repopulating this block.
        }
    }
    for (DWORD index = 0; index < slotsPerBlock; ++index) {
        if (PublishCabPoolSlotSourcePilot(tail->slots, block + index * recordBytes) == 0) return 0;
    }
    tail->available = slotsPerBlock;
    return 1;
}

extern "C" int __stdcall InitializeCabFilePoolBlockPilot(BYTE* block) // 20124d34
{
    return InitializeCabPoolBlock(block, sizeof(CabFileStreamObserved276Pilot), 0x400);
}

extern "C" int __stdcall InitializeCabWrapperPoolBlockPilot(BYTE* block) // 20127eae
{
    return InitializeCabPoolBlock(block, sizeof(CabWrappedStreamObserved48Pilot), 0x80);
}

static void ReleaseCabPoolMode(CabPoolOwnerObservedPilot* owner, DWORD mode)
{
    if (owner->lock != NULL) ReleaseCabPoolLockPilot(owner->lock, mode != 0);
}

static void* AllocateCabPoolRecord(CabPoolOwnerObservedPilot* owner, DWORD payloadBytes,
    LONG slotsPerBlock, CabPoolBlockInitializerPilot initializeBlock)
{
    DWORD mode = 0;
    BYTE* block;
    for (;;) {
        if (owner->lock == NULL || AcquireCabPoolLockForStreamPoolsPilot(owner->lock, mode) != 0)
            return NULL;
        for (block = owner->head; block != NULL; block = CabPoolTail(block, payloadBytes)->next) {
            CabPoolBlockTailPilot* tail = CabPoolTail(block, payloadBytes);
            if (tail->slots != NULL) {
                void* result = ReserveCabPoolSlotSourcePilot(tail->slots);
                if (result != NULL) {
                    InterlockedDecrement(&tail->available);
                    ReleaseCabPoolMode(owner, mode);
                    return result;
                }
            }
        }

        DWORD flags = g_alternateAllocationZeroInitializePilot ? HEAP_ZERO_MEMORY : 0;
        if (g_heapInitializedPilot == 0) InitializeHeapManagerPilot();
        SIZE_T blockBytes = payloadBytes + sizeof(CabPoolBlockTailPilot);
        block = static_cast<BYTE*>(HeapAlloc(g_privateHeapPilot, flags, blockBytes));
        if (block == NULL) {
            block = static_cast<BYTE*>(HeapAlloc(g_privateHeapPilot, flags, blockBytes));
            if (block == NULL) {
                HINSTANCE resources = g_errorResourceInstancePilot;
                if (resources == NULL) resources = GetModuleHandleA(NULL);
                LoadStringA(resources, 8, g_heapErrorTextPilot, 260);
                MessageBoxA(NULL, g_heapErrorTextPilot, NULL, MB_ICONERROR);
                TerminateProcess(GetCurrentProcess(), 1);
                ReleaseCabPoolMode(owner, mode);
                return NULL;
            }
        }

        CabPoolBlockTailPilot* tail = CabPoolTail(block, payloadBytes);
        tail->available = slotsPerBlock;
        tail->slots = NULL;
        tail->next = NULL;
        if (initializeBlock(block) == 0) {
            // Original exits through lock release with the new block unlinked.
            ReleaseCabPoolMode(owner, mode);
            return NULL;
        }
        if (mode != 0) break;
        mode = 1;
        if (owner->lock != NULL) {
            if (AcquireCabPoolLockForStreamPoolsPilot(owner->lock, 3) == 0) break;
            ReleaseCabPoolMode(owner, 0);
        }
        if (tail->slots != NULL) {
            DestroyCabPoolSlotsSourcePilot(tail->slots);
            tail->slots = NULL;
        }
        HeapFree(g_privateHeapPilot, 0, block);
    }

    CabPoolBlockTailPilot* tail = CabPoolTail(block, payloadBytes);
    tail->next = owner->head;
    ++owner->blockCount;
    owner->head = block;
    if (owner->peakBlockCount < owner->blockCount) owner->peakBlockCount = owner->blockCount;
    void* result = ReserveCabPoolSlotSourcePilot(tail->slots);
    if (result != NULL) --tail->available;
    ReleaseCabPoolMode(owner, mode);
    return result;
}

static void ReturnCabPoolRecord(CabPoolOwnerObservedPilot* owner, DWORD payloadBytes, void* record)
{
    if (owner->lock == NULL || AcquireCabPoolLockForStreamPoolsPilot(owner->lock, 0) != 0)
        return;
    ULONG_PTR address = reinterpret_cast<ULONG_PTR>(record);
    for (BYTE* block = owner->head; block != NULL; block = CabPoolTail(block, payloadBytes)->next) {
        ULONG_PTR start = reinterpret_cast<ULONG_PTR>(block);
        if (start <= address && address < start + payloadBytes) {
            CabPoolBlockTailPilot* tail = CabPoolTail(block, payloadBytes);
            if (tail->slots != NULL && PublishCabPoolSlotSourcePilot(tail->slots, record) != 0)
                InterlockedIncrement(&tail->available);
            break;
        }
    }
    ReleaseCabPoolMode(owner, 0);
}

extern "C" CabFileStreamObserved276Pilot* __cdecl AllocateCabFileStreamRecordPilot() // 20124903
{
    return static_cast<CabFileStreamObserved276Pilot*>(AllocateCabPoolRecord(
        g_cabFilePoolOwnerPilot, 0x45000, 0x400, InitializeCabFilePoolBlockPilot));
}
extern "C" CabWrappedStreamObserved48Pilot* __cdecl AllocateCabWrapperRecordPilot() // 20127a04
{
    return static_cast<CabWrappedStreamObserved48Pilot*>(AllocateCabPoolRecord(
        g_cabWrapperPoolOwnerPilot, 0x1800, 0x80, InitializeCabWrapperPoolBlockPilot));
}
extern "C" void __stdcall ReturnCabFileStreamRecordPilot(CabFileStreamObserved276Pilot* record) // 20124b11
{
    ReturnCabPoolRecord(g_cabFilePoolOwnerPilot, 0x45000, record);
}
extern "C" void __stdcall ReturnCabWrapperRecordPilot(CabWrappedStreamObserved48Pilot* record) // 20127c12
{
    ReturnCabPoolRecord(g_cabWrapperPoolOwnerPilot, 0x1800, record);
}

static void FreeCabPoolBlock(BYTE* block, DWORD payloadBytes)
{
    CabPoolBlockTailPilot* tail = CabPoolTail(block, payloadBytes);
    if (tail->slots != NULL) {
        DestroyCabPoolSlotsSourcePilot(tail->slots);
        tail->slots = NULL;
    }
    HeapFree(g_privateHeapPilot, 0, block);
}

static void ResetCabPool(CabPoolOwnerObservedPilot* owner, DWORD payloadBytes)
{
    typedef DWORD (__thiscall *BeforeReset)(CabPoolOwnerObservedPilot*);
    reinterpret_cast<BeforeReset>(owner->registry.vtable[3])(owner);
    if (owner->lock == NULL || AcquireCabPoolLockForStreamPoolsPilot(owner->lock, 1) != 0)
        return;
    BYTE* block = owner->head;
    owner->head = NULL;
    while (block != NULL) {
        BYTE* next = CabPoolTail(block, payloadBytes)->next;
        FreeCabPoolBlock(block, payloadBytes);
        block = next;
    }
    owner->blockCount = 0;
    ReleaseCabPoolMode(owner, 1);
}

static void TrimCabPool(CabPoolOwnerObservedPilot* owner, DWORD payloadBytes, LONG slotsPerBlock)
{
    if (owner->lock == NULL || AcquireCabPoolLockForStreamPoolsPilot(owner->lock, 1) != 0)
        return;
    BYTE* previous = NULL;
    BYTE* block = owner->head;
    while (block != NULL) {
        CabPoolBlockTailPilot* tail = CabPoolTail(block, payloadBytes);
        BYTE* next = tail->next;
        if (tail->available == slotsPerBlock) {
            if (block == owner->head) owner->head = next;
            if (previous != NULL) CabPoolTail(previous, payloadBytes)->next = next;
            FreeCabPoolBlock(block, payloadBytes);
            --owner->blockCount;
        } else previous = block;
        block = next;
    }
    ReleaseCabPoolMode(owner, 1);
}

extern "C" void __fastcall UnlinkCabPoolRegistryNodePilot(CabPoolRegistryNodePilot* node);

class CabPoolRegistryCleanupPilot {
    CabPoolRegistryNodePilot* node;
public:
    explicit CabPoolRegistryCleanupPilot(CabPoolRegistryNodePilot* value) : node(value) {}
    ~CabPoolRegistryCleanupPilot() { if (node != NULL) UnlinkCabPoolRegistryNodePilot(node); }
    void keepRegistered() { node = NULL; }
};

static CabPoolOwnerObservedPilot* InitializeCabPoolOwner(CabPoolOwnerObservedPilot* owner, void* const* vtable)
{
    owner->registry.next = g_cabPoolRegistryHeadPilot;
    g_cabPoolRegistryHeadPilot = &owner->registry;
    CabPoolRegistryCleanupPilot cleanup(&owner->registry);
    owner->registry.vtable = vtable;
    owner->head = NULL;
    owner->lock = NULL;
    owner->lock = CreateCabPoolLockPilot();
    if (owner->lock == NULL) DebugBreak();
    owner->blockCount = 0;
    owner->peakBlockCount = 0;
    cleanup.keepRegistered();
    return owner;
}

// Original 200f43d6, registry prefix in ECX. The return value is unused.
extern "C" void __fastcall UnlinkCabPoolRegistryNodePilot(CabPoolRegistryNodePilot* node)
{
    CabPoolRegistryNodePilot* head = g_cabPoolRegistryHeadPilot;
    node->vtable = g_cabPoolRegistryVTablePilot;
    CabPoolRegistryNodePilot* previous = NULL;
    for (CabPoolRegistryNodePilot* current = head; current != NULL; current = current->next) {
        if (current == node) {
            if (previous != NULL) previous->next = node->next;
            if (head == node) g_cabPoolRegistryHeadPilot = node->next;
            return;
        }
        previous = current;
    }
}

static void DestroyCabPoolOwner(CabPoolOwnerObservedPilot* owner, void* const* vtable, DWORD payloadBytes)
{
    owner->registry.vtable = vtable;
    CabPoolRegistryCleanupPilot cleanup(&owner->registry);
    ResetCabPool(owner, payloadBytes);
    if (owner->lock != NULL) {
        DestroyCabPoolLockSourcePilot(owner->lock);
        owner->lock = NULL;
    }
}

extern "C" CabPoolOwnerObservedPilot* __cdecl InitializeCabFilePoolOwnerPilot() // 20124ca3
{
    return InitializeCabPoolOwner(&g_cabFilePoolStoragePilot, g_cabFilePoolOwnerVTablePilot);
}
extern "C" CabPoolOwnerObservedPilot* __cdecl InitializeCabWrapperPoolOwnerPilot() // 20127e1d
{
    return InitializeCabPoolOwner(&g_cabWrapperPoolStoragePilot, g_cabWrapperPoolOwnerVTablePilot);
}
extern "C" void __stdcall DestroyCabFilePoolOwnerPilot(CabPoolOwnerObservedPilot* owner) // 2012475b
{
    DestroyCabPoolOwner(owner, g_cabFilePoolOwnerVTablePilot, 0x45000);
}
extern "C" void __stdcall DestroyCabWrapperPoolOwnerPilot(CabPoolOwnerObservedPilot* owner) // 2012785c
{
    DestroyCabPoolOwner(owner, g_cabWrapperPoolOwnerVTablePilot, 0x1800);
}
extern "C" void __fastcall ResetCabFilePoolPilot(CabPoolOwnerObservedPilot* owner) // 201247c3
{
    ResetCabPool(owner, 0x45000);
}
extern "C" void __fastcall ResetCabWrapperPoolPilot(CabPoolOwnerObservedPilot* owner) // 201278c4
{
    ResetCabPool(owner, 0x1800);
}
extern "C" void __fastcall TrimCabFilePoolPilot(CabPoolOwnerObservedPilot* owner) // 20124845
{
    TrimCabPool(owner, 0x45000, 0x400);
}
extern "C" void __fastcall TrimCabWrapperPoolPilot(CabPoolOwnerObservedPilot* owner) // 20127946
{
    TrimCabPool(owner, 0x1800, 0x80);
}

static DWORD CountCabPoolAvailable(CabPoolOwnerObservedPilot* owner, DWORD payloadBytes)
{
    if (owner->lock == NULL || AcquireCabPoolLockForStreamPoolsPilot(owner->lock, 0) != 0) return 0;
    DWORD available = 0;
    for (BYTE* block = owner->head; block != NULL; block = CabPoolTail(block, payloadBytes)->next)
        available += CabPoolTail(block, payloadBytes)->available;
    ReleaseCabPoolMode(owner, 0);
    return available;
}

extern "C" DWORD __fastcall CountCabPoolUsedPilot(CabPoolOwnerObservedPilot* owner) // 201128d7
{
    typedef DWORD (__thiscall *CountMethod)(CabPoolOwnerObservedPilot*);
    DWORD capacity = reinterpret_cast<CountMethod>(owner->registry.vtable[5])(owner);
    DWORD available = reinterpret_cast<CountMethod>(owner->registry.vtable[4])(owner);
    return capacity - available;
}
extern "C" DWORD __fastcall CountCabFilePoolAvailablePilot(CabPoolOwnerObservedPilot* owner) // 20124ba0
{
    return CountCabPoolAvailable(owner, 0x45000);
}
extern "C" DWORD __fastcall CountCabWrapperPoolAvailablePilot(CabPoolOwnerObservedPilot* owner) // 20127ca1
{
    return CountCabPoolAvailable(owner, 0x1800);
}
extern "C" DWORD __fastcall CountCabFilePoolCapacityPilot(CabPoolOwnerObservedPilot* owner) // 20124bf6
{
    return owner->blockCount << 10;
}
extern "C" DWORD __fastcall CountCabWrapperPoolCapacityPilot(CabPoolOwnerObservedPilot* owner) // 20127cf7
{
    return owner->blockCount << 7;
}
extern "C" DWORD __fastcall CountCabPoolBlocksPilot(CabPoolOwnerObservedPilot* owner) // 20112953
{
    return owner->blockCount;
}
extern "C" DWORD __fastcall CountCabPoolPeakBlocksPilot(CabPoolOwnerObservedPilot* owner) // 20124bfd
{
    return owner->peakBlockCount;
}
extern "C" DWORD __cdecl CountCabFilePoolSlotsPerBlockPilot() { return 0x400; } // 200f48b0
extern "C" DWORD __cdecl CountCabWrapperPoolSlotsPerBlockPilot() { return 0x80; } // 20127cfe
extern "C" DWORD __fastcall CountCabFilePoolBytesPilot(CabPoolOwnerObservedPilot* owner) // 20124c01
{
    return owner->blockCount * 0x4500c;
}
extern "C" DWORD __fastcall CountCabWrapperPoolBytesPilot(CabPoolOwnerObservedPilot* owner) // 20127d04
{
    return owner->blockCount * 0x180c;
}
extern "C" DWORD __cdecl CabFilePoolRecordBytesPilot() { return 0x114; } // 20124c0b
extern "C" DWORD __cdecl CabWrapperPoolRecordBytesPilot() { return 0x30; } // 20127d0e

// The unused EDX slot allows the original ECX receiver and stack flag layout.
extern "C" void* __fastcall DeleteCabFilePoolOwnerPilot(CabPoolOwnerObservedPilot* owner, void*, DWORD flags) // 20124c75
{
    DestroyCabFilePoolOwnerPilot(owner);
    if ((flags & 1) != 0 && owner != NULL) HeapFree(g_privateHeapPilot, 0, owner);
    return owner;
}
extern "C" void* __fastcall DeleteCabWrapperPoolOwnerPilot(CabPoolOwnerObservedPilot* owner, void*, DWORD flags) // 20127da5
{
    DestroyCabWrapperPoolOwnerPilot(owner);
    if ((flags & 1) != 0 && owner != NULL) HeapFree(g_privateHeapPilot, 0, owner);
    return owner;
}
extern "C" void* __fastcall DeleteCabPoolRegistryNodePilot(CabPoolRegistryNodePilot* node, void*, DWORD flags) // 200f492a
{
    UnlinkCabPoolRegistryNodePilot(node);
    if ((flags & 1) != 0 && node != NULL) HeapFree(g_privateHeapPilot, 0, node);
    return node;
}

extern "C" int __cdecl _purecall();
void* const g_cabPoolRegistryVTablePilot[] = {
    reinterpret_cast<void*>(DeleteCabPoolRegistryNodePilot),
    reinterpret_cast<void*>(_purecall), reinterpret_cast<void*>(_purecall),
    reinterpret_cast<void*>(_purecall), reinterpret_cast<void*>(_purecall),
    reinterpret_cast<void*>(_purecall), reinterpret_cast<void*>(_purecall),
    reinterpret_cast<void*>(_purecall), reinterpret_cast<void*>(_purecall),
    reinterpret_cast<void*>(_purecall), reinterpret_cast<void*>(_purecall)
};
void* const g_cabFilePoolOwnerVTablePilot[] = {
    reinterpret_cast<void*>(DeleteCabFilePoolOwnerPilot),
    reinterpret_cast<void*>(ResetCabFilePoolPilot),
    reinterpret_cast<void*>(TrimCabFilePoolPilot),
    reinterpret_cast<void*>(CountCabPoolUsedPilot),
    reinterpret_cast<void*>(CountCabFilePoolAvailablePilot),
    reinterpret_cast<void*>(CountCabFilePoolCapacityPilot),
    reinterpret_cast<void*>(CountCabPoolBlocksPilot),
    reinterpret_cast<void*>(CountCabPoolPeakBlocksPilot),
    reinterpret_cast<void*>(CountCabFilePoolSlotsPerBlockPilot),
    reinterpret_cast<void*>(CountCabFilePoolBytesPilot),
    reinterpret_cast<void*>(CabFilePoolRecordBytesPilot)
};
void* const g_cabWrapperPoolOwnerVTablePilot[] = {
    reinterpret_cast<void*>(DeleteCabWrapperPoolOwnerPilot),
    reinterpret_cast<void*>(ResetCabWrapperPoolPilot),
    reinterpret_cast<void*>(TrimCabWrapperPoolPilot),
    reinterpret_cast<void*>(CountCabPoolUsedPilot),
    reinterpret_cast<void*>(CountCabWrapperPoolAvailablePilot),
    reinterpret_cast<void*>(CountCabWrapperPoolCapacityPilot),
    reinterpret_cast<void*>(CountCabPoolBlocksPilot),
    reinterpret_cast<void*>(CountCabPoolPeakBlocksPilot),
    reinterpret_cast<void*>(CountCabWrapperPoolSlotsPerBlockPilot),
    reinterpret_cast<void*>(CountCabWrapperPoolBytesPilot),
    reinterpret_cast<void*>(CabWrapperPoolRecordBytesPilot)
};

static void __cdecl CleanUpCabFilePoolPilot() // 201eeee2
{
    DestroyCabFilePoolOwnerPilot(&g_cabFilePoolStoragePilot);
}
static void __cdecl CleanUpCabWrapperPoolPilot() // 201eeef7
{
    DestroyCabWrapperPoolOwnerPilot(&g_cabWrapperPoolStoragePilot);
}

// Original unwind actions 201ed414 and 201ed460 clear only the guard bit.
class CabPoolGuardRollbackPilot {
    DWORD* guard;
public:
    explicit CabPoolGuardRollbackPilot(DWORD* value) : guard(value) {}
    ~CabPoolGuardRollbackPilot() { if (guard != NULL) *guard &= ~1u; }
    void complete() { guard = NULL; }
};

extern "C" CabPoolOwnerObservedPilot* __cdecl GetCabFilePoolOwnerPilot() // 20124c11
{
    if ((g_cabFilePoolGuardPilot & 1) == 0) {
        g_cabFilePoolGuardPilot |= 1;
        CabPoolGuardRollbackPilot rollback(&g_cabFilePoolGuardPilot);
        InitializeCabFilePoolOwnerPilot();
        atexit(CleanUpCabFilePoolPilot);
        rollback.complete();
    }
    return &g_cabFilePoolStoragePilot;
}
extern "C" CabPoolOwnerObservedPilot* __cdecl GetCabWrapperPoolOwnerPilot() // 20127d14
{
    if ((g_cabWrapperPoolGuardPilot & 1) == 0) {
        g_cabWrapperPoolGuardPilot |= 1;
        CabPoolGuardRollbackPilot rollback(&g_cabWrapperPoolGuardPilot);
        InitializeCabWrapperPoolOwnerPilot();
        atexit(CleanUpCabWrapperPoolPilot);
        rollback.complete();
    }
    return &g_cabWrapperPoolStoragePilot;
}
extern "C" void __cdecl RegisterCabFilePoolPilot() // 201ee703
{
    g_cabFilePoolOwnerPilot = GetCabFilePoolOwnerPilot();
}
extern "C" void __cdecl RegisterCabWrapperPoolPilot() // 201ee719
{
    g_cabWrapperPoolOwnerPilot = GetCabWrapperPoolOwnerPilot();
}
