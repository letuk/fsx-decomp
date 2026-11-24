static DWORD g_serviceNextTransportClientKeyPilot; // 2020de1c
static int g_serviceTransportAuxiliaryCapacityPilot; // 201f9814

extern "C" void WINAPI GetServiceAcceptAddressesBoundaryPilot(
    void*, DWORD, DWORD, DWORD, sockaddr**, int*, sockaddr**, int*); // GetAcceptExSockaddrs

// 20114dbd, derived slot +0x10.
static void __fastcall ActivateServiceListenerClientPilot(BYTE* client, void*)
{
    DWORD& state = ServiceClientFieldPilot<DWORD>(client, 0xc);
    if (state != 1) return;
    DWORD transport = ServiceClientFieldPilot<DWORD>(client, 0x18);
    if (transport == 0) {
        sockaddr* local = NULL;
        sockaddr* remote = NULL;
        int localLength = 0;
        int remoteLength = 0;
        GetServiceAcceptAddressesBoundaryPilot(ServiceClientFieldPilot<void*>(client, 0x3c),
            ServiceClientFieldPilot<DWORD>(client, 0x168) - 0x120,
            0x90, 0x90, &local, &localLength, &remote, &remoteLength);
        memcpy(client + 0x68, local, 0x80);
        memcpy(client + 0xe8, remote, 0x80);
        if (BindIoCompletionCallback(ServiceClientFieldPilot<HANDLE>(client, 8),
            ServiceClientIoCompletionSourcePilot, 0))
            state = 2;
    } else if (transport == 1) {
        state = 2;
        ContinueServiceClientReadSourcePilot(client);
    }
}

static void DestroyServiceClientBasePilot(BYTE* client)
{
    InterlockedDecrement(&g_serviceLiveTransportClientsPilot);
    ServiceQueueObservedPilot& queue = ServiceClientFieldPilot<ServiceQueueObservedPilot>(client, 0x20);
    if (queue.lock != NULL) {
        DeleteCriticalSection(queue.lock);
        FreeServiceClientMemoryPilot(0, 0, queue.lock);
        queue.lock = NULL;
    }
    queue.count = queue.bytes = 0;
}

// The __fastcall adapters use ECX for the receiver and ignore EDX.
static BYTE* __fastcall DeleteServiceListenerClientPilot(BYTE* client, void*, DWORD flags)
{
    if (ServiceClientFieldPilot<DWORD>(client, 0x18) == 1) {
        HANDLE& wait = ServiceClientFieldPilot<HANDLE>(client, 0x1f8);
        if (wait != INVALID_HANDLE_VALUE) {
            HANDLE registeredWait = wait;
            wait = INVALID_HANDLE_VALUE;
            UnregisterWait(registeredWait);
        }
        HANDLE& event = ServiceClientFieldPilot<HANDLE>(client, 0x1f4);
        if (event != NULL && event != INVALID_HANDLE_VALUE) CloseHandle(event);
        event = INVALID_HANDLE_VALUE;
    }
    DestroyServiceClientBasePilot(client);
    if ((flags & 1) != 0) FreeServiceClientMemoryPilot(0, 0, client);
    return client;
}

static void __fastcall StartServiceListenerClientSlotPilot(BYTE* client, void*)
{
    StartServiceClientAcceptPilot(client);
}

static void __fastcall StopServiceListenerClientSlotPilot(BYTE* client, void*, int reportError, DWORD reason)
{
    StopServiceClientAcceptPilot(client, reportError, reason);
}

// Five slots verified at 200f23d4. The callback ABI is checked per slot.
static void* g_serviceListenerClientTablePilot[] = {
    reinterpret_cast<void*>(&DeleteServiceListenerClientPilot),
    reinterpret_cast<void*>(&StartServiceListenerClientSlotPilot),
    reinterpret_cast<void*>(&StopServiceListenerClientSlotPilot),
    reinterpret_cast<void*>(&DeliverServiceReceivedMessagePilot),
    reinterpret_cast<void*>(&ActivateServiceListenerClientPilot)
};

// 20116ee1 / 20116fc5. Entries own the heap buffer once a pipe client adopts it.
static bool RegisterServicePipeNamePilot(char* name)
{
    for (int i = 0; i < g_serviceTransportAuxiliaryCountPilot; ++i) {
        if (g_serviceTransportAuxiliaryEntriesPilot[i] == name) return true;
    }
    if (g_serviceTransportAuxiliaryCountPilot == g_serviceTransportAuxiliaryCapacityPilot) {
        int capacity = g_serviceTransportAuxiliaryCapacityPilot;
        if (capacity > 0x0fffffff) return false;
        capacity = capacity == 0 ? 5 : capacity * 2;
        void** storage = static_cast<void**>(AllocateServiceClientMemoryPilot(0, 0, capacity * sizeof(*storage)));
        if (storage == NULL) return false;
        if (g_serviceTransportAuxiliaryCountPilot != 0)
            memcpy(storage, g_serviceTransportAuxiliaryEntriesPilot, g_serviceTransportAuxiliaryCountPilot * sizeof(*storage));
        FreeServiceClientMemoryPilot(0, 0, g_serviceTransportAuxiliaryEntriesPilot);
        g_serviceTransportAuxiliaryEntriesPilot = storage;
        g_serviceTransportAuxiliaryCapacityPilot = capacity;
    }
    g_serviceTransportAuxiliaryEntriesPilot[g_serviceTransportAuxiliaryCountPilot++] = name;
    return true;
}

