struct ServiceQueueNodeObservedPilot {
    ServiceQueueNodeObservedPilot* next;
    void* payload;
    DWORD payloadBytes;
    DWORD generation;
    DWORD unknown10;
    const BYTE* client;
    DWORD dispatchArgument;
    DWORD timestampLow;
    DWORD timestampHigh;
};

struct ServiceQueueObservedPilot {
    CRITICAL_SECTION* lock;
    ServiceQueueNodeObservedPilot* head;
    ServiceQueueNodeObservedPilot* tail;
    DWORD count;
    DWORD bytes;
};
C_ASSERT(sizeof(ServiceQueueObservedPilot) == 0x14);
C_ASSERT(offsetof(ServiceQueueNodeObservedPilot, payload) == 4);
C_ASSERT(offsetof(ServiceQueueNodeObservedPilot, timestampLow) == 0x1c);

ServiceQueueObservedPilot* g_serviceWindowQueuePilot; // 2020d900
BYTE g_serviceWindowInitializedPilot; // 2020d908
ATOM g_serviceWindowClassPilot; // 2020d90c
HWND g_serviceWindowPilot; // 2020d910
extern ServiceQueueObservedPilot g_serviceDispatchQueuePilot; // 2020e150
extern "C" void __stdcall ServiceWindowCallbackSourcePilot(ULONG_PTR, ULONG_PTR); // 200f724b
extern "C" void __cdecl ShutDownServiceClientsSourcePilot(); // 20115e3a

static ServiceQueueNodeObservedPilot* DetachServiceQueuePilot(ServiceQueueObservedPilot* queue)
{
    EnterCriticalSection(queue->lock);
    ServiceQueueNodeObservedPilot* head = queue->head;
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    queue->bytes = 0;
    LeaveCriticalSection(queue->lock);
    return head;
}

// 200f7793. The caller owns the detached chain.
extern "C" ServiceQueueNodeObservedPilot* __cdecl DetachServiceWindowQueuePilot()
{
    ServiceQueueObservedPilot* queue = g_serviceWindowQueuePilot;
    return queue != NULL ? DetachServiceQueuePilot(queue) : NULL;
}

static void FreeServiceQueueChainPilot(ServiceQueueNodeObservedPilot* node)
{
    while (node != NULL) {
        ServiceQueueNodeObservedPilot* next = node->next;
        if (node->payload != NULL) HeapFree(g_privateHeapPilot, 0, node->payload);
        HeapFree(g_privateHeapPilot, 0, node);
        node = next;
    }
}

typedef void (__stdcall *ServiceQueueDispatchCallbackPilot)(DWORD, void*, DWORD);
ServiceQueueDispatchCallbackPilot g_serviceQueueDispatchCallbackPilot; // 2020d904
DWORD g_serviceDispatchTimestampLowPilot, g_serviceDispatchTimestampHighPilot; // 2020bbe0, 2020bbe4
static BYTE g_serviceWindowDispatchingPilot; // 2020e2c1
static BYTE g_mainWindowServiceDispatchingPilot; // 2020e2c0
BYTE g_mainWindowServiceDispatchEnabledPilot; // 2020d909
struct ServiceListenerConfigurationObservedPilot;
extern "C" void __stdcall InitializeServiceClientsSourcePilot(
    const ServiceListenerConfigurationObservedPilot* configuration); // 20115d07

// 200f7107.
static void AppendServiceQueueNodePilot(ServiceQueueObservedPilot* queue, ServiceQueueNodeObservedPilot* node)
{
    EnterCriticalSection(queue->lock);
    if (queue->head == NULL) queue->head = node;
    else queue->tail->next = node;
    queue->tail = node;
    ++queue->count;
    queue->bytes += node->payloadBytes;
    LeaveCriticalSection(queue->lock);
}

// 200f76d7. EAX carries the node in the original.
extern "C" void __stdcall RouteServiceQueueNodePilot(ServiceQueueNodeObservedPilot* node)
{
    if (g_serviceWindowQueuePilot == NULL) return;
    if (node->payloadBytes > 0xf) {
        DWORD kind = static_cast<const DWORD*>(node->payload)[2];
        if (kind == 0xf0000003) {
            LARGE_INTEGER timestamp;
            if (QueryPerformanceCounter(&timestamp)) {
                node->timestampLow = timestamp.LowPart;
                node->timestampHigh = timestamp.HighPart;
            }
        } else if (kind == 0xf000000b && node->payloadBytes == 0x1c && g_serviceWindowPilot != NULL) {
            AppendServiceQueueNodePilot(&g_serviceDispatchQueuePilot, node);
            PostMessageA(g_serviceWindowPilot, WM_USER, 0, 0);
            return;
        }
    }
    AppendServiceQueueNodePilot(g_serviceWindowQueuePilot, node);
    PostMessageA(g_mainWindowHandlePilot, WM_COMMAND, 0x2002a, 0);
}

