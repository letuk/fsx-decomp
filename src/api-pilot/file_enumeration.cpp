#include "file_enumeration.h"
#include "heap_boundary.h"
extern "C" int __stdcall CloseFileEnumerationPilot(FileEnumerationStatePilot** state)
{
    if (((*state)->flags & 2) != 0) {
        void* memory = (*state)->handle;
        if (memory != NULL && memory != INVALID_HANDLE_VALUE)
            HeapFree(g_privateHeapPilot, 0, memory);
    } else {
        FindClose((*state)->handle);
    }
    if (*state != NULL) {
        HeapFree(g_privateHeapPilot, 0, *state);
        *state = NULL;
    }
    return 0;
}
