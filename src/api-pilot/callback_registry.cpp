#include "heap_boundary.h"
#include "callback_registry_boundary.h"
static CallbackListEntryBoundaryPilot* Entries(CallbackListDataBoundaryPilot* header)
{
    return reinterpret_cast<CallbackListEntryBoundaryPilot*>(header + 1);
}
int __stdcall RegisterCallbackListOwnerBoundaryPilot(CallbackListDataBoundaryPilot** owner,
    RegisteredCallbackBoundaryPilot callback, int withContext, ULONG_PTR context1, ULONG_PTR context2)
{
    if (owner == NULL)
        return -1;
    if (*owner == NULL) {
        AllocatePrivateHeapMemoryPilot(1, 0xb0, reinterpret_cast<void**>(owner), 1);
        if (*owner == NULL)
            return -1;
        (*owner)->count = 0;
        (*owner)->pendingCompaction = 0;
        (*owner)->capacity = 10;
    }
    CallbackListDataBoundaryPilot* list = *owner;
    if (list->count >= list->capacity) {
        if (ReallocatePrivateHeapMemoryBoundaryPilot(1,
            (list->capacity + 11) * 16, reinterpret_cast<void**>(owner)) != 0)
            return -1;
        list = *owner;
        list->capacity += 10;
    }
    Entries(list)[list->count].callback = callback;
    Entries(list)[list->count].flags = 0;
    if (withContext != 0)
        Entries(list)[list->count].flags |= 2;
    Entries(list)[list->count].context1 = context1;
    Entries(list)[list->count].context2 = context2;
    ++list->count;
    return 0;
}
int CallbackRegistryOwnerBoundaryPilot::Register(RegisteredCallbackBoundaryPilot callback,
    int withContext, ULONG_PTR context1, ULONG_PTR context2)
{
    return RegisterCallbackListOwnerBoundaryPilot(&data, callback, withContext, context1, context2);
}
// One native COFF carries both real dependency bodies; heap APIs remain unresolved.
#ifndef API_SEPARATE_REALLOCATOR
#include "reallocate_boundary.cpp"
#endif
