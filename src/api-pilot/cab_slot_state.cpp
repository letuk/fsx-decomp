#include "heap_boundary.h"
#include "cab_atomic_state.h"
#include <string.h>

struct CabSlotNodePilot {
    CabSlotNodePilot* next;
    void* payload;
};
struct CabSlotHeadObservedPilot {
    CabSlotNodePilot* volatile node;
    volatile DWORD sequence;
};
struct CabPoolSlotStatePilot {
    CabSlotHeadObservedPilot freeNodes, publishedNodes;
    volatile LONG freeWaitCount;
    HANDLE freeSemaphore;
    volatile LONG publishedWaitCount;
    HANDLE publishedSemaphore;
    CabSlotHeadObservedPilot allocationChunks;
    LONG chunkNodes;
    volatile LONG remainingNodes, allocatedNodes;
    LONG allocationBytes;
    volatile LONG signature;
    void* rawStateAllocation;
    CabSlotAllocatorObservedPilot* allocator;
    DWORD unknown44;
};
C_ASSERT(sizeof(CabSlotNodePilot) == 8);
C_ASSERT(sizeof(CabPoolSlotStatePilot) == 0x48);
C_ASSERT(offsetof(CabPoolSlotStatePilot, allocationChunks) == 0x20);
C_ASSERT(offsetof(CabPoolSlotStatePilot, signature) == 0x38);
C_ASSERT(offsetof(CabPoolSlotStatePilot, allocator) == 0x40);

volatile signed char g_cabSlotAtomicModePilot = 2; // 201f7090
// These x86 primitives keep the unprefixed operations indivisible at the
// instruction boundary. The caller supplies the observed LOCK predicate.
extern "C" LONG __stdcall CabSlotCompareExchangeSourcePilot(volatile LONG* address,
    LONG desired, LONG expected, BOOL locked)
{
    LONG previous;
    __asm {
        mov ecx, address
        mov edx, desired
        mov eax, expected
        cmp locked, 0
        je exchange_unprefixed
        lock cmpxchg [ecx], edx
        jmp exchange_done
    exchange_unprefixed:
        cmpxchg [ecx], edx
    exchange_done:
        mov previous, eax
    }
    return previous;
}

extern "C" LONG __stdcall CabSlotFetchAddSourcePilot(volatile LONG* address, LONG value, BOOL locked)
{
    LONG previous;
    __asm {
        mov ecx, address
        mov eax, value
        cmp locked, 0
        je add_unprefixed
        lock xadd [ecx], eax
        jmp add_done
    add_unprefixed:
        xadd [ecx], eax
    add_done:
        mov previous, eax
    }
    return previous;
}

extern "C" void __stdcall PushCabSlotNodeSourcePilot(CabSlotHeadObservedPilot* head,
    CabSlotNodePilot* node, BOOL locked)
{
    LONG expected = reinterpret_cast<LONG>(head->node);
    LONG desired = reinterpret_cast<LONG>(node);
    for (;;) {
        node->next = reinterpret_cast<CabSlotNodePilot*>(expected);
        LONG previous = CabSlotCompareExchangeSourcePilot(
            reinterpret_cast<volatile LONG*>(&head->node), desired, expected, locked);
        if (previous == expected) return;
        expected = previous;
    }
}

bool CompareExchangeCabPair(void* address, DWORD& low, DWORD& high,
    DWORD nextLow, DWORD nextHigh, BOOL locked)
{
    DWORD observedLow = low;
    DWORD observedHigh = high;
    unsigned char exchanged;
    __asm {
        mov esi, address
        mov eax, observedLow
        mov edx, observedHigh
        mov ebx, nextLow
        mov ecx, nextHigh
        cmp locked, 0
        je pair_unprefixed
        lock cmpxchg8b [esi]
        jmp pair_done
    pair_unprefixed:
        cmpxchg8b [esi]
    pair_done:
        setz exchanged
        mov observedLow, eax
        mov observedHigh, edx
    }
    low = observedLow;
    high = observedHigh;
    return exchanged != 0;
}

