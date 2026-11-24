#include "alternate_enumeration.h"
#include "heap_boundary.h"
#include <string.h>
#include <new>

extern AlternateEnumerationOwnerPilot* g_alternateOwnerHeadPilot; // 2020d8e8
extern CRITICAL_SECTION g_alternateOwnerLockPilot; // 2020e29c
extern "C" DWORD __cdecl ReadCabOwnerUseValuePilot(); // 2012de9b; time domain unresolved
// Original 201449c4 uses ESI owner/ECX path; implementation still unresolved.
extern "C" int __stdcall LoadCabOwnerSourceBoundaryPilot(AlternateEnumerationOwnerPilot*, const char*);
extern "C" int __stdcall CopyPathSourceBoundaryPilot(const char*, char*, unsigned int);
extern "C" int __stdcall ResolvePathPilot(char*, unsigned int, const char*, int);
extern int g_alternateAllocationZeroInitializePilot;

// 20145169: input in EDI, archive/relative outputs on stack, RET8.
// Preserves observed look-ahead and output mutation; no extra path validation.
extern "C" int __stdcall SplitCabPathSourceBoundaryPilot(
    const char* input, char* archive, char* relative)
{
    int length = static_cast<int>(strlen(input));
    int index = 0, lastDot = -1, lastSlash = -1;
    while (input[index] != '\0' && index < length) {
        archive[index] = input[index];
        if (input[index] == '.') lastDot = index;
        if (input[index] == '/' || input[index] == '\\') lastSlash = index;
        if (lastDot == index &&
            (input[index + 1] == 'C' || input[index + 1] == 'c') &&
            (input[index + 2] == 'A' || input[index + 2] == 'a') &&
            (input[index + 3] == 'B' || input[index + 3] == 'b')) {
            CopyPathSourceBoundaryPilot(".CAB", archive + index, 260);
            DWORD attributes = GetFileAttributesA(archive);
            if (attributes == INVALID_FILE_ATTRIBUTES) return 0;
            if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                if (input[index + 4] == '\0') relative[0] = '\0';
                else CopyPathSourceBoundaryPilot(input + index + 5, relative, 260);
                return 1;
            }
        }
        ++index;
    }
    if (lastSlash < lastDot) {
        if (lastSlash < 1) return 0;
        CopyPathSourceBoundaryPilot(".CAB", archive + lastSlash, 260);
        if (GetFileAttributesA(archive) == INVALID_FILE_ATTRIBUTES) return 0;
        CopyPathSourceBoundaryPilot(input + lastSlash + 1, relative, 260);
        return 1;
    }
    CopyPathSourceBoundaryPilot(".CAB", archive + index, 260);
    if (GetFileAttributesA(archive) == INVALID_FILE_ATTRIBUTES) return 0;
    relative[0] = '\0';
    return 1;
}

// 2014483b: owner in EAX, path on stack, RET4, returns owner in EAX.
extern "C" AlternateEnumerationOwnerPilot* __stdcall InitializeCabOwnerSourceBoundaryPilot(
    AlternateEnumerationOwnerPilot* owner, const char* path)
{
    owner->image = NULL;
    owner->lastExactEntry = NULL;
    owner->entryCount = 0;
    owner->firstEntry = NULL;
    owner->path[0] = '\0';
    owner->lastUseValue = ReadCabOwnerUseValuePilot();
    owner->fileTimeLow = 0;
    owner->fileTimeHigh = 0;
    EnterCriticalSection(&g_alternateOwnerLockPilot);
    owner->next = g_alternateOwnerHeadPilot;
    g_alternateOwnerHeadPilot = owner;
    LeaveCriticalSection(&g_alternateOwnerLockPilot);
    LoadCabOwnerSourceBoundaryPilot(owner, path); // Original ignores load status.
    return owner;
}

// 201448a4: owner in ESI, no stack arguments; releases image/unlinks owner,
// but does not free owner storage itself. Not the constructor-unwind funclet.
extern "C" void __stdcall DestroyCabOwnerSourceBoundaryPilot(AlternateEnumerationOwnerPilot* owner)
{
    if (owner->image != NULL) {
        HeapFree(g_privateHeapPilot, 0, owner->image);
        owner->image = NULL;
    }
    EnterCriticalSection(&g_alternateOwnerLockPilot);
    if (g_alternateOwnerHeadPilot == owner) {
        g_alternateOwnerHeadPilot = owner->next;
    } else {
        AlternateEnumerationOwnerPilot* previous = g_alternateOwnerHeadPilot;
        while (previous != NULL && previous->next != owner) previous = previous->next;
        if (previous != NULL) previous->next = owner->next;
    }
    LeaveCriticalSection(&g_alternateOwnerLockPilot);
}

