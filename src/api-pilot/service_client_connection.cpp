static DWORD g_serviceConnectionGenerationPilot; // 2020de20
static volatile LONG g_serviceConnectionStatusPendingPilot; // 2020de28
static DWORD g_serviceConnectionPollCounterPilot; // 2020e2c4

extern "C" void __stdcall RemoveServiceConnectionRegistrationBoundaryPilot(DWORD); // 200fe4cc

static HANDLE CreateServicePipeIoEventPilot()
{
    HANDLE event = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (event == NULL) printf("CreateEvent failed with %d.\n", GetLastError());
    return event;
}

// 2011a7ed.
extern "C" void __stdcall ResetServiceClientConnectionSourcePilot(BYTE* client)
{
    ServiceQueueObservedPilot& queue = ServiceClientFieldPilot<ServiceQueueObservedPilot>(client, 0x20);
    FreeServiceQueueChainPilot(DetachServiceQueuePilot(&queue));
    CRITICAL_SECTION* lock = client[0x4c] == 0 ? NULL :
        &ServiceClientFieldPilot<CRITICAL_SECTION>(client, 0x50);
    if (lock != NULL) EnterCriticalSection(lock);
    void*& activeWrite = ServiceClientFieldPilot<void*>(client, 0x1c0);
    FreeServiceClientMemoryPilot(0, 0, activeWrite);
    activeWrite = NULL;
    client[0x34] = 0;
    if (++g_serviceConnectionGenerationPilot == 0) g_serviceConnectionGenerationPilot = 1;
    ServiceClientFieldPilot<DWORD>(client, 0x1cc) = g_serviceConnectionGenerationPilot;
    ServiceClientFieldPilot<DWORD>(client, 0x1dc) = 0xffffffff;
    if (lock != NULL) LeaveCriticalSection(lock);

    BYTE* receiveBuffer = ServiceClientFieldPilot<BYTE*>(client, 0x3c);
    ServiceClientFieldPilot<BYTE*>(client, 0x44) = receiveBuffer;
    ServiceClientFieldPilot<BYTE*>(client, 0x48) = receiveBuffer;
    if (ServiceClientFieldPilot<DWORD>(client, 0x18) == 1) {
        ServiceClientFieldPilot<HANDLE>(client, 0x1b4) = CreateServicePipeIoEventPilot();
        ServiceClientFieldPilot<HANDLE>(client, 0x18c) = CreateServicePipeIoEventPilot();
    }
}

static void CloseServiceClientHandlePilot(HANDLE& handle)
{
    if (handle == INVALID_HANDLE_VALUE) return;
    HANDLE previous = handle;
    handle = INVALID_HANDLE_VALUE;
    if (previous != NULL) CloseHandle(previous);
}

// 2011a8d4.
extern "C" void __stdcall CloseServiceClientConnectionSourcePilot(BYTE* client, int reportError, DWORD reason)
{
    CRITICAL_SECTION* lock = client[0x4c] == 0 ? NULL :
        &ServiceClientFieldPilot<CRITICAL_SECTION>(client, 0x50);
    if (lock != NULL) EnterCriticalSection(lock);
    DWORD& state = ServiceClientFieldPilot<DWORD>(client, 0xc);
    DWORD previousState = state;
    state = 0;
    ServiceClientFieldPilot<DWORD>(client, 0x1cc) = 0;
    if (lock != NULL) LeaveCriticalSection(lock);

    DWORD transport = ServiceClientFieldPilot<DWORD>(client, 0x18);
    if (previousState != 0 && transport == 0) {
        if (reportError != 0) {
            ServiceClientFieldPilot<DWORD>(client, 0x10) = WSAGetLastError();
            ServiceClientFieldPilot<DWORD>(client, 0x14) = reason;
        }
        SOCKET& socket = ServiceClientFieldPilot<SOCKET>(client, 8);
        if (previousState == 2) {
            shutdown(socket, SD_BOTH);
            CancelIo(reinterpret_cast<HANDLE>(socket));
        }
        SOCKET previous = socket;
        socket = INVALID_SOCKET;
        closesocket(previous);
    } else if (previousState != 0 && transport == 1) {
        if (reportError != 0) {
            ServiceClientFieldPilot<DWORD>(client, 0x10) = GetLastError();
            ServiceClientFieldPilot<DWORD>(client, 0x14) = reason;
        }
        CloseServiceClientHandlePilot(ServiceClientFieldPilot<HANDLE>(client, 8));
        CloseServiceClientHandlePilot(ServiceClientFieldPilot<HANDLE>(client, 0x18c));
        CloseServiceClientHandlePilot(ServiceClientFieldPilot<HANDLE>(client, 0x1b4));
    }
    ServiceClientFieldPilot<LONG>(client, 0x1d8) = reportError;
    if (reportError != 0) InterlockedExchange(&g_serviceConnectionStatusPendingPilot, 1);
}

