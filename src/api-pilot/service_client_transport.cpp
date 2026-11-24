typedef void (CALLBACK *ServicePipeWaitCallbackPilot)(void*, BOOLEAN);
typedef void (CALLBACK *ServiceIoCompletionCallbackPilot)(DWORD, DWORD, OVERLAPPED*);
extern "C" __declspec(dllimport) BOOL WINAPI RegisterWaitForSingleObject(
    PHANDLE, HANDLE, ServicePipeWaitCallbackPilot, void*, ULONG, ULONG);
extern "C" __declspec(dllimport) BOOL WINAPI UnregisterWait(HANDLE);
extern "C" __declspec(dllimport) BOOL WINAPI BindIoCompletionCallback(
    HANDLE, ServiceIoCompletionCallbackPilot, ULONG);

extern CRITICAL_SECTION g_serviceClientIndexLockPilot; // 201f9830
extern BYTE g_serviceClientAcceptEnabledPilot; // 2020d93b
extern "C" void __stdcall ResetServiceClientConnectionSourcePilot(BYTE*); // 2011a7ed
extern "C" void __stdcall CloseServiceClientConnectionSourcePilot(BYTE*, int, DWORD); // 2011a8d4
extern "C" bool __stdcall ReleaseServiceClientReferenceSourcePilot(BYTE*); // 20114b46
extern "C" void CALLBACK ServiceClientIoCompletionSourcePilot(DWORD, DWORD, OVERLAPPED*); // 2011ae51
extern "C" BOOL WINAPI AcceptServiceSocketSourcePilot(
    SOCKET, SOCKET, void*, DWORD, DWORD, DWORD, DWORD*, OVERLAPPED*); // mswsock.dll!AcceptEx
extern "C" BOOL WINAPI ConvertServicePipeSecuritySourcePilot(
    LPCSTR, DWORD, PSECURITY_DESCRIPTOR*, PULONG); // advapi32.dll IAT 200e1ccc

template<class Field>
static Field& ServiceClientFieldPilot(BYTE* client, unsigned int offset)
{
    return *reinterpret_cast<Field*>(client + offset);
}

// 201151b0. The transport completion callback is a separate target.
extern "C" void CALLBACK OnServiceClientPipeReadyPilot(void* context, BOOLEAN)
{
    BYTE* client = static_cast<BYTE*>(context);
    if (ServiceClientFieldPilot<DWORD>(client, 0x18) != 1 ||
        ReleaseServiceClientReferenceSourcePilot(client)) return;
    HANDLE pipe = ServiceClientFieldPilot<HANDLE>(client, 8);
    if (pipe == INVALID_HANDLE_VALUE) return;
    DWORD transferred;
    if (!GetOverlappedResult(pipe, &ServiceClientFieldPilot<OVERLAPPED>(client, 0x1e4), &transferred, FALSE)) {
        GetLastError();
        return;
    }
    if (BindIoCompletionCallback(pipe, ServiceClientIoCompletionSourcePilot, 0)) {
        typedef void (__thiscall *ActivateServiceClientPilot)(void*);
        GraphicsClipSlotPilot<ActivateServiceClientPilot>(client, 0x10)(client);
    }
}

static void StartServiceClientSocketAcceptPilot(BYTE* client)
{
    SOCKET accepted = socket(ServiceClientFieldPilot<int>(client, 0x174), SOCK_STREAM, 0);
    ServiceClientFieldPilot<SOCKET>(client, 8) = accepted;
    DWORD& state = ServiceClientFieldPilot<DWORD>(client, 0xc);
    if (accepted == INVALID_SOCKET) {
        state = 0;
        return;
    }
    if (client[0x178] != 0) {
        int enabled = 1;
        setsockopt(accepted, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&enabled), sizeof(enabled));
    }
    state = 1;
    InterlockedIncrement(&ServiceClientFieldPilot<LONG>(client, 0x1d4));
    if (!AcceptServiceSocketSourcePilot(ServiceClientFieldPilot<SOCKET>(client, 0x170), accepted,
        ServiceClientFieldPilot<void*>(client, 0x3c), ServiceClientFieldPilot<DWORD>(client, 0x168) - 0x120,
        0x90, 0x90, &ServiceClientFieldPilot<DWORD>(client, 0x1c),
        &ServiceClientFieldPilot<OVERLAPPED>(client, 0x17c)) && WSAGetLastError() != ERROR_IO_PENDING) {
        state = 0;
        ServiceClientFieldPilot<SOCKET>(client, 8) = INVALID_SOCKET;
        closesocket(accepted);
        ReleaseServiceClientReferenceSourcePilot(client);
    }
}

