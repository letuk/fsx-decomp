struct ServicePacketHeaderObservedPilot {
    DWORD length;
    DWORD version;
    DWORD command;
    DWORD sequence;
};
C_ASSERT(sizeof(ServicePacketHeaderObservedPilot) == 0x10);

struct ServicePacketRoutePilot {
    BYTE selector;
    WORD minimum;
    WORD maximum;
    DWORD originalTarget;
};

// Selector and length checks from 200fa39d. Target addresses identify
// the source boundaries and are never used as executable pointers.
static const ServicePacketRoutePilot g_servicePacketRoutesPilot[] = {
    {0x02, 0x10, 0x10, 0},
    {0x03, 0x14, 0x14, 0x200fa2a6},
    {0x04, 0x114, 0x114, 0x200f7f87},
    {0x05, 0x24, 0x24, 0x200f8038},
    {0x06, 0x18, 0x18, 0x200f808c},
    {0x07, 0x1c, 0x1c, 0x200f80c8},
    {0x08, 0x18, 0x18, 0x200f810c},
    {0x09, 0x18, 0x18, 0x200f814a},
    {0x0a, 0x14, 0x14, 0x200f8186},
    {0x0b, 0x1c, 0x1c, 0x200f81bc},
    {0x0c, 0x220, 0x220, 0x200f8201},
    {0x0d, 0x14, 0x14, 0x200f832e},
    {0x0e, 0x30, 0x30, 0x200f8363},
    {0x0f, 0x20, 0x20, 0x200f83d0},
    {0x10, 0x25, 0xa0a5, 0x200f841e},
    {0x11, 0x128, 0x128, 0x200f847d},
    {0x12, 0x18, 0x18, 0x200f8575},
    {0x13, 0x114, 0x114, 0x200f85b1},
    {0x14, 0x14, 0x14, 0x200f8661},
    {0x15, 0x18, 0x18, 0x200f8697},
    {0x16, 0x6e, 0x6e, 0x200f86d3},
    {0x17, 0x114, 0x114, 0x200f8823},
    {0x18, 0x14, 0x14, 0x200f88ce},
    {0x19, 0x20, 0x20, 0x200f8903},
    {0x1a, 0x19, 0x19, 0x200f8967},
    {0x1b, 0x1c, 0x1c, 0x200f8a18},
    {0x1c, 0x125, 0x125, 0x200f8a6e},
    {0x1d, 0x19, 0x19, 0x200f8b96},
    {0x1e, 0x15, 0x7e5, 0x200f8c41},
    {0x1f, 0x18, 0x18, 0x200f8d1a},
    {0x20, 0x110, 0x110, 0x200f8d49},
    {0x21, 0x10, 0x10, 0x200f8ddb},
    {0x22, 0x10, 0x10, 0x200f8e2a},
    {0x23, 0x14, 0x14, 0x200f8e79},
    {0x24, 0x30, 0x30, 0x200f8ea6},
    {0x25, 0x48, 0x48, 0x200f8f37},
    {0x26, 0x14, 0x14, 0x200f9027},
    {0x27, 0x125, 0x125, 0x200f905d},
    {0x28, 0x234, 0x234, 0x200f917c},
    {0x29, 0x158, 0x158, 0x200f92ed},
    {0x2a, 0x14c, 0x14c, 0x200f9404},
    {0x2b, 0x18, 0x18, 0x200f94e5},
    {0x2c, 0x18, 0x18, 0x200f9514},
    {0x2d, 0x11c, 0x11c, 0x200f954e},
    {0x2e, 0x20, 0x20, 0x200f9602},
    {0x2f, 0x20, 0x20, 0x200f9661},
    {0x30, 0x28, 0x28, 0x200f96c0},
    {0x31, 0x118, 0x118, 0x200f973c},
    {0x32, 0x14, 0x14, 0x200f980f},
    {0x33, 0x11c, 0x11c, 0x200f9853},
    {0x34, 0x18, 0x18, 0x200f992f},
    {0x35, 0x114, 0x114, 0x200f997b},
    {0x36, 0x21c, 0x21c, 0x200f9a27},
    {0x37, 0x114, 0x114, 0x200f9b40},
    {0x38, 0x1c, 0x1c, 0x200f9bf5},
    {0x39, 0x20, 0x20, 0x200f9c3b},
    {0x3a, 0x14, 0x14, 0x200f9ce6},
    {0x3b, 0x24, 0x24, 0x200f9d1b},
    {0x3c, 0x25, 0xa0a5, 0x200f9ddd},
    {0x3d, 0x114, 0x114, 0x200f9e37},
    {0x3e, 0x918, 0x918, 0x200f9ed2},
    {0x3f, 0x114, 0x114, 0x200fa101},
    {0x40, 0x21, 0xa0a1, 0x200fa19d},
    {0x41, 0x18, 0x18, 0x200fa1f9},
    {0x42, 0x14, 0x14, 0x200fa235},
    {0x43, 0x18, 0x18, 0x200fa269},
    {0x7d, 0x24, 0x24, 0x200f9c88},
    {0x7f, 0x30, 0x30, 0x200f9d70},
    {0x82, 0x918, 0x918, 0x200f9ed2},
    {0xc1, 0x24, 0x24, 0x200f9c88},
    {0xc3, 0x30, 0x30, 0x200f9d70},
    {0xc6, 0xa1c, 0xa1c, 0x200f9fc0},
};