// 20116120. Consume one report from the listener groups.
static bool ReadServiceConnectionStatusPilot(bool force, DWORD& key, LONG& status, DWORD& reason)
{
    const int connectTimeOption = 0x700c; // SO_CONNECT_TIME in the original.
    LONG pending = InterlockedExchange(&g_serviceConnectionStatusPendingPilot, 0);
    ServiceClientGroupsObservedPilot* groups = g_serviceClientGroupsPilot;
    if ((!force && pending == 0) || groups == NULL) return false;
    for (ServiceClientGroupObservedPilot** group = groups->begin; group != groups->end; ++group) {
        for (BYTE** entry = (*group)->begin; entry != (*group)->end; ++entry) {
            BYTE* client = *entry;
            bool timedOut = false;
            if (ServiceClientFieldPilot<DWORD>(client, 0x18) == 0 &&
                ServiceClientFieldPilot<DWORD>(client, 0xc) == 1) {
                DWORD seconds = 0;
                int size = sizeof(seconds);
                if (getsockopt(ServiceClientFieldPilot<SOCKET>(client, 8), SOL_SOCKET,
                    connectTimeOption, reinterpret_cast<char*>(&seconds), &size) == 0 &&
                    seconds != 0xffffffff && seconds > 3) {
                    typedef void (__thiscall *StopClientPilot)(void*, int, DWORD);
                    GraphicsClipSlotPilot<StopClientPilot>(client, 8)(client, 7, 0xc000013f);
                    timedOut = true;
                }
            }
            LONG& report = ServiceClientFieldPilot<LONG>(client, 0x1d8);
            if (report != 0 || timedOut) {
                key = ServiceClientFieldPilot<DWORD>(client, 4);
                status = InterlockedExchange(&report, 0);
                reason = ServiceClientFieldPilot<DWORD>(client, 0x14);
                return true;
            }
        }
    }
    return false;
}

// 200f7a62. Connection state is polled by application callbacks.
static void DispatchServiceConnectionStatusPilot(bool force)
{
    DWORD key;
    LONG status;
    DWORD reason;
    while (ReadServiceConnectionStatusPilot(force, key, status, reason)) {
        if (g_servicePacketLogLevelPilot < 4) {
            const char* format = "I/O Error! (%d, %08X)";
            if (status == 4 || status == 5) format = "Disconnected! (%d, %08X)";
            else if (status == 7) format = "Inactive! (%d, %08X)";
            LogServicePacketSourcePilot(key, 0, format, status, reason);
        }
        RemoveServiceConnectionRegistrationBoundaryPilot(key);
        BYTE* client = FindServiceTransportClientPilot(static_cast<LONG>(key));
        if (client != NULL) GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(client, 4)(client);
    }
}

extern "C" void __cdecl MainServiceCallback7b7bPilot(ULONG_PTR, ULONG_PTR)
{
    DispatchServiceConnectionStatusPilot((g_serviceConnectionPollCounterPilot & 3) == 0);
    ++g_serviceConnectionPollCounterPilot;
}

extern "C" void __cdecl MainServiceCallback7b95Pilot(ULONG_PTR, ULONG_PTR)
{
    DispatchServiceConnectionStatusPilot(false);
}

extern "C" void __cdecl MainServiceCallback7b9fPilot(ULONG_PTR, ULONG_PTR)
{
    DispatchServiceConnectionStatusPilot(true);
}
