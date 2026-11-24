#include "heap_boundary.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

extern "C" __declspec(dllimport) BOOL WINAPI InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION, DWORD);

struct CabLiveBufferEntryPilot {
    DWORD* key;
    DWORD* allocation;
};

template <typename Entry>
struct CabBufferArrayPilot {
    int count;
    int capacity;
    short growthStep;
    unsigned short flags;
    Entry* entries;
};

struct CabBufferCachePilot;
struct CabBufferCacheTablePilot {
    CabBufferCachePilot* (__stdcall *destroy)(CabBufferCachePilot*, unsigned char);
};

struct CabBufferCachePilot {
    const CabBufferCacheTablePilot* vtable;
    CRITICAL_SECTION lock;
    CabBufferArrayPilot<CabLiveBufferEntryPilot> live;
    CabBufferArrayPilot<DWORD*> cached;
    int limit;
};

C_ASSERT(sizeof(CabLiveBufferEntryPilot) == 8);
C_ASSERT(sizeof(CabBufferArrayPilot<DWORD*>) == 16);
C_ASSERT(offsetof(CabBufferCachePilot, lock) == 4);
C_ASSERT(offsetof(CabBufferCachePilot, live) == 0x1c);
C_ASSERT(offsetof(CabBufferCachePilot, cached) == 0x2c);
C_ASSERT(offsetof(CabBufferCachePilot, limit) == 0x3c);
C_ASSERT(sizeof(CabBufferCachePilot) == 0x40);

static CabBufferCachePilot g_cabBufferCachePilot; // 2020e1a8
extern int g_alternateAllocationZeroInitializePilot; // 2020d8ec

// 20118423 (live), 2011448e (cached). Original container input is in EDI.
template <typename Entry>
static bool ResizeCabBufferArray(CabBufferArrayPilot<Entry>& array, int requested)
{
    const int maxCount = INT_MAX / sizeof(Entry);
    if (requested > maxCount) return false;
    int capacity = 0;
    if (requested > 0) {
        if (array.growthStep < 1) array.growthStep = 5;
        capacity = array.growthStep;
        if ((array.flags & 4) != 0) {
            while (capacity < requested) capacity *= 2;
        } else {
            capacity = ((requested + capacity - 1) / capacity) * capacity;
        }
    }
    if (capacity == array.capacity) return true;

    void* memory = NULL;
    if (capacity > 0) {
        AllocatePrivateHeapMemoryPilot(1, capacity * sizeof(Entry), &memory,
            g_alternateAllocationZeroInitializePilot);
        if (memory == NULL) return false;
    }
    int retained = array.count < requested ? array.count : requested;
    if (retained > 0) {
        if (memory == NULL) return false;
        memcpy(memory, array.entries, retained * sizeof(Entry));
    }
    array.count = retained;
    if (array.entries != NULL) HeapFree(g_privateHeapPilot, 0, array.entries);
    array.capacity = capacity;
    array.flags |= 1;
    array.entries = static_cast<Entry*>(memory);
    return true;
}

template <typename Entry>
static int PrepareCabBufferAppend(CabBufferArrayPilot<Entry>& array)
{
    const int maxCount = INT_MAX / sizeof(Entry);
    int next = static_cast<int>(static_cast<unsigned>(array.count) + 1);
    if (next < 0) next = 0;
    if (next > maxCount) next = maxCount;
    // Original append and insert callers proceed even if allocation fails.
    if (next > array.capacity) ResizeCabBufferArray(array, next);
    return next;
}

// 20127fa8 (live), 20127f43 (cached).
template <typename Entry>
static Entry* InsertCabBufferEntry(CabBufferArrayPilot<Entry>& array, int index)
{
    PrepareCabBufferAppend(array);
    Entry* slot = array.entries + index;
    memmove(slot + 1, slot, (array.count - index) * sizeof(Entry));
    ++array.count;
    return slot;
}