extern "C" CabSlotNodePilot* __stdcall PopCabSlotNodeSourcePilot(CabSlotHeadObservedPilot* head, BOOL locked)
{
    DWORD sequence = head->sequence;
    DWORD nodeAddress = reinterpret_cast<DWORD>(head->node);
    while (nodeAddress != 0) {
        CabSlotNodePilot* node = reinterpret_cast<CabSlotNodePilot*>(nodeAddress);
        if (CompareExchangeCabPair(head, nodeAddress, sequence,
                reinterpret_cast<DWORD>(node->next), sequence + 1, locked)) return node;
    }
    return NULL;
}

// 20145d4c. Allocator descriptor originally arrives in EAX.
extern "C" void* __stdcall AllocateCabSlotRawSourcePilot(CabSlotAllocatorObservedPilot* allocator,
    DWORD bytes, LONG* allocatedBytes)
{
    if (allocator != NULL) {
        void* allocation = allocator->allocate(allocator->context, bytes);
        if (allocatedBytes != NULL)
            CabSlotFetchAddSourcePilot(allocatedBytes, bytes, g_cabSlotAtomicModePilot > 0);
        return allocation;
    }
    HANDLE heap = GetProcessHeap();
    void* allocation = HeapAlloc(heap, 0, bytes);
    if (allocation != NULL && allocatedBytes != NULL) {
        SIZE_T used = HeapSize(heap, 0, allocation);
        if (static_cast<LONG>(used) > 0)
            CabSlotFetchAddSourcePilot(allocatedBytes, used, g_cabSlotAtomicModePilot > 0);
    }
    return allocation;
}

// 20145e2f. Original EAX allocator, EDX size and EDI raw-allocation output.
extern "C" void* __stdcall AllocateCabSlotAlignedSourcePilot(CabSlotAllocatorObservedPilot* allocator,
    DWORD bytes, void** raw, LONG* allocatedBytes)
{
    if (static_cast<LONG>(bytes) <= 0) {
        *raw = NULL;
        return NULL;
    }
    DWORD alignment = 64;
    if (bytes <= 32) {
        alignment = 32;
        while (bytes <= alignment / 2) alignment /= 2;
    }
    if (alignment <= (allocator != NULL ? allocator->alignment : 8)) alignment = 1;
    DWORD total = alignment + bytes;
    if (static_cast<LONG>(total) <= 1) {
        *raw = NULL;
        return NULL;
    }
    *raw = AllocateCabSlotRawSourcePilot(allocator, total - 1, allocatedBytes);
    ULONG_PTR address = reinterpret_cast<ULONG_PTR>(*raw);
    return reinterpret_cast<void*>(address + ((alignment - address) & (alignment - 1)));
}

// 20145ec2. The trailing list node records the raw allocation for release.
extern "C" CabSlotNodePilot* __stdcall AllocateCabSlotChunkSourcePilot(LONG* allocatedBytes,
    LONG nodeBytes, CabSlotAllocatorObservedPilot* allocator, CabSlotHeadObservedPilot* chunks)
{
    DWORD bytes = static_cast<DWORD>(nodeBytes) + sizeof(CabSlotNodePilot);
    if (nodeBytes <= 0 || bytes == 0) return NULL;
    void* raw;
    BYTE* aligned = static_cast<BYTE*>(AllocateCabSlotAlignedSourcePilot(allocator, bytes, &raw, allocatedBytes));
    if (aligned == NULL) return NULL;
    CabSlotNodePilot* trailer = reinterpret_cast<CabSlotNodePilot*>(aligned + nodeBytes);
    trailer->payload = raw;
    PushCabSlotNodeSourcePilot(chunks, trailer, g_cabSlotAtomicModePilot >= 0);
    return reinterpret_cast<CabSlotNodePilot*>(aligned);
}

// 20145ddc. The original allocation pointer arrives in ESI.
extern "C" void __stdcall FreeCabSlotRawStateSourcePilot(void* raw, LONG* allocatedBytes)
{
    HANDLE heap = GetProcessHeap();
    if (allocatedBytes != NULL) {
        SIZE_T used = HeapSize(heap, 0, raw);
        if (static_cast<LONG>(used) > 0)
            CabSlotFetchAddSourcePilot(allocatedBytes, 0u - used, g_cabSlotAtomicModePilot > 0);
    }
    HeapFree(GetProcessHeap(), 0, raw);
}

