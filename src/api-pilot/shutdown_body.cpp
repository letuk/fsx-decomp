#include "application_boundary.h"
#include "heap_boundary.h"

extern ApplicationBoundaryPilot* g_applicationBoundaryPilot;
extern "C" void __cdecl RunApplicationShutdownPrefixPilot(); // 20120ed9..20121020
extern "C" void __cdecl ShutDownApplicationModulesPilot(); // 2012260f
extern "C" void __cdecl ClearStreamParserSlotsPilot(); // 201306ee
extern "C" void __cdecl DrainFileStreamCachePilot(); // 20143f3d
extern "C" void __cdecl DrainCabOwnersAndDestroyFdiPilot(); // 201444c7
extern "C" void __cdecl DestroyDynamicSuffixTablePilot(); // 2013556a
extern "C" void __cdecl ShutDownResourceSlotsPilot(); // 200f638d

// 20120ed5. The prefix remains a source boundary. The continuation at
// 20121021 is reached with or without the main-window cleanup branch.
int __cdecl ShutdownApplicationBodyPilot()
{
    RunApplicationShutdownPrefixPilot();
    ShutDownApplicationModulesPilot();
    ClearStreamParserSlotsPilot();
    DrainFileStreamCachePilot();
    DrainCabOwnersAndDestroyFdiPilot();
    DestroyDynamicSuffixTablePilot();
    ShutDownResourceSlotsPilot();
    if (g_applicationBoundaryPilot != NULL)
        HeapFree(g_privateHeapPilot, 0, g_applicationBoundaryPilot);
    --g_heapInitCallCountPilot;
    g_applicationBoundaryPilot = NULL;
    if (g_heapInitCallCountPilot == 0) {
        if (HeapDestroy(g_privateHeapPilot)) g_privateHeapPilot = NULL;
        g_heapInitializedPilot = 0;
    }
    return 0;
}
