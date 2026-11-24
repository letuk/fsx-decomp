// 2020d920. Application registrations own protocol clients. The signed-key
// transport tree at 2020db68 stores references to separate listener clients.
typedef std::map<DWORD, BYTE*> ServicePacketRegistrationsPilot;
static ServicePacketRegistrationsPilot g_servicePacketRegistrationsPilot;

// 2010c714.
extern "C" BYTE** __stdcall FindOrInsertServicePacketClientSlotSourcePilot(const DWORD* key)
{
    return &g_servicePacketRegistrationsPilot[*key];
}

// 200fe54f / 2010c798.
extern "C" const BYTE* __stdcall FindServicePacketClientSourcePilot(DWORD key)
{
    ServicePacketRegistrationsPilot::iterator entry = g_servicePacketRegistrationsPilot.find(key);
    return entry == g_servicePacketRegistrationsPilot.end() ? NULL : entry->second;
}

static void EraseServicePacketRegistrationPilot(ServicePacketRegistrationsPilot::iterator entry)
{
    BYTE* client = entry->second;
    if (client != NULL) {
        DestroyServicePacketClientSourcePilot(client);
        FreeServiceClientMemoryPilot(0, 0, client);
    }
    g_servicePacketRegistrationsPilot.erase(entry);
}

// 200fe4cc.
extern "C" void __stdcall RemoveServiceConnectionRegistrationBoundaryPilot(DWORD key)
{
    ServicePacketRegistrationsPilot::iterator entry = g_servicePacketRegistrationsPilot.find(key);
    if (entry != g_servicePacketRegistrationsPilot.end()) EraseServicePacketRegistrationPilot(entry);
}

// Registration loop in 200fe28e. Remaining object shutdown is delegated.
static void ShutDownServicePacketRegistrationsPilot()
{
    while (!g_servicePacketRegistrationsPilot.empty()) {
        ServicePacketRegistrationsPilot::iterator entry = g_servicePacketRegistrationsPilot.begin();
        DWORD transportKey = entry->second != NULL ?
            ServiceClientFieldPilot<DWORD>(entry->second, 0x2e4) : entry->first;
        EraseServicePacketRegistrationPilot(entry);
        DWORD* response = static_cast<DWORD*>(AllocateServiceClientMemoryPilot(0, 0, 0xc));
        if (response != NULL) {
            response[0] = 0xc;
            response[1] = 4;
            response[2] = 3;
            SendServicePacketBufferSourcePilot(transportKey, response, 0xc);
        }
    }
}

// 200f7ba9 -> 200f77d0. Export callbacks belong to SimConnect.dll.
extern "C" void __cdecl MainServiceCallback7ba9Pilot(ULONG_PTR, ULONG_PTR)
{
    if (g_mainWindowServiceDispatchEnabledPilot == 0) return;
    typedef DWORD (WINAPI *ServiceDispatchExportPilot)(DWORD, DWORD, DWORD);
    for (int i = 0; i < g_serviceDispatchExportsCountPilot; ++i)
        reinterpret_cast<ServiceDispatchExportPilot>(g_serviceDispatchExportsPilot[i])(0, 0xfffffffe, 0xfffffffd);
}

extern "C" void __cdecl InitializeServiceLogBoundaryPilot(); // 20118721
extern "C" void __cdecl InitializeServiceConsoleBoundaryPilot(); // 20117652
extern "C" void __cdecl InitializeServiceObjectsBoundaryPilot(); // 200fdab8

// 200f7bb6. Call the service-window entry on every invocation.
extern "C" void __stdcall InitializeMainWindowServicesSourcePilot(
    const ServiceListenerConfigurationObservedPilot* configuration)
{
    if (g_mainWindowServicesActivePilot == 0) {
        g_mainWindowServicesActivePilot = 1;
        InitializeServiceLogBoundaryPilot();
        InitializeServiceConsoleBoundaryPilot();
        g_serviceQueueDispatchCallbackPilot = DispatchServicePacketSourcePilot;
        RegisterCallbackListOwnerBoundaryPilot(&g_mainServiceCallbacks40Pilot, MainServiceCallback7b9fPilot, 0, 0, 0);
        RegisterCallbackListOwnerBoundaryPilot(&g_mainServiceCallbacks34Pilot, MainServiceCallback7b95Pilot, 0, 0, 0);
        RegisterCallbackListOwnerBoundaryPilot(&g_mainServiceCallbacks4cPilot, MainServiceCallback7b7bPilot, 0, 0, 0);
        RegisterCallbackListOwnerBoundaryPilot(&g_mainServiceCallbacks34Pilot, MainServiceCallback7ba9Pilot, 0, 0, 0);
        RegisterCallbackListOwnerBoundaryPilot(&g_mainServiceCallbacks4cPilot, MainServiceCallback7ba9Pilot, 0, 0, 0);
        InitializeServiceObjectsBoundaryPilot();
    }
    InitializeServiceWindowSourcePilot(configuration);
}