// A placement tag models the original allocation/constructor cleanup pair:
// free raw storage if construction throws, without an invented lock guard.
// The fixed pilot compiler profile currently omits this unwind cleanup; source
// intent is not emitted-EH equivalence. See api-cab-owner-cache.md.
struct CabOwnerStoragePilot { void* memory; };
inline void* operator new(size_t, CabOwnerStoragePilot storage) { return storage.memory; }
inline void operator delete(void* memory, CabOwnerStoragePilot) {
    if (memory != NULL) HeapFree(g_privateHeapPilot, 0, memory);
}
struct ConstructedCabOwnerPilot : AlternateEnumerationOwnerPilot {
    explicit ConstructedCabOwnerPilot(const char* path) {
        InitializeCabOwnerSourceBoundaryPilot(this, path);
    }
};
C_ASSERT(sizeof(ConstructedCabOwnerPilot) == 0x124);

// 201452e1: stack path/relative output, RET8. Normal cache paths plus explicit
// constructor-failure cleanup intent. Fixed-profile native EH remains incomplete.
extern "C" AlternateEnumerationOwnerPilot* __stdcall ResolveAlternateEnumerationOwnerPilot(
    const char* path, char* relative)
{
    char archive[260];
    if (SplitCabPathSourceBoundaryPilot(path, archive, relative) == 0) return NULL;
    ResolvePathPilot(archive, 260, archive, 0); // Original ignores status.
    EnterCriticalSection(&g_alternateOwnerLockPilot);
    AlternateEnumerationOwnerPilot* previous = NULL;
    AlternateEnumerationOwnerPilot* owner = g_alternateOwnerHeadPilot;
    while (owner != NULL && lstrcmpiA(owner->path, archive) != 0) {
        previous = owner;
        owner = owner->next;
    }
    if (owner != NULL) {
        owner->lastUseValue = ReadCabOwnerUseValuePilot();
        if (previous != NULL) {
            previous->next = owner->next;
            owner->next = g_alternateOwnerHeadPilot;
            g_alternateOwnerHeadPilot = owner;
        }
    } else {
        CabOwnerStoragePilot storage;
        AllocatePrivateHeapMemoryPilot(1, 0x124, &storage.memory,
            g_alternateAllocationZeroInitializePilot);
        if (storage.memory != NULL) owner = new (storage) ConstructedCabOwnerPilot(archive);
    }
    LeaveCriticalSection(&g_alternateOwnerLockPilot);
    return owner;
}

// 20144b9b. Original owner in EDI, relative path in ECX, output on stack.
extern "C" AlternateEnumerationEntryPilot* __stdcall FindCabOwnerEntrySourcePilot(
    AlternateEnumerationOwnerPilot* owner, const char* relativePath, DWORD* attributes)
{
    DWORD length = lstrlenA(relativePath);
    char upperPath[260];
    CopyPathSourceBoundaryPilot(relativePath, upperPath, sizeof(upperPath));
    CharUpperA(upperPath);

    AlternateEnumerationEntryPilot* entry = owner->lastExactEntry;
    if (entry != NULL && length == entry->nameLength && strcmp(upperPath, entry->name) == 0) {
        *attributes = 0x81;
        return entry;
    }

    entry = owner->firstEntry;
    for (int index = 0; index < owner->entryCount; ++index) {
        if (length == entry->nameLength && strcmp(upperPath, entry->name) == 0) {
            *attributes = 0x81;
            owner->lastExactEntry = entry;
            return entry;
        }
        if (length < entry->nameLength && entry->name[length] == '\\' &&
            strncmp(upperPath, entry->name, length) == 0) {
            *attributes = 0x11;
            return entry;
        }
        entry = reinterpret_cast<AlternateEnumerationEntryPilot*>(
            reinterpret_cast<BYTE*>(entry) + 0x11 + entry->nameLength);
    }
    *attributes = INVALID_FILE_ATTRIBUTES;
    return NULL;
}

// 2014507f. The archive root is a directory even if the owner has no entries.
extern "C" unsigned long __stdcall QueryFallbackAttributesPilot(const char* path)
{
    char relativePath[260];
    AlternateEnumerationOwnerPilot* owner = ResolveAlternateEnumerationOwnerPilot(path, relativePath);
    if (owner == NULL) return INVALID_FILE_ATTRIBUTES;
    if (relativePath[0] == '\0') return 0x11;
    if (owner->entryCount == 0) return INVALID_FILE_ATTRIBUTES;
    DWORD attributes;
    FindCabOwnerEntrySourcePilot(owner, relativePath, &attributes);
    return attributes;
}
