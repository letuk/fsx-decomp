int g_serviceDispatchExportsCountPilot; // 201f96b8
static int g_serviceDispatchExportsCapacityPilot; // 201f96bc
static short g_serviceDispatchExportsGrowthPilot = 5; // 201f96c0
static WORD g_serviceDispatchExportsFlagsPilot = 4; // 201f96c2
FARPROC* g_serviceDispatchExportsPilot; // 201f96c4

// Growth branch of 200f787d, as reached by 200f7829.
static int ServiceArrayGrowthCapacityPilot(int required, short& growth, WORD flags, int maxCount)
{
    if (required > maxCount) return 0;
    if (growth < 1) growth = 5;
    int capacity;
    if ((flags & 4) == 0) {
        capacity = ((required + growth - 1) / growth) * growth;
    } else {
        capacity = growth;
        while (capacity < required) {
            if (capacity > maxCount / 2) {
                capacity = maxCount;
                break;
            }
            capacity *= 2;
        }
    }
    return capacity <= maxCount ? capacity : 0;
}

static bool GrowServiceDispatchExportsPilot(int required)
{
    int capacity = ServiceArrayGrowthCapacityPilot(required,
        g_serviceDispatchExportsGrowthPilot, g_serviceDispatchExportsFlagsPilot, 0x1fffffff);
    if (capacity == 0) return false;
    void* memory;
    AllocatePrivateHeapMemoryPilot(1, capacity * sizeof(FARPROC), &memory, g_factoryZeroInitializeBoundaryPilot);
    if (memory == NULL) return false;
    FARPROC* exports = static_cast<FARPROC*>(memory);
    if (g_serviceDispatchExportsCountPilot > 0)
        memcpy(exports, g_serviceDispatchExportsPilot, g_serviceDispatchExportsCountPilot * sizeof(*exports));
    if (g_serviceDispatchExportsPilot != NULL) HeapFree(g_privateHeapPilot, 0, g_serviceDispatchExportsPilot);
    g_serviceDispatchExportsCapacityPilot = capacity;
    g_serviceDispatchExportsPilot = exports;
    g_serviceDispatchExportsFlagsPilot |= 1;
    return true;
}

// 200f7829.
extern "C" void __stdcall AppendServiceDispatchExportSourcePilot(const FARPROC* entry)
{
    FARPROC dispatch = *entry;
    if (g_serviceDispatchExportsCountPilot >= 0x1fffffff) return;
    int required = g_serviceDispatchExportsCountPilot + 1;
    if (required > g_serviceDispatchExportsCapacityPilot && !GrowServiceDispatchExportsPilot(required)) return;
    g_serviceDispatchExportsPilot[g_serviceDispatchExportsCountPilot] = dispatch;
    g_serviceDispatchExportsCountPilot = required;
}

struct ServiceClientIndexEntryObservedPilot {
    DWORD key;
    BYTE* client;
};
C_ASSERT(sizeof(ServiceClientIndexEntryObservedPilot) == 8);
DWORD g_serviceClientIndexCountPilot; // 201f9820
static DWORD g_serviceClientIndexCapacityPilot; // 201f9824
static short g_serviceClientIndexGrowthPilot = 5; // 201f9828
static WORD g_serviceClientIndexFlagsPilot = 4; // 201f982a
ServiceClientIndexEntryObservedPilot* g_serviceClientIndexPilot; // 201f982c

// 20117198. Publish the lower-bound slot even for a miss.
extern "C" bool __stdcall FindServiceClientIndexEntryPilot(const DWORD* key, ServiceClientIndexEntryObservedPilot** output)
{
    DWORD first = 0, last = g_serviceClientIndexCountPilot;
    while (first < last) {
        DWORD middle = first + (last - first) / 2;
        if (g_serviceClientIndexPilot[middle].key < *key) first = middle + 1;
        else last = middle;
    }
    if (output != NULL)
        *output = first == 0 ? g_serviceClientIndexPilot : g_serviceClientIndexPilot + first;
    return first < g_serviceClientIndexCountPilot && g_serviceClientIndexPilot[first].key == *key;
}

