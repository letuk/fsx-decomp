#include "callback_registry_boundary.h"
#include <string.h>

extern CallbackListDataBoundaryPilot* g_mainServiceCallbacks34Pilot; // 2020bb34
extern CallbackListDataBoundaryPilot* g_mainServiceCallbacks40Pilot; // 2020bb40
extern CallbackListDataBoundaryPilot* g_mainServiceCallbacks4cPilot; // 2020bb4c
extern CallbackListDataBoundaryPilot* g_serviceWindowCallbacksPilot; // 2020bb50
extern CallbackRegistryOwnerBoundaryPilot g_initializeCallbackRegistryBoundaryPilot; // 2020bb48
extern void* g_messageStartupCallbacksPilot; // 2020bba8
extern CallbackListDataBoundaryPilot* g_applicationCallbackListsPilot[43]; // 2020bb30..2020bbdc
DWORD g_callbackListCompactionPendingPilot; // 2020d8fc
BYTE g_mainWindowServicesActivePilot; // 2020d90a

extern "C" void __cdecl MainServiceCallback7b7bPilot(ULONG_PTR, ULONG_PTR);
extern "C" void __cdecl MainServiceCallback7b95Pilot(ULONG_PTR, ULONG_PTR);
extern "C" void __cdecl MainServiceCallback7b9fPilot(ULONG_PTR, ULONG_PTR);
extern "C" void __cdecl MainServiceCallback7ba9Pilot(ULONG_PTR, ULONG_PTR);
extern "C" void __cdecl ShutDownServiceObjectsSourcePilot(); // 200fdfd4
extern "C" void __cdecl ShutDownServiceWindowSourcePilot(); // 200f757e
extern "C" void __cdecl ShutDownServiceConsoleSourcePilot(); // 201179b0
static void ShutDownServicePacketRegistrationsPilot(); // registration portion of 200fe28e

static void EraseMainCallbackListEntryPilot(CallbackListDataBoundaryPilot* list, DWORD index)
{
    CallbackListEntryBoundaryPilot* entries = reinterpret_cast<CallbackListEntryBoundaryPilot*>(list + 1);
    DWORD remaining = list->count - index - 1;
    if (remaining != 0) memmove(&entries[index], &entries[index + 1], remaining * sizeof(*entries));
    memset(&entries[--list->count], 0, sizeof(*entries));
}

// 200f65b1. The original list owner arrives in EAX.
extern "C" int __stdcall RemoveMainCallbackListEntryPilot(CallbackListDataBoundaryPilot** owner,
    RegisteredCallbackBoundaryPilot callback, int matchContext, ULONG_PTR context1, ULONG_PTR context2)
{
    CallbackListDataBoundaryPilot* list = *owner;
    if (list == NULL) return 0;
    CallbackListEntryBoundaryPilot* entries = reinterpret_cast<CallbackListEntryBoundaryPilot*>(list + 1);
    for (DWORD i = 0; i < list->count; ++i) {
        CallbackListEntryBoundaryPilot& entry = entries[i];
        if (entry.callback != callback || (entry.flags & 1) != 0 ||
            (matchContext != 0 && (entry.context1 != context1 || entry.context2 != context2)))
            continue;
        if (list->dispatchDepth == 0) {
            EraseMainCallbackListEntryPilot(list, i);
        } else {
            entry.flags |= 1;
            list->pendingCompaction = 1;
            g_callbackListCompactionPendingPilot = 1;
        }
        return 0;
    }
    return -1;
}

static CallbackListDataBoundaryPilot* ReadApplicationCallbackListPilot(DWORD index)
{
    // These slots already have named owners in the pilot.
    switch (index) {
    case 1: return g_mainServiceCallbacks34Pilot;
    case 4: return g_mainServiceCallbacks40Pilot;
    case 6: return g_initializeCallbackRegistryBoundaryPilot.data;
    case 7: return g_mainServiceCallbacks4cPilot;
    case 8: return g_serviceWindowCallbacksPilot;
    case 16: return static_cast<CallbackListDataBoundaryPilot*>(g_mainWindowCommandCallbacksPilot);
    case 30: return static_cast<CallbackListDataBoundaryPilot*>(g_messageStartupCallbacksPilot);
    default: return g_applicationCallbackListsPilot[index];
    }
}

