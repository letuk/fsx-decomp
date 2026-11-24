template <class Entry>
struct ServiceTransportVectorObservedPilot {
    DWORD unknown00;
    Entry* begin;
    Entry* end;
    Entry* capacity;
};

typedef ServiceTransportVectorObservedPilot<BYTE*> ServiceClientGroupObservedPilot;
typedef ServiceTransportVectorObservedPilot<ServiceClientGroupObservedPilot*> ServiceClientGroupsObservedPilot;
C_ASSERT(sizeof(ServiceClientGroupsObservedPilot) == 0x10);
C_ASSERT(offsetof(ServiceClientGroupsObservedPilot, begin) == 4);
C_ASSERT(sizeof(ServiceTransportTreeObservedPilot) == 0xc);
C_ASSERT(sizeof(ServiceTransportTreeNodeObservedPilot) == 0x18);

struct ServiceListenerConfigurationObservedPilot {
    std::string transport;
    std::string scope;
    std::string address;
    DWORD maxClients;
    std::string endpoint;
    DWORD clientOption74;
    DWORD clientOption78;
};
C_ASSERT(sizeof(std::string) == 0x1c);

BYTE g_serviceClientRegistrationDisabledPilot; // 2020da5c
BYTE g_serviceClientAcceptEnabledPilot; // 2020d93b
ServiceTransportTreeObservedPilot* g_serviceTransportTreePilot; // 2020db68
static BYTE g_serviceTransportInitializedPilot; // 2020da5d
static ServiceClientGroupsObservedPilot* g_serviceClientGroupsPilot; // 2020da64

BYTE g_serviceWinsockStartedPilot; // 2020da5e
BYTE g_servicePublishedIpv4PortPilot; // 2020da5f
BYTE g_servicePublishedIpv6PortPilot; // 2020db6c
DWORD g_serviceListenerSocketCountPilot; // 2020da60
SOCKET g_serviceListenerSocketsPilot[64]; // 2020da68
static volatile LONG g_serviceLiveTransportClientsPilot; // 2020de24
static int g_serviceTransportAuxiliaryCountPilot; // 201f9810
static void** g_serviceTransportAuxiliaryEntriesPilot; // 201f981c

extern void StartServiceSocketListenersBoundaryPilot(const ServiceListenerConfigurationObservedPilot*); // 20115689
extern void StartServicePipeListenersBoundaryPilot(const ServiceListenerConfigurationObservedPilot*); // 20115ad8
extern void SetServiceListenerRegistryStateBoundaryPilot(const char* name, DWORD port, BYTE* published); // 2011523c

// 20110614 and 20115d99–20115db6.
static ServiceTransportTreeObservedPilot* CreateServiceTransportTreePilot()
{
    ServiceTransportTreeObservedPilot* tree = static_cast<ServiceTransportTreeObservedPilot*>(
        AllocateServiceClientMemoryPilot(0, 0, sizeof(ServiceTransportTreeObservedPilot)));
    if (tree == NULL) return NULL;
    ServiceTransportTreeNodeObservedPilot* head = static_cast<ServiceTransportTreeNodeObservedPilot*>(
        AllocateServiceClientMemoryPilot(0, 0, sizeof(ServiceTransportTreeNodeObservedPilot)));
    if (head == NULL) {
        FreeServiceClientMemoryPilot(0, 0, tree);
        return NULL;
    }
    head->left = head->parent = head->right = head;
    head->color = head->sentinel = 1;
    tree->head = head;
    tree->count = 0;
    return tree;
}

// 20115d07. The original takes the configuration in ECX.
extern "C" void __stdcall InitializeServiceClientsSourcePilot(
    const ServiceListenerConfigurationObservedPilot* configuration)
{
    if (g_serviceTransportInitializedPilot == 0) {
        g_serviceTransportInitializedPilot = 1;
        g_serviceClientGroupsPilot = static_cast<ServiceClientGroupsObservedPilot*>(
            AllocateServiceClientMemoryPilot(0, 0, sizeof(ServiceClientGroupsObservedPilot)));
        if (g_serviceClientGroupsPilot != NULL) {
            g_serviceClientGroupsPilot->begin = NULL;
            g_serviceClientGroupsPilot->end = NULL;
            g_serviceClientGroupsPilot->capacity = NULL;
        }
        g_serviceTransportTreePilot = CreateServiceTransportTreePilot();
        int disabled = g_serviceClientRegistrationDisabledPilot != 0;
        ReadSelectedConfigurationIntPilot("SimConnect", "InprocDisable", &disabled, NULL, 0);
        g_serviceClientRegistrationDisabledPilot = disabled != 0;
    }
    if (configuration->transport == "Pipe")
        StartServicePipeListenersBoundaryPilot(configuration);
    else
        StartServiceSocketListenersBoundaryPilot(configuration);
}

