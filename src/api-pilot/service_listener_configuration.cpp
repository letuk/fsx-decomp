// The property set is owned by simprop.dll. Only the slots used by api.dll
// are modeled here, and each receives the property set on the stack.
struct ServiceListenerPropertySetPilot;
extern "C" HRESULT __stdcall SimPropertySet_ReadFile(const char*, const GUID*, DWORD, DWORD, DWORD,
    const GUID*, ServiceListenerPropertySetPilot**);
extern "C" HRESULT __stdcall SimPropertySet_GetBool(ServiceListenerPropertySetPilot*, const GUID*, DWORD, DWORD, BOOL*);
extern "C" HRESULT __stdcall SimPropertySet_GetText(ServiceListenerPropertySetPilot*, const GUID*, DWORD, DWORD, const char**);
extern "C" HRESULT __stdcall SimPropertySet_GetULong(ServiceListenerPropertySetPilot*, const GUID*, DWORD, DWORD, DWORD*);
extern "C" int __stdcall BuildServiceListenerConfigurationPathBoundaryPilot(
    DWORD selector, char* path, DWORD capacity, const char* subdirectory, const char* filename); // 20133f57

static const GUID g_listenerDocumentPilot = {0xDFD821E6, 0xBB11, 0x41BA, {0x92, 0x7C, 0xE0, 0xE8, 0x1C, 0x00, 0x09, 0x22}};
static const GUID g_listenerSchemaPilot = {0xEE6FD77B, 0x14BB, 0x497C, {0xAE, 0x59, 0x0C, 0xCD, 0xD9, 0x0E, 0xAA, 0x68}};
static const GUID g_listenerDisabledPilot = {0x0882927D, 0x81A3, 0x42C7, {0xAF, 0x2F, 0xD6, 0x9D, 0x73, 0xFE, 0x74, 0x5E}};
static const GUID g_listenerEntryPilot = {0x433A02FE, 0xD648, 0x4DEF, {0x86, 0xC0, 0xC1, 0xF7, 0x23, 0xDB, 0x1B, 0x1C}};
static const GUID g_listenerProtocolPilot = {0x277A4B72, 0x1AFD, 0x4495, {0x9A, 0x46, 0xF9, 0x42, 0x2A, 0x07, 0x28, 0x87}};
static const GUID g_listenerScopePilot = {0x99D5570F, 0x53B7, 0x4E9B, {0xAF, 0xA4, 0x12, 0x6A, 0xC8, 0x09, 0x94, 0xF9}};
static const GUID g_listenerAddressPilot = {0xBEE8483E, 0xBFBD, 0x4EF6, {0x8A, 0xC4, 0x84, 0x26, 0x0F, 0x9E, 0x1F, 0x2E}};
static const GUID g_listenerEndpointPilot = {0xB47ACFC2, 0x4C69, 0x4185, {0x97, 0xB6, 0x09, 0x16, 0x69, 0x46, 0x46, 0x32}};
static const GUID g_listenerMaxClientsPilot = {0x694EE6D3, 0xF587, 0x4B0D, {0xA1, 0xA7, 0x4B, 0x04, 0xCC, 0x3C, 0x94, 0xF4}};
static const GUID g_listenerMaxReceivePilot = {0x9A988B19, 0x23F5, 0x4347, {0x8F, 0xE0, 0x2D, 0x7D, 0x14, 0x9B, 0x4A, 0xF8}};
static const GUID g_listenerNoDelayPilot = {0xBE998207, 0x9FB9, 0x4809, {0x96, 0x37, 0x96, 0x53, 0x76, 0x29, 0xC7, 0x54}};

class ServiceListenerPropertySetOwnerPilot {
public:
    ServiceListenerPropertySetPilot* value;
    ServiceListenerPropertySetOwnerPilot() : value(NULL) {}
    ~ServiceListenerPropertySetOwnerPilot()
    {
        if (value != NULL) {
            typedef ULONG (__stdcall *ReleasePilot)(ServiceListenerPropertySetPilot*);
            GraphicsClipSlotPilot<ReleasePilot>(value, 8)(value);
        }
    }
private:
    ServiceListenerPropertySetOwnerPilot(const ServiceListenerPropertySetOwnerPilot&);
    ServiceListenerPropertySetOwnerPilot& operator=(const ServiceListenerPropertySetOwnerPilot&);
};

static void SetDefaultServiceListenerConfigurationPilot(ServiceListenerConfigurationObservedPilot& configuration,
    const char* protocol)
{
    configuration.transport = protocol;
    configuration.scope = "local";
    configuration.address.clear();
    configuration.endpoint = "0";
    configuration.maxClients = 64;
    configuration.clientOption74 = 0xa080;
    configuration.clientOption78 = 0;
}