// 2011468d. The key is the unsigned address, not the DWORD at that address.
static bool FindLiveCabBuffer(const CabBufferArrayPilot<CabLiveBufferEntryPilot>& live,
    const DWORD* key, CabLiveBufferEntryPilot** insertion)
{
    unsigned first = 0;
    unsigned last = live.count;
    while (first < last) {
        unsigned middle = first + (last - first) / 2;
        if (reinterpret_cast<ULONG_PTR>(live.entries[middle].key) < reinterpret_cast<ULONG_PTR>(key))
            first = middle + 1;
        else
            last = middle;
    }
    CabLiveBufferEntryPilot* entry = first == 0 ? live.entries : live.entries + first;
    if (insertion != NULL) *insertion = entry;
    return first < static_cast<unsigned>(live.count) && entry->key == key;
}

// 2011369a, including the empty-array append at 20114798.
static bool TrackCabBuffer(CabBufferArrayPilot<CabLiveBufferEntryPilot>& live,
    DWORD* key, DWORD* allocation)
{
    CabLiveBufferEntryPilot* slot;
    if (live.count == 0) {
        live.count = PrepareCabBufferAppend(live);
        slot = live.entries + live.count - 1;
    } else {
        if (FindLiveCabBuffer(live, key, &slot)) return false;
        slot = InsertCabBufferEntry(live, static_cast<int>(slot - live.entries));
    }
    slot->key = key;
    slot->allocation = allocation;
    return true;
}

// 2011cbcc.
static void AppendCabCachedBuffer(CabBufferArrayPilot<DWORD*>& cached, DWORD* allocation)
{
    PrepareCabBufferAppend(cached);
    DWORD** slot = cached.count == 0 ? cached.entries : cached.entries + cached.count;
    if (slot != NULL) *slot = allocation;
    ++cached.count;
}

template <typename Entry>
static void FreeOwnedCabBufferArray(CabBufferArrayPilot<Entry>& array)
{
    if (array.entries != NULL && (array.flags & 1) != 0 && array.capacity != 0) {
        if (array.count > 0) array.count = 0;
        HeapFree(g_privateHeapPilot, 0, array.entries);
        array.flags |= 1;
        array.entries = NULL;
        array.capacity = 0;
    }
}

static void DestroyCabBufferCache(CabBufferCachePilot* cache);

// 20127d78. Source table uses an explicit self parameter.
static CabBufferCachePilot* __stdcall DeleteCabBufferCache(CabBufferCachePilot* cache, unsigned char flags)
{
    DestroyCabBufferCache(cache);
    if ((flags & 1) != 0 && cache != NULL) HeapFree(g_privateHeapPilot, 0, cache);
    return cache;
}
static const CabBufferCacheTablePilot CabBufferCacheTable = { DeleteCabBufferCache }; // 200f2d4c

// 201274a6. Startup registration is kept separate from the initializer.
extern "C" CabBufferCachePilot* __cdecl InitializeCabBufferCachePilot()
{
    CabBufferCachePilot* cache = &g_cabBufferCachePilot;
    cache->vtable = &CabBufferCacheTable;
    InitializeCriticalSectionAndSpinCount(&cache->lock, 100);
    const CabBufferArrayPilot<CabLiveBufferEntryPilot> live = { 0, 0, 5, 4, NULL };
    const CabBufferArrayPilot<DWORD*> cached = { 0, 0, 5, 4, NULL };
    cache->live = live;
    cache->cached = cached;
    cache->limit = INT_MAX;
    return cache;
}

// 20127513. Release buffer payloads and arrays under the cache lock.
static void DestroyCabBufferCache(CabBufferCachePilot* cache)
{
    cache->vtable = &CabBufferCacheTable;
    EnterCriticalSection(&cache->lock);
    for (int index = 0; index < cache->live.count; ++index) {
        DWORD* allocation = cache->live.entries[index].allocation;
        if (allocation != NULL) HeapFree(g_privateHeapPilot, 0, allocation);
    }
    cache->live.count = 0;
    ResizeCabBufferArray(cache->live, 0);
    for (int index = 0; index < cache->cached.count; ++index) {
        DWORD* allocation = cache->cached.entries[index];
        if (allocation != NULL) HeapFree(g_privateHeapPilot, 0, allocation);
    }
    cache->cached.count = 0;
    ResizeCabBufferArray(cache->cached, 0);
    LeaveCriticalSection(&cache->lock);
    FreeOwnedCabBufferArray(cache->cached);
    FreeOwnedCabBufferArray(cache->live);
    DeleteCriticalSection(&cache->lock);
}