// 201112ae. The tree indexes clients owned by the group vectors.
static void FreeServiceTransportSubtreePilot(ServiceTransportTreeNodeObservedPilot* node)
{
    while (node->sentinel == 0) {
        FreeServiceTransportSubtreePilot(node->right);
        ServiceTransportTreeNodeObservedPilot* left = node->left;
        FreeServiceClientMemoryPilot(0, 0, node);
        node = left;
    }
}

static void ClearServicePublishedPortPilot(BYTE& published, const char* name)
{
    if (published == 0) return;
    SetServiceListenerRegistryStateBoundaryPilot(name, 0, NULL);
    published = 0;
}

static void ReleaseServiceTransportClientsPilot(ServiceClientGroupsObservedPilot* groups)
{
    if (groups == NULL) return;
    g_serviceClientAcceptEnabledPilot = 0;
    typedef void (__thiscall *ClientShutdownPilot)(void*, DWORD, DWORD);
    for (ServiceClientGroupObservedPilot** entry = groups->begin; entry != groups->end; ++entry) {
        ServiceClientGroupObservedPilot* group = *entry;
        for (BYTE** client = group->begin; client != group->end; ++client) {
            GraphicsClipSlotPilot<ClientShutdownPilot>(*client, 8)(*client, 0, 0);
            ReleaseServiceClientReferenceSourcePilot(*client);
        }
        FreeServiceClientMemoryPilot(0, 0, group->begin);
        FreeServiceClientMemoryPilot(0, 0, group);
    }
}

// 20115e3a. Client shutdown precedes socket closure and worker quiescence.
extern "C" void __cdecl ShutDownServiceClientsSourcePilot()
{
    ClearServicePublishedPortPilot(g_servicePublishedIpv4PortPilot, "SimConnect_Port_IPv4");
    ClearServicePublishedPortPilot(g_servicePublishedIpv6PortPilot, "SimConnect_Port_IPv6");

    ServiceTransportTreeObservedPilot* tree = g_serviceTransportTreePilot;
    if (tree != NULL) {
        FreeServiceTransportSubtreePilot(tree->head->parent);
        FreeServiceClientMemoryPilot(0, 0, tree->head);
        FreeServiceClientMemoryPilot(0, 0, tree);
    }
    g_serviceTransportTreePilot = NULL;

    ReleaseServiceTransportClientsPilot(g_serviceClientGroupsPilot);
    for (DWORD i = 0; i < g_serviceListenerSocketCountPilot; ++i) {
        if (g_serviceListenerSocketsPilot[i] != INVALID_SOCKET)
            closesocket(g_serviceListenerSocketsPilot[i]);
    }
    for (int i = 0; i < 20 && g_serviceLiveTransportClientsPilot != 0; ++i)
        Sleep(250);
    if (g_serviceWinsockStartedPilot != 0) {
        WSACleanup();
        g_serviceWinsockStartedPilot = 0;
    }
    if (g_serviceClientGroupsPilot != NULL) {
        FreeServiceClientMemoryPilot(0, 0, g_serviceClientGroupsPilot->begin);
        FreeServiceClientMemoryPilot(0, 0, g_serviceClientGroupsPilot);
    }
    g_serviceClientGroupsPilot = NULL;
    for (int i = 0; i < g_serviceTransportAuxiliaryCountPilot; ++i) {
        FreeServiceClientMemoryPilot(0, 0, g_serviceTransportAuxiliaryEntriesPilot[i]);
        g_serviceTransportAuxiliaryEntriesPilot[i] = NULL;
    }
}