// 200fb303. The default pipe call in 200fb4d7 is inline in the original.
static void StartDefaultServiceListenerPilot(unsigned int protocol)
{
    static const char* const protocols[] = {"IPv4", "IPv6", "Pipe"};
    if (protocol >= sizeof(protocols) / sizeof(protocols[0])) return;
    ServiceListenerConfigurationObservedPilot configuration;
    SetDefaultServiceListenerConfigurationPilot(configuration, protocols[protocol]);
    InitializeMainWindowServicesSourcePilot(&configuration);
}

static void ReadServiceListenerTextPilot(ServiceListenerPropertySetPilot* entry,
    const GUID& property, std::string& value)
{
    const char* text = NULL;
    if (SUCCEEDED(SimPropertySet_GetText(entry, &property, 0, 0, &text)) && text != NULL && text[0] != '\0')
        value = text;
}

static void StartConfiguredServiceListenersPilot(ServiceListenerPropertySetPilot* document, bool (&defaults)[3])
{
    BOOL disabled = FALSE;
    if (SUCCEEDED(SimPropertySet_GetBool(document, &g_listenerDisabledPilot, 0, 0, &disabled)) && disabled)
        return;
    typedef int (__stdcall *EntryCountPilot)(ServiceListenerPropertySetPilot*, const GUID*);
    typedef HRESULT (__stdcall *ReadEntryPilot)(ServiceListenerPropertySetPilot*, const GUID*, DWORD,
        ServiceListenerPropertySetPilot**);
    int count = GraphicsClipSlotPilot<EntryCountPilot>(document, 0x38)(document, &g_listenerEntryPilot);
    for (int i = 0; i < count; ++i) {
        ServiceListenerPropertySetOwnerPilot entry;
        if (FAILED(GraphicsClipSlotPilot<ReadEntryPilot>(document, 0x3c)(
            document, &g_listenerEntryPilot, i, &entry.value)) || entry.value == NULL) continue;
        ServiceListenerConfigurationObservedPilot configuration;
        SetDefaultServiceListenerConfigurationPilot(configuration, "Auto");
        disabled = FALSE;
        SimPropertySet_GetBool(entry.value, &g_listenerDisabledPilot, 0, 0, &disabled);
        ReadServiceListenerTextPilot(entry.value, g_listenerProtocolPilot, configuration.transport);
        ReadServiceListenerTextPilot(entry.value, g_listenerScopePilot, configuration.scope);
        ReadServiceListenerTextPilot(entry.value, g_listenerAddressPilot, configuration.address);
        ReadServiceListenerTextPilot(entry.value, g_listenerEndpointPilot, configuration.endpoint);
        if (SUCCEEDED(SimPropertySet_GetULong(entry.value, &g_listenerMaxClientsPilot, 0, 0,
            &configuration.maxClients)) && configuration.maxClients > 1000)
            configuration.maxClients = 1000;
        DWORD maxReceive = 0xa080;
        if (SUCCEEDED(SimPropertySet_GetULong(entry.value, &g_listenerMaxReceivePilot, 0, 0, &maxReceive)) &&
            maxReceive > configuration.clientOption74)
            configuration.clientOption74 = maxReceive < 0xffff ? maxReceive : 0xffff;
        BOOL noDelay = FALSE;
        SimPropertySet_GetBool(entry.value, &g_listenerNoDelayPilot, 0, 0, &noDelay);
        configuration.clientOption78 = noDelay;

        if (configuration.scope == "local" && configuration.endpoint == "0") {
            if (configuration.transport == "IPv4") defaults[0] = false;
            if (configuration.transport == "IPv6") defaults[1] = false;
            if (configuration.transport == "Pipe") defaults[2] = false;
        }
        if (!disabled) InitializeMainWindowServicesSourcePilot(&configuration);
    }
}

// 200fb4d7. A disabled document skips configured entries and retains defaults.
extern "C" void __cdecl ConfigureServiceListenersSourcePilot()
{
    bool defaults[] = {true, true, true};
    char path[MAX_PATH] = {0};
    BuildServiceListenerConfigurationPathBoundaryPilot(0x1a, path, sizeof(path), NULL, "SimConnect.xml");
    unsigned int attributes;
    if (QueryPathAttributesPilot(&attributes, path, 0) == 0 && attributes != 0xffffffffu) {
        ServiceListenerPropertySetOwnerPilot document;
        if (SUCCEEDED(SimPropertySet_ReadFile(path, &g_listenerSchemaPilot, 0, 2, 0,
            &g_listenerDocumentPilot, &document.value)) && document.value != NULL)
            StartConfiguredServiceListenersPilot(document.value, defaults);
    }
    if (defaults[2]) StartDefaultServiceListenerPilot(2);
    if (defaults[1]) StartDefaultServiceListenerPilot(1);
    if (defaults[0]) StartDefaultServiceListenerPilot(0);
    if (g_servicePacketLogLevelPilot < 1) LogServicePacketSourcePilot(0, 7, "\n\n");
}