int g_servicePacketLogLevelPilot = 5; // 201f95a4
DWORD g_servicePacketSequencePilot; // 2020d914
DWORD g_servicePacketClientPilot; // 2020d918

extern "C" const BYTE* __stdcall FindServicePacketClientSourcePilot(DWORD); // 200fe54f
extern "C" void __stdcall SendServicePacketExceptionSourcePilot(DWORD, DWORD, DWORD, DWORD); // 200f7d05
extern "C" void __stdcall OpenServicePacketClientSourcePilot(DWORD, const ServicePacketHeaderObservedPilot*); // 200f7d7a
extern "C" void __cdecl LogServicePacketSourcePilot(DWORD, DWORD, const char*, ...); // 20117c78
extern "C" void __stdcall InvokeServicePacketHandlerSourcePilot(
    DWORD originalTarget, DWORD clientId, const BYTE* client, const ServicePacketHeaderObservedPilot*, DWORD bytes);

static const ServicePacketRoutePilot* FindServicePacketRoutePilot(DWORD selector)
{
    const DWORD count = sizeof(g_servicePacketRoutesPilot) / sizeof(g_servicePacketRoutesPilot[0]);
    if (selector >= 2 && selector <= 0x43) return &g_servicePacketRoutesPilot[selector - 2];
    for (DWORD i = 0x42; i < count; ++i)
        if (g_servicePacketRoutesPilot[i].selector == selector) return &g_servicePacketRoutesPilot[i];
    return NULL;
}

// 200fa39d, API side of the queue callback.
extern "C" void __stdcall DispatchServicePacketSourcePilot(DWORD clientId, void* storage, DWORD bytes)
{
    if (storage == NULL || bytes < sizeof(ServicePacketHeaderObservedPilot)) return;
    const ServicePacketHeaderObservedPilot* packet = static_cast<const ServicePacketHeaderObservedPilot*>(storage);
    if (g_servicePacketLogLevelPilot <= 1)
        LogServicePacketSourcePilot(0, 0x101, "[%2d, %d]", clientId, packet->sequence);
    g_servicePacketSequencePilot = packet->sequence;
    g_servicePacketClientPilot = clientId;

    const BYTE* client = FindServicePacketClientSourcePilot(clientId);
    if (client == NULL) {
        if (packet->command == 0xf0000001u) {
            if (bytes == 0x128) OpenServicePacketClientSourcePilot(clientId, packet);
            else SendServicePacketExceptionSourcePilot(clientId, 2, packet->sequence, 0);
        } else if (g_servicePacketLogLevelPilot < 3) {
            LogServicePacketSourcePilot(0, 7, "  Unrecognized Client: received data ignored. [%2d, %d]",
                clientId, packet->sequence);
        }
        return;
    }
    if (packet->version != *reinterpret_cast<const DWORD*>(client + 0x3c8)) {
        SendServicePacketExceptionSourcePilot(clientId, 5, packet->sequence, 0xef4b);
        if (g_servicePacketLogLevelPilot < 3)
            LogServicePacketSourcePilot(0, 7, "Version Mismatch: (0x%X)", packet->version);
        return;
    }

    const DWORD base = 0xf0000000u;
    if (packet->command < base + 0x44) {
        DWORD command = packet->command + (packet->version - 2) * 0x44;
        while (command >= base) {
            const ServicePacketRoutePilot* route = FindServicePacketRoutePilot(command - base);
            if (route != NULL) {
                if (bytes < route->minimum || bytes > route->maximum) {
                    SendServicePacketExceptionSourcePilot(clientId, 2, g_servicePacketSequencePilot, 0);
                } else {
                    if (route->originalTarget != 0)
                        InvokeServicePacketHandlerSourcePilot(route->originalTarget, clientId, client, packet, bytes);
                    else if (g_servicePacketLogLevelPilot < 2)
                        LogServicePacketSourcePilot(0, 7, "Close: ");
                    return;
                }
            }
            command -= 0x44;
        }
    }
    if (g_servicePacketLogLevelPilot < 3)
        LogServicePacketSourcePilot(0, 7, "  Unrecognized Data Received. [%2d, %d]",
            clientId, g_servicePacketSequencePilot);
}
