#include "callback_registry_boundary.h"

// Observed owner slots at 2020bb30. These entry points retain both caller shapes.
extern "C" {
__declspec(dllexport) CallbackListDataBoundaryPilot* g_registryProbeOwners[43];
}

extern "C" __declspec(dllexport) int __stdcall RegisterSimpleCallbackProbe(
    unsigned int index, RegisteredCallbackBoundaryPilot callback)
{
    return RegisterCallbackListOwnerBoundaryPilot(&g_registryProbeOwners[index], callback, 0, 0, 0);
}

extern "C" __declspec(dllexport) int __stdcall RegisterContextCallbackProbe(
    unsigned int index, RegisteredCallbackBoundaryPilot callback, ULONG_PTR context1, ULONG_PTR context2)
{
    return RegisterCallbackListOwnerBoundaryPilot(&g_registryProbeOwners[index], callback, 1, context1, context2);
}
