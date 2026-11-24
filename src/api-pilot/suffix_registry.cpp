#include "heap_boundary.h"
#include <string.h>
extern "C" char g_builtinSuffixesPilot[][8];
extern "C" int g_dynamicSuffixCountPilot;
extern "C" const char** g_dynamicSuffixesPilot;
extern "C" int __stdcall CopyPathSourceBoundaryPilot(const char*, char*, unsigned int);

// 2013556a. The original clears the table pointer and leaves the count stored.
extern "C" void __cdecl DestroyDynamicSuffixTablePilot()
{
    if (g_dynamicSuffixesPilot == NULL) return;
    for (int i = 0; i < g_dynamicSuffixCountPilot; ++i) {
        const char** slot = &g_dynamicSuffixesPilot[i];
        if (*slot != NULL) {
            HeapFree(g_privateHeapPilot, 0, const_cast<char*>(*slot));
            *slot = NULL;
        }
    }
    if (g_dynamicSuffixesPilot != NULL) {
        HeapFree(g_privateHeapPilot, 0, g_dynamicSuffixesPilot);
        g_dynamicSuffixesPilot = NULL;
    }
}

// Original 201350be, RET12. Observed return is always zero, including skipped copy.
extern "C" int __stdcall CopySelectedSuffixPilot(int selector, char* output, unsigned int capacity)
{
    const char* suffix = "";
    if (selector != -2) {
        if (selector < 70) suffix = g_builtinSuffixesPilot[selector];
        else if (selector < g_dynamicSuffixCountPilot + 70)
            suffix = g_dynamicSuffixesPilot[selector - 70];
    }
    if (capacity < 0x80000000u) CopyPathSourceBoundaryPilot(suffix, output, capacity);
    return 0;
}

// Original 201336ab, RET8. Input is uppercased in place, not const.
extern "C" int __stdcall FindSuffixSelectorPilot(char* suffix, int* selector)
{
    _strupr(suffix);
    for (int i = 0; i < 70; ++i) {
        if (strcmp(suffix, g_builtinSuffixesPilot[i]) == 0) {
            *selector = i;
            return 0;
        }
    }
    for (int j = 0; j < g_dynamicSuffixCountPilot; ++j) {
        if (strcmp(suffix, g_dynamicSuffixesPilot[j]) == 0) {
            *selector = j + 70;
            return 0;
        }
    }
    return -1;
}

#include "duplicate_heap_string.cpp"

// Original 20133778, RET8. Dynamic table grows by one pointer, no dedup changes.
extern "C" int __stdcall RegisterSuffixSelectorPilot(char* suffix, int* selector)
{
    _strupr(suffix);
    if (FindSuffixSelectorPilot(suffix, selector) == -1) {
        if (g_dynamicSuffixCountPilot == 0) {
            if (g_heapInitializedPilot == 0) InitializeHeapManagerPilot();
            g_dynamicSuffixesPilot = static_cast<const char**>(HeapAlloc(g_privateHeapPilot, 0, 4));
            if (g_dynamicSuffixesPilot == NULL) {
                g_dynamicSuffixesPilot = static_cast<const char**>(HeapAlloc(g_privateHeapPilot, 0, 4));
                if (g_dynamicSuffixesPilot == NULL) {
                    HINSTANCE instance = g_errorResourceInstancePilot;
                    if (instance == NULL) instance = GetModuleHandleA(NULL);
                    LoadStringA(instance, 8, g_heapErrorTextPilot, 260);
                    MessageBoxA(NULL, g_heapErrorTextPilot, NULL, MB_ICONERROR);
                    TerminateProcess(GetCurrentProcess(), 1);
                }
            }
        } else {
            ReallocatePrivateHeapMemoryBoundaryPilot(1, g_dynamicSuffixCountPilot * 4 + 4,
                reinterpret_cast<void**>(&g_dynamicSuffixesPilot));
        }
        *selector = g_dynamicSuffixCountPilot + 70;
        g_dynamicSuffixesPilot[g_dynamicSuffixCountPilot] = DuplicateHeapStringPilot(suffix);
        ++g_dynamicSuffixCountPilot;
    }
    return 0;
}
