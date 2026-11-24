struct ServiceReceivedMessageObservedPilot {
    DWORD length;
    DWORD capacity;
    BYTE* data;
};
C_ASSERT(sizeof(ServiceReceivedMessageObservedPilot) == 0xc);

// 2011ae4b, base table +0x0c. ECX is the receiver, EDX is unused.
extern "C" DWORD __fastcall IgnoreServiceReceivedMessagePilot(
    BYTE*, void*, ServiceReceivedMessageObservedPilot*)
{
    return 0;
}

// 20114cdd, derived table +0x0c. Use the capacity word as in the original.
extern "C" DWORD __fastcall DeliverServiceReceivedMessagePilot(
    BYTE* client, void*, ServiceReceivedMessageObservedPilot* message)
{
    if (message->data == NULL || QueueServiceClientPayloadPilot(client, message->capacity, message->data)) {
        message->length = 0;
        message->capacity = 0;
        message->data = NULL;
    }
    return 0;
}

static BYTE* AllocateServiceBufferPilot(DWORD bytes)
{
    return static_cast<BYTE*>(AllocateServiceClientMemoryPilot(0, 0, bytes));
}

// 2011aa7f. Result zero publishes a message to client slot +0x0c.
extern "C" int __stdcall InspectServiceClientReceiveBufferSourcePilot(BYTE* client, void* storage)
{
    ServiceReceivedMessageObservedPilot* message = static_cast<ServiceReceivedMessageObservedPilot*>(storage);
    BYTE*& base = ServiceClientFieldPilot<BYTE*>(client, 0x3c);
    BYTE*& end = ServiceClientFieldPilot<BYTE*>(client, 0x44);
    BYTE*& cursor = ServiceClientFieldPilot<BYTE*>(client, 0x48);
    DWORD& capacity = ServiceClientFieldPilot<DWORD>(client, 0x168);
    DWORD available = end - cursor;
    int result = 1;
    if (available >= sizeof(DWORD)) {
        DWORD length;
        memcpy(&length, cursor, sizeof(length));
        if (length > ServiceClientFieldPilot<DWORD>(client, 0x1d0)) return 2;
        if (length < sizeof(DWORD)) {
            result = 3;
        } else {
            if (message->capacity < length) {
                BYTE* data = AllocateServiceBufferPilot(length);
                if (data == NULL) return 4;
                FreeServiceClientMemoryPilot(0, 0, message->data);
                message->data = data;
                message->capacity = length;
            }
            message->length = length;
            if (message->data == NULL) return 4;
            if (available >= length) {
                memcpy(message->data, cursor, length);
                cursor += length;
                if (cursor == end) cursor = end = base;
                result = 0;
            }
        }
    }

    available = end - cursor;
    if (result == 0 && available >= sizeof(DWORD)) {
        DWORD nextLength;
        memcpy(&nextLength, cursor, sizeof(nextLength));
        if (available >= nextLength) return result;
    }
    if (static_cast<DWORD>(end - base) < capacity) return result;
    if (cursor > base) {
        memmove(base, cursor, available);
        cursor = base;
        end = base + available;
    } else if (capacity < 0x100000) {
        BYTE* expanded = AllocateServiceBufferPilot(capacity + 0x800);
        if (expanded == NULL) return 4;
        memcpy(expanded, base, capacity);
        FreeServiceClientMemoryPilot(0, 0, base);
        base = cursor = expanded;
        end = expanded + capacity;
        capacity += 0x800;
    } else {
        message->length = 0xffffffffu;
        FreeServiceClientMemoryPilot(0, 0, message->data);
        message->data = NULL;
        return 0;
    }
    return result;
}

