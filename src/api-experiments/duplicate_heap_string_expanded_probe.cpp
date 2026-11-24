#include "heap_boundary.h"
#include <string.h>

extern "C" char* __stdcall DuplicateHeapStringPilot(const char* input)
{
    if (input == NULL) return NULL;
    unsigned int bytes = static_cast<unsigned int>(strlen(input)) + 1;
    if (g_heapInitializedPilot == 0) InitializeHeapManagerPilot();
    char* copy = static_cast<char*>(HeapAlloc(g_privateHeapPilot, 0, bytes));
    if (copy == NULL) {
        copy = static_cast<char*>(HeapAlloc(g_privateHeapPilot, 0, bytes));
        if (copy == NULL) {
            HINSTANCE instance = g_errorResourceInstancePilot;
            if (instance == NULL) instance = GetModuleHandleA(NULL);
            LoadStringA(instance, 8, g_heapErrorTextPilot, 260);
            MessageBoxA(NULL, g_heapErrorTextPilot, NULL, MB_ICONERROR);
            TerminateProcess(GetCurrentProcess(), 1);
        }
    }
    lstrcpyA(copy, input);
    return copy;
}
