#include "heap_boundary.h"
int __stdcall ReallocatePrivateHeapMemoryBoundaryPilot(int failureMode, SIZE_T sizeBytes, void** inOutMemory)
{
    const volatile int& initialized = g_heapInitializedPilot;
    if (initialized == 0)
        InitializeHeapManagerPilot();
    void* oldMemory = *inOutMemory;
    if (oldMemory == NULL)
        return AllocatePrivateHeapMemoryPilot(failureMode, sizeBytes, inOutMemory, 0);
    void* memory = HeapReAlloc(g_privateHeapPilot, 0, oldMemory, sizeBytes);
    *inOutMemory = memory;
    if (memory == NULL) {
        if (failureMode == 4) {
            *inOutMemory = oldMemory;
            return -1;
        }
        // Original second attempt passes NULL, not oldMemory. Preserve it.
        memory = HeapReAlloc(g_privateHeapPilot, 0, NULL, sizeBytes);
        if (memory == NULL) {
            // Original has no GetModuleHandle fallback on this path.
            LoadStringA(g_errorResourceInstancePilot, 8, g_heapErrorTextPilot, 260);
            MessageBoxA(NULL, g_heapErrorTextPilot, NULL, MB_ICONHAND);
            TerminateProcess(GetCurrentProcess(), 1);
        }
        *inOutMemory = memory;
    }
    return 0;
}