// 20145f32. The original chunk-list head arrives in EAX.
extern "C" void __stdcall FreeCabSlotChunksSourcePilot(CabSlotHeadObservedPilot* chunks,
    CabSlotAllocatorObservedPilot* allocator, LONG* allocatedBytes)
{
    for (;;) {
        CabSlotNodePilot* node = PopCabSlotNodeSourcePilot(chunks, g_cabSlotAtomicModePilot >= 0);
        if (node == NULL) return;
        if (allocator == NULL) FreeCabSlotRawStateSourcePilot(node->payload, allocatedBytes);
        else allocator->release(allocator->context, node->payload);
    }
}

void InitializeCabSlotAtomicMode()
{
    if (g_cabSlotAtomicModePilot == 2) {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        DWORD_PTR mask = info.dwActiveProcessorMask;
        g_cabSlotAtomicModePilot = info.dwNumberOfProcessors <= 1 && (mask & (mask - 1)) == 0 ? -1 : 1;
        // Only the synchronization effect of the original scratch increment is used.
        LONG scratch = 0;
        InterlockedIncrement(&scratch);
    }
}

void FinishCabSlotMutation()
{
    LONG scratch = 0;
    CabSlotFetchAddSourcePilot(&scratch, 1, g_cabSlotAtomicModePilot > 0);
}

// Original 20145fe3, RET8. Semaphores and lists start zeroed.
extern "C" CabPoolSlotStatePilot* __stdcall CreateCabPoolSlotsPilot(LONG chunkNodes, LONG nodeLimit)
{
    LONG allocatedBytes = 0;
    void* raw;
    CabPoolSlotStatePilot* state = static_cast<CabPoolSlotStatePilot*>(
        AllocateCabSlotAlignedSourcePilot(NULL, 0x48, &raw, &allocatedBytes));
    if (state == NULL) return NULL;
    memset(state, 0, sizeof(*state));
    state->rawStateAllocation = raw;
    state->allocationBytes = allocatedBytes;
    if (chunkNodes < 16) chunkNodes = 16;
    else if (chunkNodes > 256) chunkNodes = 256;
    if (nodeLimit < 1) nodeLimit = -1;
    else {
        if (nodeLimit >= 0x0fffffff) nodeLimit = 0x0ffffffe;
        if (nodeLimit < chunkNodes) chunkNodes = nodeLimit;
    }
    state->chunkNodes = chunkNodes;
    state->remainingNodes = nodeLimit;
    state->signature = 0x4c665374;
    if (g_cabSlotAtomicModePilot == 2) InitializeCabSlotAtomicMode();
    FinishCabSlotMutation();
    return state;
}

// Original 201460ff takes state in EAX and returns 1 on every path.
extern "C" int __stdcall DestroyCabPoolSlotsSourcePilot(CabPoolSlotStatePilot* state)
{
    if (state != NULL && InterlockedExchange(&state->signature, 0) == 0x4c665374) {
        InitializeCabSlotAtomicMode();
        if (state->freeSemaphore != NULL) CloseHandle(state->freeSemaphore);
        if (state->publishedSemaphore != NULL) CloseHandle(state->publishedSemaphore);
        LONG allocatedBytes = state->allocationBytes;
        FreeCabSlotChunksSourcePilot(&state->allocationChunks, state->allocator, &allocatedBytes);
        if (state->allocator == NULL) FreeCabSlotRawStateSourcePilot(state->rawStateAllocation, &allocatedBytes);
        else state->allocator->release(state->allocator->context, state->rawStateAllocation);
        FinishCabSlotMutation();
    }
    return 1;
}

