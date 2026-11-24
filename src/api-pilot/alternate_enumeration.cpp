#include "alternate_enumeration.h"
#include "heap_boundary.h"
#include <stdlib.h>
#include <string.h>

// 2020d8ec; initialization and cache ownership remain dependencies.
extern int g_alternateAllocationZeroInitializePilot;

// 20144df0: owner on stack, cursor in EDI, output in ESI, RET4.
extern "C" int __stdcall NextAlternateEnumerationSourceBoundaryPilot(
    void* ownerInput, void* cursorInput, WIN32_FIND_DATAA* output)
{
    AlternateEnumerationOwnerPilot* owner = static_cast<AlternateEnumerationOwnerPilot*>(ownerInput);
    AlternateEnumerationCursorPilot* cursor = static_cast<AlternateEnumerationCursorPilot*>(cursorInput);
    if (cursor == NULL) return 0;
    while (cursor->index < owner->entryCount) {
        char directory[260], stem[260], extension[260];
        _splitpath(cursor->entry->name, NULL, directory, stem, extension);
        bool found = false;
        if (lstrcmpiA(directory, cursor->directory) == 0 &&
            (cursor->extension[1] == '*' || lstrcmpiA(extension, cursor->extension) == 0)) {
            memset(output, 0, sizeof(*output));
            const char* source = cursor->entry->name;
            char* destination = output->cFileName;
            int remaining = 260;
            while (*source != '\0' && remaining != 0) {
                *destination++ = *source++;
                --remaining;
            }
            if (remaining == 0) --destination;
            *destination = '\0';
            output->dwFileAttributes = 0x81;
            output->nFileSizeLow = cursor->entry->sizeLow;
            FILETIME localTime, utcTime;
            DosDateTimeToFileTime(cursor->entry->dosDate, cursor->entry->dosTime, &localTime);
            LocalFileTimeToFileTime(&localTime, &utcTime);
            output->ftCreationTime = utcTime;
            output->ftLastAccessTime = utcTime;
            // Original leaves ftLastWriteTime zero, despite outer reader using it.
            found = true;
        }
        ++cursor->index;
        cursor->entry = reinterpret_cast<AlternateEnumerationEntryPilot*>(
            reinterpret_cast<BYTE*>(cursor->entry) + cursor->entry->nameLength + 0x11);
        if (found) return 1;
    }
    return 0;
}

// 20144d5d: owner in EAX, path/output on stack, RET8.
extern "C" HANDLE __stdcall CreateAlternateEnumerationSourceBoundaryPilot(
    AlternateEnumerationOwnerPilot* owner, const char* path, WIN32_FIND_DATAA* output)
{
    AlternateEnumerationCursorPilot* cursor;
    AllocatePrivateHeapMemoryPilot(1, 0x318, reinterpret_cast<void**>(&cursor),
        g_alternateAllocationZeroInitializePilot);
    if (cursor == NULL) return INVALID_HANDLE_VALUE;
    _splitpath(path, NULL, cursor->directory, cursor->stem, cursor->extension);
    cursor->owner = owner;
    cursor->index = 0;
    cursor->entry = owner->firstEntry;
    if (NextAlternateEnumerationSourceBoundaryPilot(owner, cursor, output) == 0) {
        HeapFree(g_privateHeapPilot, 0, cursor);
        return INVALID_HANDLE_VALUE;
    }
    return cursor;
}

// 20145110: path on stack, output in ESI, RET4.
extern "C" HANDLE __stdcall OpenAlternateEnumerationSourceBoundaryPilot(
    const char* path, WIN32_FIND_DATAA* output)
{
    char relativePath[260];
    AlternateEnumerationOwnerPilot* owner = ResolveAlternateEnumerationOwnerPilot(path, relativePath);
    if (owner == NULL) return INVALID_HANDLE_VALUE;
    return CreateAlternateEnumerationSourceBoundaryPilot(owner, relativePath, output);
}