// 200f6688. A busy owner keeps the global pending state set.
extern "C" BOOL __cdecl CompactMainCallbackListsPilot()
{
    BOOL complete = TRUE;
    for (DWORD slot = 0; slot < 43; ++slot) {
        CallbackListDataBoundaryPilot* list = ReadApplicationCallbackListPilot(slot);
        if (list == NULL || list->count == 0 || list->pendingCompaction == 0) continue;
        if (list->dispatchDepth != 0) {
            complete = FALSE;
            continue;
        }
        CallbackListEntryBoundaryPilot* entries = reinterpret_cast<CallbackListEntryBoundaryPilot*>(list + 1);
        for (DWORD index = 0; index < list->count;) {
            if ((entries[index].flags & 1) != 0) EraseMainCallbackListEntryPilot(list, index);
            else ++index;
        }
        list->pendingCompaction = 0;
    }
    return complete;
}

// 200f67bd. Reload the owner after callbacks that can grow its list.
void __stdcall DispatchCallbackListSourcePilot(void** owner, unsigned int event, unsigned int argument)
{
    CallbackListDataBoundaryPilot* list = static_cast<CallbackListDataBoundaryPilot*>(*owner);
    if (list == NULL || list->count == 0) return;
    typedef void (__stdcall *CallbackPilot)(ULONG_PTR, ULONG_PTR);
    typedef void (__stdcall *CallbackWithContextPilot)(ULONG_PTR, ULONG_PTR, ULONG_PTR, ULONG_PTR);
    ++list->dispatchDepth;
    for (DWORD index = 0; index < list->count; ++index) {
        const CallbackListEntryBoundaryPilot& entry =
            reinterpret_cast<const CallbackListEntryBoundaryPilot*>(list + 1)[index];
        if ((entry.flags & 1) == 0) {
            if ((entry.flags & 2) != 0)
                reinterpret_cast<CallbackWithContextPilot>(entry.callback)(event, argument, entry.context1, entry.context2);
            else
                reinterpret_cast<CallbackPilot>(entry.callback)(event, argument);
        }
        list = static_cast<CallbackListDataBoundaryPilot*>(*owner);
    }
    --list->dispatchDepth;
    if (g_callbackListCompactionPendingPilot != 0 && CompactMainCallbackListsPilot())
        g_callbackListCompactionPendingPilot = 0;
}

// 200f7c66.
extern "C" void __cdecl ShutDownMainWindowServicesSourcePilot()
{
    ShutDownServicePacketRegistrationsPilot();
    ShutDownServiceObjectsSourcePilot();
    RemoveMainCallbackListEntryPilot(&g_mainServiceCallbacks40Pilot, MainServiceCallback7b9fPilot, 0, 0, 0);
    RemoveMainCallbackListEntryPilot(&g_mainServiceCallbacks34Pilot, MainServiceCallback7b95Pilot, 0, 0, 0);
    RemoveMainCallbackListEntryPilot(&g_mainServiceCallbacks4cPilot, MainServiceCallback7b7bPilot, 0, 0, 0);
    RemoveMainCallbackListEntryPilot(&g_mainServiceCallbacks34Pilot, MainServiceCallback7ba9Pilot, 0, 0, 0);
    RemoveMainCallbackListEntryPilot(&g_mainServiceCallbacks4cPilot, MainServiceCallback7ba9Pilot, 0, 0, 0);
    ShutDownServiceWindowSourcePilot();
    ShutDownServiceConsoleSourcePilot();
    g_mainWindowServicesActivePilot = 0;
}