// Original 201461b4, RET4. Keep one node and publish the rest in reverse order.
extern "C" CabSlotNodePilot* __stdcall GrowCabSlotNodesPilot(CabPoolSlotStatePilot* state)
{
    LONG count = state->chunkNodes;
    for (;;) {
        LONG budget = state->remainingNodes;
        if (budget < 0) break;
        if (budget == 0) return NULL;
        if (budget < count) count = budget;
        if (CabSlotCompareExchangeSourcePilot(&state->remainingNodes, budget - count, budget,
                g_cabSlotAtomicModePilot > 0) == budget) break;
    }
    CabSlotNodePilot* nodes = AllocateCabSlotChunkSourcePilot(&state->allocationBytes,
        count * sizeof(CabSlotNodePilot), state->allocator, &state->allocationChunks);
    if (nodes == NULL) {
        state->remainingNodes = 0;
        return NULL;
    }
    CabSlotFetchAddSourcePilot(&state->allocatedNodes, count, g_cabSlotAtomicModePilot > 0);
    for (LONG index = count - 1; index > 0; --index)
        PushCabSlotNodeSourcePilot(&state->freeNodes, nodes + index, g_cabSlotAtomicModePilot >= 0);
    if (state->freeSemaphore != NULL) {
        LONG previous = CabSlotFetchAddSourcePilot(&state->freeWaitCount, count, g_cabSlotAtomicModePilot > 0);
        if (previous < -1 && count > 1) {
            LONG pending = ~previous;
            ReleaseSemaphore(state->freeSemaphore, pending < count - 1 ? pending : count - 1, NULL);
        }
    }
    return nodes;
}

static LONG CabSlotCountAfter(LONG before, DWORD change)
{
    return static_cast<LONG>(static_cast<DWORD>(before) + change);
}

static CabSlotNodePilot* TakeCabSlotNode(CabSlotHeadObservedPilot* head, volatile LONG* waitCount,
    bool counted, CabPoolSlotStatePilot* growthState)
{
    CabSlotNodePilot* node = NULL;
    if (!counted || CabSlotCountAfter(CabSlotFetchAddSourcePilot(waitCount, -1,
            g_cabSlotAtomicModePilot >= 0), 0xffffffff) >= 0)
        node = PopCabSlotNodeSourcePilot(head, g_cabSlotAtomicModePilot >= 0);
    if (node == NULL && growthState != NULL && growthState->remainingNodes != 0)
        node = GrowCabSlotNodesPilot(growthState);
    if (node == NULL && counted)
        CabSlotFetchAddSourcePilot(waitCount, 1, g_cabSlotAtomicModePilot > 0);
    return node;
}

static void SignalCabSlotNode(HANDLE semaphore, volatile LONG* waitCount)
{
    if (semaphore != NULL && CabSlotCountAfter(CabSlotFetchAddSourcePilot(waitCount, 1,
            g_cabSlotAtomicModePilot > 0), 1) <= 0)
        ReleaseSemaphore(semaphore, 1, NULL);
}

// 201462fc. The original receives state in EDI and payload on the stack.
extern "C" int __stdcall PublishCabPoolSlotSourcePilot(CabPoolSlotStatePilot* state, void* payload)
{
    if (payload == NULL || state == NULL || state->signature != 0x4c665374) return 0;
    InitializeCabSlotAtomicMode();
    CabSlotNodePilot* node = TakeCabSlotNode(&state->freeNodes, &state->freeWaitCount,
        state->freeSemaphore != NULL, state);
    if (node == NULL) return 0;
    node->payload = payload;
    PushCabSlotNodeSourcePilot(&state->publishedNodes, node, g_cabSlotAtomicModePilot >= 0);
    SignalCabSlotNode(state->publishedSemaphore, &state->publishedWaitCount);
    return 1;
}

// 201464c3. The original receives state in EDI.
extern "C" void* __stdcall ReserveCabPoolSlotSourcePilot(CabPoolSlotStatePilot* state)
{
    if (state == NULL || state->signature != 0x4c665374) return NULL;
    InitializeCabSlotAtomicMode();
    CabSlotNodePilot* node = TakeCabSlotNode(&state->publishedNodes, &state->publishedWaitCount,
        state->publishedSemaphore != NULL, NULL);
    if (node == NULL) return NULL;
    void* payload = node->payload;
    PushCabSlotNodeSourcePilot(&state->freeNodes, node, g_cabSlotAtomicModePilot >= 0);
    SignalCabSlotNode(state->freeSemaphore, &state->freeWaitCount);
    return payload;
}
