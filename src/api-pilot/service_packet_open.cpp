struct ServiceOpenRequestObservedPilot {
    ServicePacketHeaderObservedPilot header;
    char name[0x100];
    DWORD reserved;
    DWORD versionFields[5];
};

struct ServiceOpenResponseObservedPilot {
    DWORD length;
    DWORD version;
    DWORD type;
    char name[0x100];
    DWORD productVersion[4];
    DWORD protocolVersion[4];
    DWORD generation;
    DWORD trailing;
};

struct ServiceExceptionResponseObservedPilot {
    DWORD length;
    DWORD version;
    DWORD type;
    DWORD exception;
    DWORD sequence;
    DWORD index;
};
C_ASSERT(sizeof(ServiceOpenRequestObservedPilot) == 0x128);
C_ASSERT(offsetof(ServiceOpenRequestObservedPilot, versionFields) == 0x114);
C_ASSERT(sizeof(ServiceOpenResponseObservedPilot) == 0x134);
C_ASSERT(offsetof(ServiceOpenResponseObservedPilot, generation) == 0x12c);
C_ASSERT(sizeof(ServiceExceptionResponseObservedPilot) == 0x18);

extern "C" BYTE* __stdcall ConstructServicePacketClientSourcePilot(void*, DWORD); // 200ff057
extern "C" bool __stdcall ConfigureServicePacketClientSourcePilot(
    BYTE*, DWORD, const char*, const DWORD*); // 201003e2
extern "C" void __stdcall DestroyServicePacketClientSourcePilot(BYTE*); // 200ff859
extern "C" BYTE** __stdcall FindOrInsertServicePacketClientSlotSourcePilot(const DWORD*); // 2010c714
extern "C" DWORD __stdcall ReadServicePacketTransportGenerationSourcePilot(DWORD); // 2011639f
extern "C" void __stdcall MarkServicePacketTransportReadySourcePilot(DWORD); // 20116417
extern "C" void __stdcall SendServicePacketBufferSourcePilot(DWORD, void*, DWORD); // 2011628d

// 200f7d05. Sequence and index arrive in EDI and ESI in the original.
extern "C" void __stdcall SendServicePacketExceptionSourcePilot(
    DWORD clientId, DWORD exception, DWORD sequence, DWORD index)
{
    ServiceExceptionResponseObservedPilot* response = static_cast<ServiceExceptionResponseObservedPilot*>(
        AllocateServiceClientMemoryPilot(0, 0, sizeof(ServiceExceptionResponseObservedPilot)));
    if (response == NULL) return;
    response->length = sizeof(*response);
    response->version = 4;
    response->type = 1;
    response->exception = exception;
    response->sequence = sequence;
    response->index = index;
    SendServicePacketBufferSourcePilot(clientId, response, sizeof(*response));
    if (g_servicePacketLogLevelPilot < 4)
        LogServicePacketSourcePilot(clientId, 0x200,
            ">>>>>  EXCEPTION=%d, SendID=%d, Index=%d  <<<<<", exception, sequence, index);
}

// 200f7d7a. The request arrives in ECX in the original.
extern "C" void __stdcall OpenServicePacketClientSourcePilot(
    DWORD clientId, const ServicePacketHeaderObservedPilot* packet)
{
    const ServiceOpenRequestObservedPilot* request = reinterpret_cast<const ServiceOpenRequestObservedPilot*>(packet);
    if (memchr(request->name, '\0', sizeof(request->name)) == NULL || request->reserved != 0) {
        SendServicePacketExceptionSourcePilot(clientId, 2, g_servicePacketSequencePilot, 0x10);
        return;
    }
    if (g_servicePacketLogLevelPilot < 2)
        LogServicePacketSourcePilot(0, 7, "Open: Version=0x%08X  Name=\"%s\"", packet->version, request->name);

    void* memory = AllocateServiceClientMemoryPilot(0, 0, 0x3d0);
    if (memory == NULL) return;
    BYTE* client = ConstructServicePacketClientSourcePilot(memory, clientId);
    if (client == NULL) return;
    if (!ConfigureServicePacketClientSourcePilot(client, packet->version, request->name, request->versionFields)) {
        DestroyServicePacketClientSourcePilot(client);
        HeapFree(g_privateHeapPilot, 0, client);
        return;
    }
    *FindOrInsertServicePacketClientSlotSourcePilot(&clientId) = client;

    ServiceOpenResponseObservedPilot* response = static_cast<ServiceOpenResponseObservedPilot*>(
        AllocateServiceClientMemoryPilot(0, 0, sizeof(ServiceOpenResponseObservedPilot)));
    if (response != NULL) {
        memset(response, 0, sizeof(*response));
        response->length = sizeof(*response);
        response->version = 4;
        response->type = 2;
        const char product[] = "Microsoft Flight Simulator X";
        memcpy(response->name, product, sizeof(product));
        response->productVersion[0] = 10;
        response->productVersion[2] = 0xf0c5;
        response->protocolVersion[0] = 10;
        response->protocolVersion[2] = 0xef4b;
        response->generation = ReadServicePacketTransportGenerationSourcePilot(clientId);
        SendServicePacketBufferSourcePilot(clientId, response, sizeof(*response));
    }
    MarkServicePacketTransportReadySourcePilot(clientId);
}
