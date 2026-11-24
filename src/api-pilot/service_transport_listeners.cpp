void SetServiceListenerRegistryStateBoundaryPilot(const char* name, DWORD port, BYTE* published)
{
    HKEY key;
    if (RegCreateKeyExA(HKEY_CURRENT_USER,
        "SOFTWARE\\Microsoft\\Microsoft Games\\Flight Simulator", 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS) return;
    char value[16];
    int length = _snprintf(value, sizeof(value), "%d", static_cast<int>(port));
    if (length > 0 && RegSetValueExA(key, name, 0, REG_SZ,
        reinterpret_cast<const BYTE*>(value), length) == ERROR_SUCCESS && published != NULL)
        *published = 1;
    RegCloseKey(key);
}

// 20116580 owns the outer vector append. This source uses the same private heap.
static bool AppendServiceClientGroupPilot(ServiceClientGroupObservedPilot* group)
{
    ServiceClientGroupsObservedPilot* groups = g_serviceClientGroupsPilot;
    if (groups == NULL) return false;
    if (groups->end == groups->capacity) {
        DWORD count = groups->begin == NULL ? 0 : static_cast<DWORD>(groups->end - groups->begin);
        if (count >= 0x3fffffffu) return false;
        DWORD capacity = count + 1;
        if (count <= 0x3fffffffu - (count >> 1) && count + (count >> 1) > capacity)
            capacity = count + (count >> 1);
        ServiceClientGroupObservedPilot** storage = static_cast<ServiceClientGroupObservedPilot**>(
            AllocateServiceClientMemoryPilot(0, 0, capacity * sizeof(*storage)));
        if (storage == NULL) return false;
        if (count != 0) memcpy(storage, groups->begin, count * sizeof(*storage));
        FreeServiceClientMemoryPilot(0, 0, groups->begin);
        groups->begin = storage;
        groups->end = storage + count;
        groups->capacity = storage + capacity;
    }
    *groups->end++ = group;
    return true;
}

static DWORD StartServiceListenerClientsPilot(const ServiceListenerConfigurationObservedPilot* configuration,
    SOCKET socket, int family, DWORD count, bool pipe, char* pipeName, bool* pipeNameAdopted)
{
    if (count > 0x3fffffffu || g_serviceTransportTreePilot == NULL) return 0;
    ServiceClientGroupObservedPilot* group = static_cast<ServiceClientGroupObservedPilot*>(
        AllocateServiceClientMemoryPilot(0, 0, sizeof(ServiceClientGroupObservedPilot)));
    if (group == NULL) return 0;
    group->begin = group->end = group->capacity = NULL;
    if (count != 0) {
        group->begin = static_cast<BYTE**>(AllocateServiceClientMemoryPilot(0, 0, count * sizeof(BYTE*)));
        if (group->begin == NULL) {
            FreeServiceClientMemoryPilot(0, 0, group);
            return 0;
        }
        group->end = group->begin;
        group->capacity = group->begin + count;
        memset(group->begin, 0, count * sizeof(BYTE*));
    }
    if (!AppendServiceClientGroupPilot(group)) {
        FreeServiceClientMemoryPilot(0, 0, group->begin);
        FreeServiceClientMemoryPilot(0, 0, group);
        return 0;
    }

    DWORD active = 0;
    bool globalPipe = pipe && lstrcmpiA(configuration->scope.c_str(), "global") == 0;
    for (DWORD i = 0; i < count; ++i) {
        g_serviceClientAcceptEnabledPilot = 1;
        BYTE* storage = static_cast<BYTE*>(AllocateServiceClientMemoryPilot(0, 0, 0x208));
        if (storage == NULL) break;
        BYTE* client = ConstructServiceListenerClientPilot(storage, pipe, pipeName);
        if (client == NULL) {
            FreeServiceClientMemoryPilot(0, 0, storage);
            break;
        }
        if (pipeNameAdopted != NULL && pipeName != NULL &&
            ServiceClientFieldPilot<char*>(client, 0x1fc) == pipeName)
            *pipeNameAdopted = true;
        if (globalPipe) client[0x1e1] = 1;
        if (!ConfigureServiceListenerClientPilot(client, family,
            configuration->clientOption74, socket, configuration->clientOption78 != 0)) {
            ReleaseServiceClientReferenceSourcePilot(client);
            break;
        }
        *group->end++ = client;
        GraphicsClipSlotPilot<GraphicsClipNotifyPilot>(client, 4)(client);
        if (ServiceClientFieldPilot<DWORD>(client, 0xc) != 0) ++active;
        BYTE** slot = FindOrInsertServiceTransportClientPilot(ServiceClientFieldPilot<LONG>(client, 4));
        if (slot != NULL) *slot = client;
    }
    return active;
}

// 2011b0ca.
static bool ProbeServiceSocketFamilyPilot(int family, const char* loopback)
{
    WSADATA data = {0};
    if (WSAStartup(2, &data) != 0) return false;
    addrinfo hints = {0};
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo* list = NULL;
    bool available = false;
    if (getaddrinfo(loopback, NULL, &hints, &list) == 0 && list != NULL) {
        for (addrinfo* entry = list; entry != NULL; entry = entry->ai_next) {
            if (entry->ai_family == family && entry->ai_socktype == SOCK_STREAM && entry->ai_protocol == IPPROTO_TCP) {
                available = true;
                break;
            }
        }
        freeaddrinfo(list);
    }
    WSACleanup();
    return available;
}

static bool ServiceListenerAddressAllowedPilot(const addrinfo& entry, bool linkLocal, DWORD interfaceIndex)
{
    if (entry.ai_family == AF_INET6) {
        const sockaddr_in6& address = *reinterpret_cast<const sockaddr_in6*>(entry.ai_addr);
        if (interfaceIndex != 0 && interfaceIndex != address.sin6_scope_id) return false;
        const BYTE* bytes = reinterpret_cast<const BYTE*>(&address.sin6_addr);
        return !linkLocal || (bytes[0] == 0xfe && (bytes[1] & 0xc0) == 0x80);
    }
    if (!linkLocal) return true;
    if (entry.ai_family != AF_INET) return false;
    const sockaddr_in& address = *reinterpret_cast<const sockaddr_in*>(entry.ai_addr);
    return (ntohl(address.sin_addr.s_addr) & 0xffff0000u) == 0xa9fe0000u;
}

// 20115314. The original retains each created socket for shutdown, including
// sockets whose bind, listen or completion registration subsequently fails.
static bool OpenServiceSocketEndpointsPilot(const ServiceListenerConfigurationObservedPilot* configuration,
    const char* host, int family, bool linkLocal, DWORD interfaceIndex, int protection, DWORD* port)
{
    const DWORD capacity = sizeof(g_serviceListenerSocketsPilot) / sizeof(SOCKET);
    if (g_serviceListenerSocketCountPilot >= capacity) return false;
    addrinfo hints = {0};
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo* list = NULL;
    if (getaddrinfo(host, configuration->endpoint.c_str(), &hints, &list) != 0 || list == NULL) return false;
    bool listening = false;
    for (addrinfo* entry = list; entry != NULL && g_serviceListenerSocketCountPilot < capacity; entry = entry->ai_next) {
        g_serviceListenerSocketsPilot[g_serviceListenerSocketCountPilot] = INVALID_SOCKET;
        if (!ServiceListenerAddressAllowedPilot(*entry, linkLocal, interfaceIndex)) continue;
        SOCKET listener = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
        if (listener == INVALID_SOCKET) continue;
        g_serviceListenerSocketsPilot[g_serviceListenerSocketCountPilot++] = listener;
        if (entry->ai_family == AF_INET6 && protection != 0)
            setsockopt(listener, IPPROTO_IPV6, 23, reinterpret_cast<const char*>(&protection), sizeof(protection));
        int exclusive = 1;
        setsockopt(listener, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char*>(&exclusive), sizeof(exclusive));
        if (bind(listener, entry->ai_addr, static_cast<int>(entry->ai_addrlen)) != 0) continue;
        sockaddr_in6 local;
        int size = sizeof(local);
        if (getsockname(listener, reinterpret_cast<sockaddr*>(&local), &size) != SOCKET_ERROR && port != NULL)
            *port = ntohs(local.sin6_port);
        if (listen(listener, 0x7fffffff) != 0 ||
            !BindIoCompletionCallback(reinterpret_cast<HANDLE>(listener), ServiceClientIoCompletionSourcePilot, 0))
            continue;
        StartServiceListenerClientsPilot(configuration, listener, entry->ai_family,
            configuration->maxClients, false, NULL, NULL);
        listening = true;
    }
    freeaddrinfo(list);
    return listening;
}

void StartServiceSocketListenersBoundaryPilot(const ServiceListenerConfigurationObservedPilot* configuration)
{
    if (g_serviceWinsockStartedPilot == 0) {
        WSADATA data = {0};
        if (WSAStartup(2, &data) == 0) {
            g_serviceWinsockStartedPilot = 1;
            g_serviceListenerSocketCountPilot = 0;
        }
    }
    bool ipv6 = ProbeServiceSocketFamilyPilot(AF_INET6, "::1");
    bool ipv4 = ProbeServiceSocketFamilyPilot(AF_INET, "127.0.0.1");
    if (configuration->maxClients == 0) return;
    bool automatic = configuration->transport == "Auto";
    int family;
    const char* loopback;
    if (ipv6 && (automatic || configuration->transport == "IPv6")) {
        family = AF_INET6;
        loopback = "::1";
    } else if (ipv4 && (automatic || configuration->transport == "IPv4")) {
        family = AF_INET;
        loopback = "127.0.0.1";
    } else {
        return;
    }

    const char* host = configuration->address.c_str();
    DWORD interfaceIndex = host[0] == '%' ? static_cast<DWORD>(atoi(host + 1)) : 0;
    if (host[0] == '\0' || interfaceIndex != 0) host = NULL;
    DWORD port = static_cast<DWORD>(atoi(configuration->endpoint.c_str()));
    bool listening;
    const std::string& scope = configuration->scope;
    if (scope == "local") {
        host = host == NULL ? loopback : host;
        port = 0;
        listening = OpenServiceSocketEndpointsPilot(configuration, host, family, false, 0,
            family == AF_INET6 ? 30 : 0, &port);
        if (family == AF_INET6)
            SetServiceListenerRegistryStateBoundaryPilot("SimConnect_Port_IPv6", port, &g_servicePublishedIpv6PortPilot);
        else
            SetServiceListenerRegistryStateBoundaryPilot("SimConnect_Port_IPv4", port, &g_servicePublishedIpv4PortPilot);
    } else if (scope == "link-local") {
        const char* linkHost = host == NULL ? "..localmachine" : host;
        listening = OpenServiceSocketEndpointsPilot(configuration, linkHost, family, true,
            family == AF_INET6 ? interfaceIndex : 0, family == AF_INET6 ? 30 : 0, NULL);
        host = family == AF_INET6 ? linkHost : (host == NULL ? loopback : host);
    } else {
        int protection = 0;
        if (family == AF_INET6) {
            if (scope == "global") protection = 20;
            else if (scope == "unrestricted") protection = 10;
            else return;
        }
        listening = OpenServiceSocketEndpointsPilot(configuration, host, family, false, 0, protection, NULL);
    }
    if (listening && g_servicePacketLogLevelPilot < 1)
        LogServicePacketSourcePilot(0, 1, "Server: Scope=%s, Protocol=%s, Address=%s, Port=%u, MaxClients=%u",
            scope.c_str(), configuration->transport.c_str(), host, port, configuration->maxClients);
}

void StartServicePipeListenersBoundaryPilot(const ServiceListenerConfigurationObservedPilot* configuration)
{
    DWORD count = configuration->maxClients < 255 ? configuration->maxClients : 255;
    char* pipeName = NULL;
    if (configuration->endpoint != "0") {
        size_t bytes = configuration->endpoint.size() < 243 ? configuration->endpoint.size() + 13 : 256;
        pipeName = static_cast<char*>(AllocateServiceClientMemoryPilot(0, 0, static_cast<DWORD>(bytes)));
        if (pipeName == NULL) return;
        _snprintf(pipeName, bytes, "\\\\.\\pipe\\%s", configuration->endpoint.c_str());
        pipeName[bytes - 1] = '\0';
    }
    bool pipeNameAdopted = false;
    DWORD active = StartServiceListenerClientsPilot(configuration, INVALID_SOCKET, 0, count, true, pipeName, &pipeNameAdopted);
    if (active != 0 && g_servicePacketLogLevelPilot < 1)
        LogServicePacketSourcePilot(0, 1, "Server: Scope=%s, Protocol=%s, Name=%s, MaxClients=%u",
            configuration->scope.c_str(), configuration->transport.c_str(),
            pipeName == NULL ? "\\\\.\\pipe\\Microsoft Flight Simulator\\SimConnect" : pipeName, active);
    if (!pipeNameAdopted) FreeServiceClientMemoryPilot(0, 0, pipeName);
}
