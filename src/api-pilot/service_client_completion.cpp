extern "C" BOOL __stdcall ContinueServiceClientWriteSourcePilot(BYTE*, DWORD); // 2011ac24
extern "C" int __stdcall InspectServiceClientReceiveBufferSourcePilot(BYTE*, void*); // 2011aa7f
extern "C" void __stdcall ContinueServiceClientReadSourcePilot(BYTE*); // 2011ad8b

// 2011a782. Its embedded queue is drained by 2011a4eb.
static void DestroyServiceClientIoStoragePilot(BYTE* client)
{
    ServiceQueueObservedPilot* queue = &ServiceClientFieldPilot<ServiceQueueObservedPilot>(client, 0x20);
    FreeServiceQueueChainPilot(DetachServiceQueuePilot(queue));
    void* receiveBuffer = ServiceClientFieldPilot<void*>(client, 0x3c);
    if (receiveBuffer != NULL) HeapFree(g_privateHeapPilot, 0, receiveBuffer);
    BYTE* message = ServiceClientFieldPilot<BYTE*>(client, 0x16c);
    if (message != NULL) {
        void* data = ServiceClientFieldPilot<void*>(message, 8);
        if (data != NULL) HeapFree(g_privateHeapPilot, 0, data);
        HeapFree(g_privateHeapPilot, 0, message);
    }
    ServiceClientFieldPilot<void*>(client, 0x44) = NULL;
    ServiceClientFieldPilot<void*>(client, 0x48) = NULL;
    if (client[0x4c] != 0) {
        DeleteCriticalSection(&ServiceClientFieldPilot<CRITICAL_SECTION>(client, 0x50));
        client[0x4c] = 0;
    }
}

// 20114b46. The original client arrives in EAX and the bool returns in AL.
extern "C" bool __stdcall ReleaseServiceClientReferenceSourcePilot(BYTE* client)
{
    if (InterlockedDecrement(&ServiceClientFieldPilot<LONG>(client, 0x1d4)) != 0) return false;
    DestroyServiceClientIoStoragePilot(client);
    GraphicsClipSlotPilot<GraphicsClipWordPilot>(client, 0)(client, 1);
    return true;
}

static void ReportServiceClientCompletionPilot(BYTE* client, DWORD reason, DWORD status)
{
    typedef void (__thiscall *ClientErrorPilot)(void*, DWORD, DWORD);
    GraphicsClipSlotPilot<ClientErrorPilot>(client, 8)(client, reason, status);
}

static void CompleteServiceClientReadPilot(BYTE* client, DWORD transferred)
{
    if (ServiceClientFieldPilot<DWORD>(client, 0xc) == 1)
        GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(client, 0x10)(client);
    ServiceClientFieldPilot<DWORD>(client, 0x44) += transferred;
    ServiceClientFieldPilot<DWORD>(client, 0x1c) = transferred;
    DWORD& firstTransfer = ServiceClientFieldPilot<DWORD>(client, 0x1dc);
    if (firstTransfer == 0xffffffffu) firstTransfer = transferred;

    void*& message = ServiceClientFieldPilot<void*>(client, 0x16c);
    int result = InspectServiceClientReceiveBufferSourcePilot(client, message);
    while (result == 0) {
        typedef void (__thiscall *DeliverServiceMessagePilot)(void*, void*);
        GraphicsClipSlotPilot<DeliverServiceMessagePilot>(client, 0xc)(client, message);
        result = InspectServiceClientReceiveBufferSourcePilot(client, message);
    }
    switch (result) {
    case 2: ReportServiceClientCompletionPilot(client, 1, 0x80004004u); break;
    case 3: ReportServiceClientCompletionPilot(client, 2, 0x80004004u); break;
    case 4: ReportServiceClientCompletionPilot(client, 3, 0x8007000eu); break;
    default: ContinueServiceClientReadSourcePilot(client); break;
    }
}

// 2011ae51. The native OVERLAPPED record is followed by the client and operation byte.
extern "C" void CALLBACK ServiceClientIoCompletionSourcePilot(DWORD status, DWORD transferred, OVERLAPPED* operation)
{
    if (operation == NULL) return;
    BYTE* storage = reinterpret_cast<BYTE*>(operation);
    BYTE* client = ServiceClientFieldPilot<BYTE*>(storage, 0x14);
    if (status != 0 && (status != 0x80000005u || ServiceClientFieldPilot<DWORD>(client, 0x18) != 1)) {
        DWORD reason = 9;
        switch (status) {
        case 0xc0000120u: reason = 8; break;
        case 0xc000013cu: case 0xc000014bu: case 0xc000020du: reason = 5; break;
        }
        ReportServiceClientCompletionPilot(client, reason, status);
    } else if (transferred == 0) {
        ReportServiceClientCompletionPilot(client, 4, 0xc000013eu);
    } else if (storage[0x18] == 0) {
        CompleteServiceClientReadPilot(client, transferred);
    } else if (ServiceClientFieldPilot<DWORD>(client, 0xc) == 2) {
        CRITICAL_SECTION* lock = &ServiceClientFieldPilot<CRITICAL_SECTION>(client, 0x50);
        if (client[0x4c] != 0) EnterCriticalSection(lock);
        ContinueServiceClientWriteSourcePilot(client, 1);
        if (client[0x4c] != 0) LeaveCriticalSection(lock);
    }
    ReleaseServiceClientReferenceSourcePilot(client);
}
