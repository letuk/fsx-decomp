#include "cab_stream.h"
#include "heap_boundary.h"
#include <stdlib.h>

extern "C" __declspec(dllimport) BOOL WINAPI InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION, DWORD);

struct FileStreamCacheEntryPilot {
    FileStreamCacheEntryPilot* next;
    DWORD hits;
    DWORD lastUse;
    CabStreamSourcePilot* stream;
    char path[260];
};
C_ASSERT(sizeof(FileStreamCacheEntryPilot) == 0x114);
C_ASSERT(offsetof(FileStreamCacheEntryPilot, path) == 0x10);

extern "C" {
CRITICAL_SECTION g_fileStreamCacheLockPilot; // 2020e284
FileStreamCacheEntryPilot* g_fileStreamCacheHeadPilot; // 2020d8cc
}
extern int g_factoryZeroInitializeBoundaryPilot; // 2020d8ec
extern "C" DWORD __cdecl ReadCabOwnerUseValuePilot(); // 2012de9b
extern "C" int __stdcall ResolvePathPilot(char*, unsigned int, const char*, int);

// 201eef1c and 201ee73a. CRT placement remains a separate link step.
extern "C" void __cdecl DeleteFileStreamCacheLockPilot()
{
    DeleteCriticalSection(&g_fileStreamCacheLockPilot);
}

extern "C" void __cdecl RegisterFileStreamCachePilot()
{
    InitializeCriticalSectionAndSpinCount(&g_fileStreamCacheLockPilot, 100);
    atexit(DeleteFileStreamCacheLockPilot);
}

// 20144003: original output in ECX. Return reports a cache hit independently
// of the Clone status. A failed path resolution can leave the key unwritten.
extern "C" int __stdcall FindCachedFileStreamSourcePilot(CabStreamSourcePilot** output, const char* path)
{
    char key[260];
    ResolvePathPilot(key, sizeof(key), path, 0);
    EnterCriticalSection(&g_fileStreamCacheLockPilot);
    int found = 0;
    FileStreamCacheEntryPilot* previous = NULL;
    for (FileStreamCacheEntryPilot* entry = g_fileStreamCacheHeadPilot;
        entry != NULL; entry = entry->next) {
        if (lstrcmpiA(entry->path, key) == 0) {
            found = 1;
            ++entry->hits;
            entry->lastUse = ReadCabOwnerUseValuePilot();
            if (previous != NULL) {
                previous->next = entry->next;
                entry->next = g_fileStreamCacheHeadPilot;
                g_fileStreamCacheHeadPilot = entry;
            }
            if (output != NULL) entry->stream->vtable->clone(entry->stream, output);
            break;
        }
        previous = entry;
    }
    LeaveCriticalSection(&g_fileStreamCacheLockPilot);
    return found;
}

// 201441a4: original entry in ESI. The caller frees the detached record.
extern "C" void __stdcall DetachFileStreamCacheEntryPilot(FileStreamCacheEntryPilot* entry)
{
    entry->stream->vtable->release(entry->stream);
    entry->stream = NULL;
    EnterCriticalSection(&g_fileStreamCacheLockPilot);
    FileStreamCacheEntryPilot** link = &g_fileStreamCacheHeadPilot;
    while (*link != NULL && *link != entry) link = &(*link)->next;
    if (*link == entry) *link = entry->next;
    LeaveCriticalSection(&g_fileStreamCacheLockPilot);
}

// 201440cc: snapshot next before calling the stream and before detaching.
extern "C" void __stdcall PruneFileStreamCachePilot(DWORD now)
{
    EnterCriticalSection(&g_fileStreamCacheLockPilot);
    DWORD rank = 0;
    FileStreamCacheEntryPilot* entry = g_fileStreamCacheHeadPilot;
    while (entry != NULL) {
        FileStreamCacheEntryPilot* next = entry->next;
        ++rank;
        if (now - entry->lastUse > 300000 || rank > 5) {
            CabStreamSourcePilot* stream = entry->stream;
            stream->vtable->addReference(stream);
            if (stream->vtable->release(stream) == 1) {
                DetachFileStreamCacheEntryPilot(entry);
                HeapFree(g_privateHeapPilot, 0, entry);
            }
        }
        entry = next;
    }
    LeaveCriticalSection(&g_fileStreamCacheLockPilot);
}

// 20144152: original entry in ESI, stream in EDI and path in EAX.
extern "C" FileStreamCacheEntryPilot* __stdcall LinkFileStreamCacheEntryPilot(
    FileStreamCacheEntryPilot* entry, CabStreamSourcePilot* stream, const char* path)
{
    ResolvePathPilot(entry->path, sizeof(entry->path), path, 0);
    stream->vtable->addReference(stream);
    entry->stream = stream;
    entry->hits = 0;
    entry->lastUse = ReadCabOwnerUseValuePilot();
    EnterCriticalSection(&g_fileStreamCacheLockPilot);
    entry->next = g_fileStreamCacheHeadPilot;
    g_fileStreamCacheHeadPilot = entry;
    LeaveCriticalSection(&g_fileStreamCacheLockPilot);
    return entry;
}

// 20143f76: original stream in EAX. A valid miss returns one even if
// allocation returns NULL after the original termination path.
extern "C" int __stdcall RememberFileStreamSourcePilot(CabStreamSourcePilot* stream, const char* path)
{
    if (stream == NULL || path == NULL || *path == '\0') return 0;
    EnterCriticalSection(&g_fileStreamCacheLockPilot);
    int insertedOnMiss = 0;
    if (FindCachedFileStreamSourcePilot(NULL, path) == 0) {
        PruneFileStreamCachePilot(ReadCabOwnerUseValuePilot());
        void* memory;
        AllocatePrivateHeapMemoryPilot(1, sizeof(FileStreamCacheEntryPilot),
            &memory, g_factoryZeroInitializeBoundaryPilot);
        if (memory != NULL)
            LinkFileStreamCacheEntryPilot(static_cast<FileStreamCacheEntryPilot*>(memory), stream, path);
        insertedOnMiss = 1;
    }
    LeaveCriticalSection(&g_fileStreamCacheLockPilot);
    return insertedOnMiss;
}

// 20143f3d: teardown drains the head through the same release/unlink path.
extern "C" void __cdecl DrainFileStreamCachePilot()
{
    while (g_fileStreamCacheHeadPilot != NULL) {
        FileStreamCacheEntryPilot* entry = g_fileStreamCacheHeadPilot;
        DetachFileStreamCacheEntryPilot(entry);
        HeapFree(g_privateHeapPilot, 0, entry);
    }
}
