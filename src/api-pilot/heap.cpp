#include "heap_boundary.h"
int InitializeHeapManagerPilot()
{
    const volatile int& initialized = g_heapInitializedPilot;
    const int initialState = initialized;
    if (initialState == 0) {
        // Both HeapCreate arguments reuse the tested zero state.
        g_privateHeapPilot = HeapCreate(initialState, 0x800000, initialState);
        DWORD privateMode = 2;
        HeapSetInformation(g_privateHeapPilot, HeapCompatibilityInformation, &privateMode, sizeof(privateMode));
        HANDLE heap = GetProcessHeap();
        DWORD processMode = 2;
        HeapSetInformation(heap, HeapCompatibilityInformation, &processMode, sizeof(processMode));
        heap = reinterpret_cast<HANDLE>(_get_heap_handle());
        DWORD crtMode = 2;
        HeapSetInformation(heap, HeapCompatibilityInformation, &crtMode, sizeof(crtMode));
        ++g_heapInitCallCountPilot;
        g_heapInitializedPilot = 1;
        return 0;
    }
    ++g_heapInitCallCountPilot;
    return 0;
}
int __stdcall AllocatePrivateHeapMemoryPilot(int failureMode, SIZE_T sizeBytes, void** outMemory, int zeroInitialize)
{
    // Read the flag once. VC8 emits the original register test in the inlined callers.
    const volatile int& initialized = g_heapInitializedPilot;
    if (initialized == 0)
        InitializeHeapManagerPilot();
    DWORD flags = 0;
    if (zeroInitialize != 0)
        flags = HEAP_ZERO_MEMORY;
    void* memory = HeapAlloc(g_privateHeapPilot, flags, sizeBytes);
    *outMemory = memory;
    if (memory == NULL) {
        if (failureMode == 4)
            return -1;
        memory = HeapAlloc(g_privateHeapPilot, flags, sizeBytes);
        *outMemory = memory;
        if (memory == NULL) {
            HINSTANCE instance = g_errorResourceInstancePilot;
            if (instance == NULL)
                instance = GetModuleHandleA(NULL);
            LoadStringA(instance, 8, g_heapErrorTextPilot, sizeof(g_heapErrorTextPilot));
            MessageBoxA(NULL, g_heapErrorTextPilot, NULL, MB_ICONHAND);
            TerminateProcess(GetCurrentProcess(), 1);
        }
    }
    return 0;
}
