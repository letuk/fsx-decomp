#include "heap_boundary.h"
#include <string.h>

// api.dll 20137efe. The returned copy belongs to the caller on the private heap.
extern "C" char* __stdcall DuplicateHeapStringPilot(const char* input)
{
    if (input == NULL)
        return NULL;

    const SIZE_T bytes = strlen(input) + 1;
    void* memory;
    // Retry a failed allocation. The subsequent copy supplies all bytes.
    AllocatePrivateHeapMemoryPilot(1, bytes, &memory, 0);
    char* copy = static_cast<char*>(memory);
    lstrcpyA(copy, input);
    return copy;
}