// 201170d3 and the growth/insertion paths at 20117204, 20117312 and 2011736a.
extern "C" bool __stdcall InsertServiceClientIndexEntryPilot(DWORD key, BYTE* client)
{
    ServiceClientIndexEntryObservedPilot* slot;
    if (FindServiceClientIndexEntryPilot(&key, &slot)) return false;
    DWORD index = g_serviceClientIndexCountPilot == 0 ? 0 : slot - g_serviceClientIndexPilot;
    if (g_serviceClientIndexCountPilot >= 0x0fffffff) return false;
    DWORD required = g_serviceClientIndexCountPilot + 1;
    if (required > g_serviceClientIndexCapacityPilot) {
        int capacity = ServiceArrayGrowthCapacityPilot(required,
            g_serviceClientIndexGrowthPilot, g_serviceClientIndexFlagsPilot, 0x0fffffff);
        if (capacity == 0) return false;
        void* memory;
        AllocatePrivateHeapMemoryPilot(1, capacity * sizeof(*slot), &memory, g_factoryZeroInitializeBoundaryPilot);
        if (memory == NULL) return false;
        if (g_serviceClientIndexCountPilot != 0)
            memcpy(memory, g_serviceClientIndexPilot, g_serviceClientIndexCountPilot * sizeof(*slot));
        if (g_serviceClientIndexPilot != NULL) HeapFree(g_privateHeapPilot, 0, g_serviceClientIndexPilot);
        g_serviceClientIndexPilot = static_cast<ServiceClientIndexEntryObservedPilot*>(memory);
        g_serviceClientIndexCapacityPilot = capacity;
        g_serviceClientIndexFlagsPilot |= 1;
    }
    slot = g_serviceClientIndexPilot + index;
    DWORD tail = g_serviceClientIndexCountPilot - index;
    if (tail != 0) memmove(slot + 1, slot, tail * sizeof(*slot));
    slot->key = key;
    slot->client = client;
    g_serviceClientIndexCountPilot = required;
    return true;
}

// 20117139. The original key pointer arrives in ECX.
extern "C" bool __stdcall RemoveServiceClientIndexEntryPilot(const DWORD* key)
{
    ServiceClientIndexEntryObservedPilot* slot;
    if (!FindServiceClientIndexEntryPilot(key, &slot)) return false;
    DWORD tail = (g_serviceClientIndexPilot + g_serviceClientIndexCountPilot) - (slot + 1);
    if (tail != 0) memmove(slot, slot + 1, tail * sizeof(*slot));
    --g_serviceClientIndexCountPilot;
    return true;
}

// 20114d58. The original client arrives in ESI.
static bool QueueServiceClientPayloadPilot(const BYTE* client, DWORD bytes, void* payload)
{
    if (g_serviceWindowQueuePilot == NULL || payload == NULL) return false;
    void* memory;
    AllocatePrivateHeapMemoryPilot(1, sizeof(ServiceQueueNodeObservedPilot), &memory, g_factoryZeroInitializeBoundaryPilot);
    if (memory == NULL) return false;
    ServiceQueueNodeObservedPilot* node = static_cast<ServiceQueueNodeObservedPilot*>(memory);
    node->next = NULL;
    node->payload = payload;
    node->payloadBytes = bytes;
    node->generation = *reinterpret_cast<const DWORD*>(client + 0x1cc);
    node->unknown10 = 0;
    node->client = client;
    node->dispatchArgument = *reinterpret_cast<const DWORD*>(client + 4);
    node->timestampLow = node->timestampHigh = 0;
    RouteServiceQueueNodePilot(node);
    return true;
}

// 200f7177.
extern "C" void __stdcall DispatchServiceClientDataSourcePilot(DWORD key, DWORD, DWORD bytes, DWORD payload)
{
    ServiceClientIndexEntryObservedPilot* slot;
    if (FindServiceClientIndexEntryPilot(&key, &slot) && slot->client != NULL)
        QueueServiceClientPayloadPilot(slot->client, bytes, reinterpret_cast<void*>(payload));
}

// 201164e2. The original key arrives in EAX.
extern "C" void __stdcall SetServiceClientRegistrationSourcePilot(DWORD key, DWORD context, DWORD registration)
{
    if (g_serviceClientIndexCountPilot == 0) return;
    ServiceClientIndexEntryObservedPilot* entry;
    if (!FindServiceClientIndexEntryPilot(&key, &entry) || entry->client == NULL) return;
    *reinterpret_cast<DWORD*>(entry->client + 0x200) = context;
    *reinterpret_cast<DWORD*>(entry->client + 0x204) = registration;
}

// 200f7137 and 200f7158.
extern "C" void* __stdcall AllocateServiceClientMemoryPilot(DWORD, DWORD, DWORD bytes)
{
    void* memory;
    AllocatePrivateHeapMemoryPilot(1, bytes, &memory, g_factoryZeroInitializeBoundaryPilot);
    return memory;
}

extern "C" void __stdcall FreeServiceClientMemoryPilot(DWORD, DWORD, void* memory)
{
    if (memory != NULL) HeapFree(g_privateHeapPilot, 0, memory);
}