// 2011a549 followed by 20114b77 or its socket-only inline equivalent.
static BYTE* ConstructServiceListenerClientPilot(BYTE* client, bool pipe, char* pipeName)
{
    CRITICAL_SECTION* queueLock = static_cast<CRITICAL_SECTION*>(
        AllocateServiceClientMemoryPilot(0, 0, sizeof(CRITICAL_SECTION)));
    if (queueLock == NULL) return NULL;
    InitializeCriticalSection(queueLock);
    ServiceClientFieldPilot<void**>(client, 0) = g_serviceListenerClientTablePilot;
    ServiceClientFieldPilot<SOCKET>(client, 8) = INVALID_SOCKET;
    ServiceClientFieldPilot<DWORD>(client, 0xc) = 0;
    ServiceClientFieldPilot<DWORD>(client, 0x10) = 0;
    ServiceClientFieldPilot<DWORD>(client, 0x14) = 0;
    ServiceClientFieldPilot<DWORD>(client, 0x18) = pipe ? 1 : 0;
    ServiceQueueObservedPilot& queue = ServiceClientFieldPilot<ServiceQueueObservedPilot>(client, 0x20);
    queue.lock = queueLock;
    queue.head = queue.tail = NULL;
    queue.count = queue.bytes = 0;
    client[0x34] = 0;
    ServiceClientFieldPilot<void*>(client, 0x3c) = NULL;
    ServiceClientFieldPilot<DWORD>(client, 0x40) = 0;
    ServiceClientFieldPilot<void*>(client, 0x44) = NULL;
    ServiceClientFieldPilot<void*>(client, 0x48) = NULL;
    client[0x4c] = 0;
    ServiceClientFieldPilot<ServiceReceivedMessageObservedPilot*>(client, 0x16c) = NULL;
    ServiceClientFieldPilot<SOCKET>(client, 0x170) = INVALID_SOCKET;
    client[0x178] = 0;
    ServiceClientFieldPilot<HANDLE>(client, 0x18c) = INVALID_HANDLE_VALUE;
    ServiceClientFieldPilot<DWORD>(client, 0x198) = 0;
    ServiceClientFieldPilot<HANDLE>(client, 0x1b4) = INVALID_HANDLE_VALUE;
    ServiceClientFieldPilot<void*>(client, 0x1c0) = NULL;
    ServiceClientFieldPilot<DWORD>(client, 0x1cc) = 0;
    ServiceClientFieldPilot<DWORD>(client, 0x1d0) = 0x400;
    ServiceClientFieldPilot<LONG>(client, 0x1d4) = 1;
    ServiceClientFieldPilot<DWORD>(client, 0x1d8) = 0;
    ServiceClientFieldPilot<DWORD>(client, 0x1dc) = 0xffffffff;
    client[0x1e0] = client[0x1e1] = 0;
    ServiceClientFieldPilot<HANDLE>(client, 0x1f8) = INVALID_HANDLE_VALUE;
    ServiceClientFieldPilot<char*>(client, 0x1fc) = NULL;
    ServiceClientFieldPilot<DWORD>(client, 0x200) = 0;
    ServiceClientFieldPilot<DWORD>(client, 0x204) = 0;
    if (pipe) {
        ZeroMemory(client + 0x1e4, 0x10);
        HANDLE event = CreateEventA(NULL, TRUE, FALSE, NULL);
        ServiceClientFieldPilot<HANDLE>(client, 0x1f4) = event;
        if (event != NULL && pipeName != NULL && RegisterServicePipeNamePilot(pipeName))
            ServiceClientFieldPilot<char*>(client, 0x1fc) = pipeName;
    }
    InterlockedIncrement(&g_serviceLiveTransportClientsPilot);
    return client;
}

// 2011a66f. ECX supplies addrinfo::ai_family for sockets and zero for pipes.
static bool ConfigureServiceListenerClientPilot(BYTE* client, int family, DWORD option74,
    SOCKET listener, bool noDelay)
{
    ServiceClientFieldPilot<int>(client, 0x174) = family;
    ServiceClientFieldPilot<DWORD>(client, 0x1d0) = option74;
    ServiceClientFieldPilot<SOCKET>(client, 0x170) = listener;
    client[0x178] = noDelay;
    BYTE* receiveBuffer = AllocateServiceBufferPilot(0x800);
    ServiceClientFieldPilot<BYTE*>(client, 0x3c) = receiveBuffer;
    ServiceClientFieldPilot<DWORD>(client, 0x168) = 0x800;
    ZeroMemory(client + 0x17c, sizeof(OVERLAPPED));
    ZeroMemory(client + 0x1a4, sizeof(OVERLAPPED));
    ServiceClientFieldPilot<BYTE*>(client, 0x190) = client;
    ServiceClientFieldPilot<BYTE*>(client, 0x1b8) = client;
    client[0x194] = 0;
    client[0x195] = client[0x1bc] = client[0x1bd] = 1;
    ServiceReceivedMessageObservedPilot* message = static_cast<ServiceReceivedMessageObservedPilot*>(
        AllocateServiceClientMemoryPilot(0, 0, sizeof(ServiceReceivedMessageObservedPilot)));
    if (message != NULL) ZeroMemory(message, sizeof(*message));
    ServiceClientFieldPilot<ServiceReceivedMessageObservedPilot*>(client, 0x16c) = message;
    InitializeCriticalSection(&ServiceClientFieldPilot<CRITICAL_SECTION>(client, 0x50));
    client[0x4c] = 1;
    ServiceClientFieldPilot<DWORD>(client, 4) = g_serviceNextTransportClientKeyPilot++;
    return receiveBuffer != NULL && message != NULL;
}