static void StartServiceClientPipeAcceptPilot(BYTE* client)
{
    HANDLE event = ServiceClientFieldPilot<HANDLE>(client, 0x1f4);
    ResetEvent(event);
    HANDLE& wait = ServiceClientFieldPilot<HANDLE>(client, 0x1f8);
    if (wait != INVALID_HANDLE_VALUE) {
        HANDLE oldWait = wait;
        wait = INVALID_HANDLE_VALUE;
        if (!UnregisterWait(oldWait)) GetLastError();
    }
    if (!RegisterWaitForSingleObject(&wait, event, OnServiceClientPipeReadyPilot, client, INFINITE, 0x18)) {
        GetLastError();
        return;
    }

    SECURITY_ATTRIBUTES security = {sizeof(security), NULL, FALSE};
    if (client[0x1e1] == 0 &&
        !ConvertServicePipeSecuritySourcePilot(
            "D:P(D;OICI;GA;;;S-1-5-2)(A;OICI;GA;;;SY)(A;OICI;GA;;;CO)(A;OICI;GA;;;BA)(A;OICI;GA;;;IU)",
            1, &security.lpSecurityDescriptor, NULL))
        GetLastError();
    LPCSTR name = ServiceClientFieldPilot<LPCSTR>(client, 0x1fc);
    if (name == NULL) name = "\\\\.\\pipe\\Microsoft Flight Simulator\\SimConnect";
    HANDLE pipe = CreateNamedPipeA(name, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, 0,
        PIPE_UNLIMITED_INSTANCES, 0x2000, 0x2000, 5000, security.lpSecurityDescriptor != NULL ? &security : NULL);
    ServiceClientFieldPilot<HANDLE>(client, 8) = pipe;
    if (security.lpSecurityDescriptor != NULL) LocalFree(security.lpSecurityDescriptor);
    if (pipe == INVALID_HANDLE_VALUE) {
        GetLastError();
        return;
    }

    DWORD& state = ServiceClientFieldPilot<DWORD>(client, 0xc);
    state = 1;
    InterlockedIncrement(&ServiceClientFieldPilot<LONG>(client, 0x1d4));
    if (!ConnectNamedPipe(pipe, &ServiceClientFieldPilot<OVERLAPPED>(client, 0x1e4)) &&
        GetLastError() == ERROR_IO_PENDING) {
        DWORD status = GetLastError();
        if (status == ERROR_PIPE_CONNECTED) {
            if (!SetEvent(event)) GetLastError();
        } else if (status != ERROR_IO_PENDING) {
            GetLastError();
        }
        return;
    }
    state = 0;
    ServiceClientFieldPilot<HANDLE>(client, 8) = INVALID_HANDLE_VALUE;
    CloseHandle(pipe);
    ReleaseServiceClientReferenceSourcePilot(client);
    GetLastError();
}

// 20114ee9. Original receiver in ECX.
extern "C" void __stdcall StartServiceClientAcceptPilot(BYTE* client)
{
    DWORD key = ServiceClientFieldPilot<DWORD>(client, 0x1cc);
    EnterCriticalSection(&g_serviceClientIndexLockPilot);
    RemoveServiceClientIndexEntryPilot(&key);
    LeaveCriticalSection(&g_serviceClientIndexLockPilot);
    ResetServiceClientConnectionSourcePilot(client);
    key = ServiceClientFieldPilot<DWORD>(client, 0x1cc);
    EnterCriticalSection(&g_serviceClientIndexLockPilot);
    bool indexed = InsertServiceClientIndexEntryPilot(key, client);
    LeaveCriticalSection(&g_serviceClientIndexLockPilot);
    client[0x1e0] = 0;
    if (!indexed || g_serviceClientAcceptEnabledPilot == 0) return;

    DWORD transport = ServiceClientFieldPilot<DWORD>(client, 0x18);
    if (transport == 0) StartServiceClientSocketAcceptPilot(client);
    else if (transport == 1) StartServiceClientPipeAcceptPilot(client);
}

// 20114e71. The original receiver is in ECX.
extern "C" void __stdcall StopServiceClientAcceptPilot(BYTE* client, int reportError, DWORD reason)
{
    HANDLE& pipe = ServiceClientFieldPilot<HANDLE>(client, 8);
    if (ServiceClientFieldPilot<DWORD>(client, 0x18) == 1 && pipe != INVALID_HANDLE_VALUE) {
        HANDLE connected = pipe;
        pipe = INVALID_HANDLE_VALUE;
        if (!DisconnectNamedPipe(connected)) GetLastError();
        CloseHandle(connected);
    }
    DWORD key = ServiceClientFieldPilot<DWORD>(client, 0x1cc);
    EnterCriticalSection(&g_serviceClientIndexLockPilot);
    RemoveServiceClientIndexEntryPilot(&key);
    LeaveCriticalSection(&g_serviceClientIndexLockPilot);
    CloseServiceClientConnectionSourcePilot(client, reportError, reason);
}
