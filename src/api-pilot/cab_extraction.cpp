#include "alternate_enumeration.h"
#include "cab_stream.h"
#include <fdi.h>
#include <string.h>

extern HFDI g_fdiContextBoundaryPilot; // 2020d8dc
extern AlternateEnumerationOwnerPilot* g_alternateOwnerHeadPilot; // 2020d8e8
extern CRITICAL_SECTION g_alternateOwnerLockPilot; // 2020e29c
extern "C" int __stdcall ResolvePathPilot(char*, unsigned int, const char*, int);
extern "C" void __stdcall DestroyCabOwnerSourceBoundaryPilot(AlternateEnumerationOwnerPilot*);

// 201444c7. The FDI context word is left as stored after destruction.
extern "C" void __cdecl DrainCabOwnersAndDestroyFdiPilot()
{
    while (g_alternateOwnerHeadPilot != NULL) {
        AlternateEnumerationOwnerPilot* owner = g_alternateOwnerHeadPilot;
        DestroyCabOwnerSourceBoundaryPilot(owner);
        HeapFree(g_privateHeapPilot, 0, owner);
    }
    FDIDestroy(g_fdiContextBoundaryPilot);
}

static DWORD g_cabFdiOpenCountPilot; // 2020d8e0
static AlternateEnumerationOwnerPilot* g_activeCabFdiOwnerPilot; // 2020d8e4

struct CabExtractionPilot {
    AlternateEnumerationOwnerPilot* owner;
    const char* relativePath;
    CabStreamSourcePilot* output;
    int completed;
};
C_ASSERT(sizeof(CabExtractionPilot) == 16);
C_ASSERT(offsetof(FDINOTIFICATION, pv) == 0x10);

// 201445fe. FDI opens the image prefix first and the archive backing stream later.
FNOPEN(FdiOpenBoundaryPilot)
{
    AlternateEnumerationOwnerPilot* owner = g_activeCabFdiOwnerPilot;
    if (owner == NULL || owner->image == NULL) return -1;

    const CabHeaderObserved36Pilot* image = static_cast<const CabHeaderObserved36Pilot*>(owner->image);
    const BYTE* bytes = reinterpret_cast<const BYTE*>(image);
    const BYTE* folder = bytes + 0x24;
    if (image->flags == 4) {
        WORD reserveLength;
        memcpy(&reserveLength, folder, sizeof(reserveLength));
        folder += 4 + reserveLength;
    }
    DWORD prefixSize;
    memcpy(&prefixSize, folder, sizeof(prefixSize));
    DWORD data = reinterpret_cast<DWORD>(image);

    CabStreamSourcePilot* result;
    if (g_cabFdiOpenCountPilot == 0) {
        result = WrapCabStreamSourcePilot(image->cabinetSize, 0, data, prefixSize, NULL, 0);
    } else {
        char resolved[260];
        if (ResolvePathPilot(resolved, sizeof(resolved), owner->path, 0x34) != 0) return -1;
        CabStreamSourcePilot* stream = OpenCabFileStreamSourcePilot(
            resolved, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (stream == NULL) return -1;
        CabStreamSourcePilot* wrapped = WrapCabStreamSourcePilot(0xffffffff, 0, 0, 0x1000, stream, 0);
        if (wrapped != NULL) {
            ReleaseCabStreamPilot(stream);
            stream = wrapped;
        }
        image = static_cast<const CabHeaderObserved36Pilot*>(g_activeCabFdiOwnerPilot->image);
        result = WrapCabStreamSourcePilot(image->cabinetSize, 0x20000000, data, prefixSize, stream, 0);
        stream->vtable->release(stream);
    }
    ++g_cabFdiOpenCountPilot;
    return reinterpret_cast<INT_PTR>(result);
}

// 201447d8. Close-file notification rewinds and marks completion.
static INT_PTR DIAMONDAPI NotifyCabExtractionPilot(FDINOTIFICATIONTYPE type, PFDINOTIFICATION info)
{
    CabExtractionPilot* state = static_cast<CabExtractionPilot*>(info->pv);
    if (type == fdintCOPY_FILE) {
        if (state->output == NULL)
            state->output = WrapCabStreamSourcePilot(info->cb, 0, 0, 0, NULL, 0);
        return reinterpret_cast<INT_PTR>(state->output);
    }
    if (type == fdintCLOSE_FILE_INFO) {
        CabStreamSourcePilot* output = state->output;
        output->vtable->seek(output, 0, 0, 0, NULL);
        state->completed = 1;
    }
    return 0;
}

// 20144924. Original ESI owner, EAX entry, one stack argument.
static BOOL DispatchCabExtractionPilot(AlternateEnumerationOwnerPilot* owner,
    AlternateEnumerationEntryPilot* entry, CabExtractionPilot* state)
{
    static char cabinetName[] = "X";
    static char cabinetPath[] = "";
    EnterCriticalSection(&g_alternateOwnerLockPilot);
    CabHeaderObserved36Pilot* image = static_cast<CabHeaderObserved36Pilot*>(owner->image);
    BYTE* bytes = reinterpret_cast<BYTE*>(image);
    image->entryOffset = static_cast<DWORD>(reinterpret_cast<BYTE*>(entry) - bytes);
    g_activeCabFdiOwnerPilot = owner;
    g_cabFdiOpenCountPilot = 0;
    image->entryCount = 1;
    BOOL result = FDICopy(g_fdiContextBoundaryPilot, cabinetName, cabinetPath, 0,
        NotifyCabExtractionPilot, NULL, state);
    image = static_cast<CabHeaderObserved36Pilot*>(owner->image);
    bytes = reinterpret_cast<BYTE*>(image);
    image->entryOffset = static_cast<DWORD>(reinterpret_cast<BYTE*>(owner->firstEntry) - bytes);
    image->entryCount = static_cast<WORD>(owner->entryCount);
    g_activeCabFdiOwnerPilot = NULL;
    LeaveCriticalSection(&g_alternateOwnerLockPilot);
    return result;
}

// 20144b0e. Original EAX owner and two stack arguments.
extern "C" CabStreamSourcePilot* __stdcall ExtractCabEntryToStreamPilot(
    AlternateEnumerationOwnerPilot* owner, const char* relativePath, CabStreamSourcePilot* output)
{
    if (owner->entryCount == 0) return NULL;
    DWORD attributes;
    AlternateEnumerationEntryPilot* entry = FindCabOwnerEntrySourcePilot(owner, relativePath, &attributes);
    if (entry == NULL || (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) return NULL;

    CabExtractionPilot state = { owner, relativePath, output, 0 };
    if (output != NULL) output->vtable->addReference(output);
    DispatchCabExtractionPilot(owner, entry, &state);
    if (output != NULL && state.completed != 0) output->vtable->release(output);
    return state.completed != 0 ? state.output : NULL;
}

// 20144f5f. Returns the stream only after the close-file notification.
extern "C" CabStreamSourcePilot* __stdcall OpenCabEntryStreamPilot(const char* path)
{
    char relativePath[260];
    AlternateEnumerationOwnerPilot* owner = ResolveAlternateEnumerationOwnerPilot(path, relativePath);
    if (owner == NULL || owner->entryCount == 0) return NULL;
    DWORD attributes;
    AlternateEnumerationEntryPilot* entry = FindCabOwnerEntrySourcePilot(owner, relativePath, &attributes);
    if (entry == NULL || (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) return NULL;

    CabExtractionPilot state = { owner, relativePath, NULL, 0 };
    DispatchCabExtractionPilot(owner, entry, &state);
    return state.completed != 0 ? state.output : NULL;
}