// 2011ad8b. Clear the operation fields through OffsetHigh, preserving hEvent.
extern "C" void __stdcall ContinueServiceClientReadSourcePilot(BYTE* client)
{
    DWORD& transferred = ServiceClientFieldPilot<DWORD>(client, 0x1c);
    transferred = 0;
    if (ServiceClientFieldPilot<DWORD>(client, 0xc) != 2) return;
    DWORD transport = ServiceClientFieldPilot<DWORD>(client, 0x18);
    if (transport > 1) return;
    OVERLAPPED& operation = ServiceClientFieldPilot<OVERLAPPED>(client, 0x17c);
    operation.Internal = operation.InternalHigh = operation.Offset = operation.OffsetHigh = 0;
    WSABUF& buffer = ServiceClientFieldPilot<WSABUF>(client, 0x19c);
    buffer.buf = ServiceClientFieldPilot<char*>(client, 0x44);
    buffer.len = ServiceClientFieldPilot<DWORD>(client, 0x168) -
        (buffer.buf - ServiceClientFieldPilot<char*>(client, 0x3c));
    InterlockedIncrement(&ServiceClientFieldPilot<LONG>(client, 0x1d4));
    DWORD error;
    if (transport == 0) {
        DWORD flags = 0;
        if (WSARecv(ServiceClientFieldPilot<SOCKET>(client, 8), &buffer, 1, &transferred, &flags, &operation, NULL)
            != SOCKET_ERROR) return;
        error = WSAGetLastError();
    } else {
        if (ReadFile(ServiceClientFieldPilot<HANDLE>(client, 8), buffer.buf, buffer.len, NULL, &operation)) return;
        error = GetLastError();
    }
    if (error != ERROR_IO_PENDING) ReleaseServiceClientReferenceSourcePilot(client);
}

// 2011ac24. Pop one queued write and transfer its payload to the active slot.
extern "C" BOOL __stdcall ContinueServiceClientWriteSourcePilot(BYTE* client, DWORD completed)
{
    BYTE& writing = client[0x34];
    if (completed == 0 && writing != 0) return FALSE;
    DWORD transport = ServiceClientFieldPilot<DWORD>(client, 0x18);
    if (transport > 1) return FALSE;
    BYTE*& active = ServiceClientFieldPilot<BYTE*>(client, 0x1c0);
    FreeServiceClientMemoryPilot(0, 0, active);
    active = NULL;
    ServiceQueueObservedPilot& queue = ServiceClientFieldPilot<ServiceQueueObservedPilot>(client, 0x20);
    EnterCriticalSection(queue.lock);
    ServiceQueueNodeObservedPilot* node = queue.head;
    if (node == NULL) {
        queue.count = queue.bytes = 0;
    } else {
        queue.head = node->next;
        if (queue.head == NULL) queue.tail = NULL;
        --queue.count;
        queue.bytes -= node->payloadBytes;
    }
    LeaveCriticalSection(queue.lock);
    WSABUF& buffer = ServiceClientFieldPilot<WSABUF>(client, 0x1c4);
    if (node == NULL) {
        if (completed != 0) {
            buffer.buf = NULL;
            writing = 0;
        }
        return FALSE;
    }
    buffer.len = node->payloadBytes;
    buffer.buf = static_cast<char*>(node->payload);
    active = static_cast<BYTE*>(node->payload);
    HeapFree(g_privateHeapPilot, 0, node);
    InterlockedIncrement(&ServiceClientFieldPilot<LONG>(client, 0x1d4));
    OVERLAPPED* operation = &ServiceClientFieldPilot<OVERLAPPED>(client, 0x1a4);
    DWORD error = ERROR_IO_PENDING;
    if (transport == 0) {
        if (WSASend(ServiceClientFieldPilot<SOCKET>(client, 8), &buffer, 1,
            &ServiceClientFieldPilot<DWORD>(client, 0x38), 0, operation, NULL) == SOCKET_ERROR)
            error = WSAGetLastError();
    } else if (!WriteFile(ServiceClientFieldPilot<HANDLE>(client, 8), buffer.buf, buffer.len, NULL, operation)) {
        error = GetLastError();
    }
    writing = 1;
    if (error != ERROR_IO_PENDING) ReleaseServiceClientReferenceSourcePilot(client);
    return TRUE;
}
