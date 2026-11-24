#include "heap_boundary.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern "C" char* __stdcall CopyResolvedPathPilot(char*, const char*, unsigned int);
extern "C" char* __stdcall AppendResolvedPathPilot(char*, const char*, unsigned int);

// 200fb1ab. Original output in EDI and capacity in EAX.
extern "C" int __cdecl FormatBoundedTextPilot(
    char* output, unsigned int capacity, const char* format, ...)
{
    if (capacity == 0 || capacity > 0x7fffffffu) return 0x80070057;
    unsigned int limit = capacity - 1;
    va_list arguments;
    va_start(arguments, format);
    int written = _vsnprintf(output, limit, format, arguments);
    va_end(arguments);
    if (written < 0 || static_cast<unsigned int>(written) > limit) {
        output[limit] = '\0';
        return 0x8007007a;
    }
    if (static_cast<unsigned int>(written) == limit) output[limit] = '\0';
    return 0;
}

#include "numbered_name_exports.h"

// 201322e2 / 206. The result is owned by the caller on the private heap.
extern "C" char* __stdcall AllocateNumberedNamePilot(const char* prefix, int number)
{
    if (prefix == NULL) return NULL;
    char suffix[100];
    StringCchPrintfA(suffix, sizeof(suffix), ".%d", number);
    unsigned int bytes = static_cast<unsigned int>(strlen(prefix) + strlen(suffix) + 1);
    void* memory;
    AllocatePrivateHeapMemoryPilot(1, bytes, &memory, 1);
    char* output = static_cast<char*>(memory);
    CopyResolvedPathPilot(output, prefix, bytes);
    AppendResolvedPathPilot(output, suffix, bytes);
    return output;
}

// 20135f51 / 108. The slot is required, while a null pointee is allowed.
extern "C" int __stdcall FreePrivateHeapSlotPilot(void** slot)
{
    if (*slot != NULL) {
        HeapFree(g_privateHeapPilot, 0, *slot);
        *slot = NULL;
    }
    return 0;
}