// 200f71c2.
extern "C" void __stdcall DispatchDetachedServiceQueuePilot(ServiceQueueNodeObservedPilot* node)
{
    while (node != NULL) {
        g_serviceDispatchTimestampLowPilot = node->timestampLow;
        g_serviceDispatchTimestampHighPilot = node->timestampHigh;
        if (static_cast<LONG>(node->payloadBytes) > 0) {
            DWORD size = *static_cast<const DWORD*>(node->payload);
            if (size >= 0x10 && g_serviceQueueDispatchCallbackPilot != NULL) {
                DWORD generation = node->client != NULL ?
                    *reinterpret_cast<const DWORD*>(node->client + 0x1cc) : 0;
                if (generation == node->generation)
                    g_serviceQueueDispatchCallbackPilot(node->dispatchArgument, node->payload, size);
            }
        }
        ServiceQueueNodeObservedPilot* next = node->next;
        if (node->payload != NULL) HeapFree(g_privateHeapPilot, 0, node->payload);
        HeapFree(g_privateHeapPilot, 0, node);
        node = next;
    }
}

// 200f7426.
extern "C" LRESULT CALLBACK ServiceWindowProcedurePilot(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message != WM_USER) return DefWindowProcA(hwnd, message, wParam, lParam);
    if (g_serviceWindowDispatchingPilot == 0) {
        g_serviceWindowDispatchingPilot = 1;
        DispatchDetachedServiceQueuePilot(DetachServiceQueuePilot(&g_serviceDispatchQueuePilot));
        g_serviceWindowDispatchingPilot = 0;
    }
    return 0;
}

extern int g_serviceDispatchExportsCountPilot; // 201f96b8
extern FARPROC* g_serviceDispatchExportsPilot; // 201f96c4
extern BYTE g_serviceClientRegistrationDisabledPilot; // 2020da5c
extern "C" void __stdcall AppendServiceDispatchExportSourcePilot(const FARPROC*); // 200f7829
extern "C" void __stdcall SetServiceClientRegistrationSourcePilot(DWORD, DWORD, DWORD); // 201164e2
extern "C" void* __stdcall AllocateServiceClientMemoryPilot(DWORD, DWORD, DWORD); // 200f7137
extern "C" void __stdcall FreeServiceClientMemoryPilot(DWORD, DWORD, void*); // 200f7158
extern "C" void __stdcall DispatchServiceClientDataSourcePilot(DWORD, DWORD, DWORD, DWORD); // 200f7177

struct ServiceClientRegistrationObservedPilot {
    DWORD size;
    DWORD version;
    DWORD client;
    DWORD context;
    void* (__stdcall *allocate)(DWORD, DWORD, DWORD);
    void (__stdcall *release)(DWORD, DWORD, void*);
    void (__stdcall *dispatch)(DWORD, DWORD, DWORD, DWORD);
    void (__stdcall *releaseAlternate)(DWORD, DWORD, void*);
};
C_ASSERT(sizeof(ServiceClientRegistrationObservedPilot) == 0x20);

static void RegisterServiceClientDescriptorPilot(const DWORD* descriptor)
{
    if (g_serviceClientRegistrationDisabledPilot != 0 ||
        descriptor[0] != 0x18 || descriptor[1] != 1 ||
        descriptor[2] == 0 || descriptor[3] == 0 || descriptor[4] == 0 || descriptor[5] == 0)
        return;
    ServiceClientRegistrationObservedPilot registration = {
        0x20, 2, descriptor[2], descriptor[3], AllocateServiceClientMemoryPilot,
        FreeServiceClientMemoryPilot, DispatchServiceClientDataSourcePilot, FreeServiceClientMemoryPilot
    };
    typedef BOOL (__stdcall *AcceptRegistrationPilot)(ServiceClientRegistrationObservedPilot*);
    if (reinterpret_cast<AcceptRegistrationPilot>(descriptor[4])(&registration))
        SetServiceClientRegistrationSourcePilot(descriptor[2], descriptor[3], descriptor[5]);
}

