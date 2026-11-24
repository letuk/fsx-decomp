struct ServiceTransportTreeNodeObservedPilot {
    ServiceTransportTreeNodeObservedPilot* left;
    ServiceTransportTreeNodeObservedPilot* parent;
    ServiceTransportTreeNodeObservedPilot* right;
    LONG key;
    BYTE* client;
    BYTE color;
    BYTE sentinel;
};
struct ServiceTransportTreeObservedPilot {
    DWORD unknown00;
    ServiceTransportTreeNodeObservedPilot* head;
    DWORD count;
};
C_ASSERT(offsetof(ServiceTransportTreeNodeObservedPilot, sentinel) == 0x15);
extern ServiceTransportTreeObservedPilot* g_serviceTransportTreePilot; // 2020db68

// 20116676, reduced to its exact-key lookup result.
static BYTE* FindServiceTransportClientPilot(LONG key)
{
    if (g_serviceTransportTreePilot == NULL) return NULL;
    ServiceTransportTreeNodeObservedPilot* head = g_serviceTransportTreePilot->head;
    ServiceTransportTreeNodeObservedPilot* candidate = head;
    for (ServiceTransportTreeNodeObservedPilot* node = head->parent; node->sentinel == 0;) {
        if (node->key < key) {
            node = node->right;
        } else {
            candidate = node;
            node = node->left;
        }
    }
    return candidate != head && candidate->key == key ? candidate->client : NULL;
}

// 2011639f.
extern "C" DWORD __stdcall ReadServicePacketTransportGenerationSourcePilot(DWORD clientId)
{
    if (g_serviceClientRegistrationDisabledPilot != 0) return 0;
    BYTE* client = FindServiceTransportClientPilot(static_cast<LONG>(clientId));
    return client != NULL ? ServiceClientFieldPilot<DWORD>(client, 0x1cc) : 0;
}

// 2011a9d8. Client state and queue use distinct locks.
static void QueueServiceTransportWritePilot(BYTE* client, ServiceQueueNodeObservedPilot* node)
{
    CRITICAL_SECTION* lock = &ServiceClientFieldPilot<CRITICAL_SECTION>(client, 0x50);
    bool locked = client[0x4c] != 0;
    if (locked) EnterCriticalSection(lock);
    if (ServiceClientFieldPilot<DWORD>(client, 0xc) == 2) {
        AppendServiceQueueNodePilot(&ServiceClientFieldPilot<ServiceQueueObservedPilot>(client, 0x20), node);
        ContinueServiceClientWriteSourcePilot(client, 0);
        if (locked) LeaveCriticalSection(lock);
        return;
    }
    if (locked) LeaveCriticalSection(lock);
    FreeServiceQueueChainPilot(node);
}

// 2011628d. Ownership is transferred to the callback or write queue.
extern "C" void __stdcall SendServicePacketBufferSourcePilot(DWORD clientId, void* payload, DWORD bytes)
{
    BYTE* client = FindServiceTransportClientPilot(static_cast<LONG>(clientId));
    if (client == NULL) {
        FreeServiceClientMemoryPilot(0, 0, payload);
        return;
    }
    typedef void (__stdcall *SendCallbackPilot)(DWORD, DWORD, DWORD, void*);
    SendCallbackPilot send = ServiceClientFieldPilot<SendCallbackPilot>(client, 0x204);
    DWORD generation = ServiceClientFieldPilot<DWORD>(client, 0x1cc);
    if (send != NULL && ServiceClientFieldPilot<DWORD>(client, 0xc) != 0) {
        send(generation, ServiceClientFieldPilot<DWORD>(client, 0x200), bytes, payload);
        return;
    }
    const DWORD nodeBytes = offsetof(ServiceQueueNodeObservedPilot, client);
    C_ASSERT(offsetof(ServiceQueueNodeObservedPilot, client) == 0x14);
    ServiceQueueNodeObservedPilot* node = static_cast<ServiceQueueNodeObservedPilot*>(
        AllocateServiceClientMemoryPilot(0, 0, nodeBytes));
    if (node == NULL) {
        FreeServiceClientMemoryPilot(0, 0, payload);
        return;
    }
    node->next = NULL;
    node->payload = payload;
    node->payloadBytes = bytes;
    node->generation = generation;
    node->unknown10 = 0;
    QueueServiceTransportWritePilot(client, node);
}