// 201eeeed and 201ee724. Explicit source entry point for CRT registration.
static void __cdecl CleanUpCabBufferCache()
{
    DestroyCabBufferCache(&g_cabBufferCachePilot);
}
extern "C" void __cdecl RegisterCabBufferCachePilot()
{
    InitializeCabBufferCachePilot();
    atexit(CleanUpCabBufferCache);
}

// 20127dd3. Caller holds cache lock. First size at least as large as requested.
extern "C" int __stdcall FindCabCachedBufferPilot(DWORD requested)
{
    int first = 0;
    int last = g_cabBufferCachePilot.cached.count;
    int result = -1;
    while (first < last) {
        int middle = (first + last) / 2;
        if (*g_cabBufferCachePilot.cached.entries[middle] < requested) first = middle + 1;
        else {
            last = middle;
            result = middle;
        }
    }
    return result;
}

// 20127642. Original size input is in EAX. This is its source interface.
extern "C" void* __stdcall AllocateCabBufferSourcePilot(DWORD requested)
{
    if (requested == 0) return NULL;
    EnterCriticalSection(&g_cabBufferCachePilot.lock);
    int index = FindCabCachedBufferPilot(requested);
    if (index >= 0) {
        DWORD** slot = g_cabBufferCachePilot.cached.entries + index;
        DWORD* allocation = *slot;
        memmove(slot, slot + 1, (g_cabBufferCachePilot.cached.count - index - 1) * sizeof(*slot));
        --g_cabBufferCachePilot.cached.count;
        TrackCabBuffer(g_cabBufferCachePilot.live, allocation, allocation);
        LeaveCriticalSection(&g_cabBufferCachePilot.lock);
        return allocation + 1;
    }
    LeaveCriticalSection(&g_cabBufferCachePilot.lock);

    void* memory;
    AllocatePrivateHeapMemoryPilot(1, requested + 4, &memory, g_alternateAllocationZeroInitializePilot);
    DWORD* allocation = static_cast<DWORD*>(memory);
    if (allocation == NULL) return NULL;
    *allocation = requested;
    EnterCriticalSection(&g_cabBufferCachePilot.lock);
    TrackCabBuffer(g_cabBufferCachePilot.live, allocation, allocation);
    LeaveCriticalSection(&g_cabBufferCachePilot.lock);
    return allocation + 1;
}

// 2012772b, RET4. Misses leave the cache untouched.
extern "C" void __stdcall ReturnCabBufferPilot(void* payload)
{
    EnterCriticalSection(&g_cabBufferCachePilot.lock);
    int liveCount = g_cabBufferCachePilot.live.count;
    DWORD* allocation = static_cast<DWORD*>(payload) - 1;
    CabLiveBufferEntryPilot* entry;
    if (liveCount != 0 && FindLiveCabBuffer(g_cabBufferCachePilot.live, allocation, &entry)) {
        int index = static_cast<int>(entry - g_cabBufferCachePilot.live.entries);
        if (index >= 0) {
            allocation = entry->allocation;
            memmove(entry, entry + 1, (liveCount - index - 1) * sizeof(*entry));
            --g_cabBufferCachePilot.live.count;
            if (g_cabBufferCachePilot.cached.count >= g_cabBufferCachePilot.limit) {
                int victimIndex = rand() % g_cabBufferCachePilot.limit;
                DWORD** slot = g_cabBufferCachePilot.cached.entries + victimIndex;
                DWORD* victim = *slot;
                memmove(slot, slot + 1, (g_cabBufferCachePilot.cached.count - victimIndex - 1) * sizeof(*slot));
                --g_cabBufferCachePilot.cached.count;
                if (victim != NULL) HeapFree(g_privateHeapPilot, 0, victim);
            }
            int insert = FindCabCachedBufferPilot(*allocation);
            if (insert < 0) AppendCabCachedBuffer(g_cabBufferCachePilot.cached, allocation);
            else {
                DWORD** slot = InsertCabBufferEntry(g_cabBufferCachePilot.cached, insert);
                if (slot != NULL) *slot = allocation;
            }
        }
    }
    LeaveCriticalSection(&g_cabBufferCachePilot.lock);
}