// 200f724b. The second argument points to the application's event record.
extern "C" void __stdcall ServiceWindowCallbackSourcePilot(ULONG_PTR event, ULONG_PTR eventRecord)
{
    if (event != 0x2002a) return;
    ULONG_PTR candidate = reinterpret_cast<const ULONG_PTR*>(eventRecord)[3];
    if (candidate == 0) {
        if (g_mainWindowServiceDispatchEnabledPilot != 0 && g_mainWindowServiceDispatchingPilot == 0) {
            g_mainWindowServiceDispatchingPilot = 1;
            DispatchDetachedServiceQueuePilot(DetachServiceWindowQueuePilot());
            g_mainWindowServiceDispatchingPilot = 0;
        }
        return;
    }
    HMODULE module = reinterpret_cast<HMODULE>(candidate);
    char path[MAX_PATH + 1];
    DWORD length = GetModuleFileNameA(module, path, MAX_PATH);
    if (length == 0) {
        RegisterServiceClientDescriptorPilot(reinterpret_cast<const DWORD*>(candidate));
        return;
    }
    path[length] = '\0';
    if (strstr(path, "SimConnect.dll") == NULL) return;
    FARPROC dispatch = GetProcAddress(module, "SimConnect_CallDispatch");
    if (dispatch == NULL) return;
    for (int i = 0; i < g_serviceDispatchExportsCountPilot; ++i) {
        if (g_serviceDispatchExportsPilot[i] == dispatch) return;
    }
    AppendServiceDispatchExportSourcePilot(&dispatch);
}

// 200f7809, API ordinal 323.
extern "C" void __cdecl EnableMainWindowServiceDispatchPilot()
{
    g_mainWindowServiceDispatchEnabledPilot = 1;
    PostMessageA(g_mainWindowHandlePilot, WM_COMMAND, 0x2002a, 0);
}

// 200f74ab and the queue-header constructor at 200f70ba.
extern "C" void __stdcall InitializeServiceWindowSourcePilot(
    const ServiceListenerConfigurationObservedPilot* configuration)
{
    if (g_serviceWindowInitializedPilot == 0) {
        g_serviceWindowInitializedPilot = 1;
        void* memory;
        AllocatePrivateHeapMemoryPilot(1, sizeof(ServiceQueueObservedPilot), &memory, g_factoryZeroInitializeBoundaryPilot);
        ServiceQueueObservedPilot* queue = static_cast<ServiceQueueObservedPilot*>(memory);
        if (queue != NULL) {
            queue->head = queue->tail = NULL;
            queue->count = queue->bytes = 0;
            AllocatePrivateHeapMemoryPilot(1, sizeof(CRITICAL_SECTION), &memory, g_factoryZeroInitializeBoundaryPilot);
            queue->lock = static_cast<CRITICAL_SECTION*>(memory);
            if (queue->lock != NULL) InitializeCriticalSection(queue->lock);
        }
        g_serviceWindowQueuePilot = queue;
        RegisterCallbackListOwnerBoundaryPilot(&g_serviceWindowCallbacksPilot,
            reinterpret_cast<RegisteredCallbackBoundaryPilot>(ServiceWindowCallbackSourcePilot), 0, 0, 0);
        WNDCLASSA windowClass = {0};
        windowClass.lpfnWndProc = ServiceWindowProcedurePilot;
        windowClass.lpszClassName = "CD7CFFA8-06F0-416c-A0C7-9061C125D353.FSX";
        g_serviceWindowClassPilot = RegisterClassA(&windowClass);
        g_serviceWindowPilot = CreateWindowExA(0, MAKEINTATOM(g_serviceWindowClassPilot), "",
            0, 0, 0, 0, 0, reinterpret_cast<HWND>(-3), NULL, NULL, NULL);
    }
    InitializeServiceClientsSourcePilot(configuration);
}

// 200f757e. Producers are shut down before either queue is drained.
extern "C" void __cdecl ShutDownServiceWindowSourcePilot()
{
    ShutDownServiceClientsSourcePilot();
    RemoveMainCallbackListEntryPilot(&g_serviceWindowCallbacksPilot,
        reinterpret_cast<RegisteredCallbackBoundaryPilot>(ServiceWindowCallbackSourcePilot), 0, 0, 0);
    DestroyWindow(g_serviceWindowPilot);
    g_serviceWindowPilot = NULL;
    UnregisterClassA(MAKEINTATOM(g_serviceWindowClassPilot), NULL);
    g_serviceWindowClassPilot = 0;

    FreeServiceQueueChainPilot(DetachServiceWindowQueuePilot());
    FreeServiceQueueChainPilot(DetachServiceQueuePilot(&g_serviceDispatchQueuePilot));
    ServiceQueueObservedPilot* queue = g_serviceWindowQueuePilot;
    if (queue != NULL) {
        if (queue->lock != NULL) {
            DeleteCriticalSection(queue->lock);
            HeapFree(g_privateHeapPilot, 0, queue->lock);
        }
        queue->lock = NULL;
        queue->count = 0;
        queue->bytes = 0;
        HeapFree(g_privateHeapPilot, 0, queue);
    }
    g_serviceWindowInitializedPilot = 0;
    g_serviceWindowQueuePilot = NULL;
}
