#pragma once
#include "file_enumeration.h"

// Reconstructed minimum layouts, not original declarations. Record bytes are
// modified by 2014420b; offset0xe is a cached name length, not a disk-field claim.
#pragma pack(push, 1)
struct CabHeaderObserved36Pilot {
    char signature[4];
    BYTE unknown04[4];
    DWORD cabinetSize;
    BYTE unknown0c[4];
    DWORD entryOffset;
    BYTE unknown14[8];
    WORD entryCount, flags;
    BYTE unknown20[4];
};
struct AlternateEnumerationEntryPilot {
    DWORD sizeLow;
    BYTE unknown04[6];
    WORD dosDate, dosTime, nameLength;
    char name[1];
};
#pragma pack(pop)
C_ASSERT(sizeof(CabHeaderObserved36Pilot) == 0x24);
C_ASSERT(offsetof(CabHeaderObserved36Pilot, cabinetSize) == 8);
C_ASSERT(offsetof(CabHeaderObserved36Pilot, entryOffset) == 0x10);
C_ASSERT(offsetof(CabHeaderObserved36Pilot, entryCount) == 0x1c);
C_ASSERT(offsetof(CabHeaderObserved36Pilot, flags) == 0x1e);
struct AlternateEnumerationOwnerPilot {
    AlternateEnumerationOwnerPilot* next;
    DWORD lastUseValue;
    DWORD fileTimeLow, fileTimeHigh;
    char path[260];
    void* image;
    int entryCount;
    AlternateEnumerationEntryPilot* firstEntry;
    AlternateEnumerationEntryPilot* lastExactEntry;
};
struct AlternateEnumerationCursorPilot {
    AlternateEnumerationOwnerPilot* owner;
    int index;
    AlternateEnumerationEntryPilot* entry;
    char directory[260], stem[260], extension[260];
};
C_ASSERT(offsetof(AlternateEnumerationEntryPilot, dosDate) == 0xa);
C_ASSERT(offsetof(AlternateEnumerationEntryPilot, nameLength) == 0xe);
C_ASSERT(offsetof(AlternateEnumerationEntryPilot, name) == 0x10);
C_ASSERT(offsetof(AlternateEnumerationOwnerPilot, firstEntry) == 0x11c);
C_ASSERT(offsetof(AlternateEnumerationOwnerPilot, lastExactEntry) == 0x120);
C_ASSERT(offsetof(AlternateEnumerationOwnerPilot, path) == 0x10);
C_ASSERT(offsetof(AlternateEnumerationOwnerPilot, image) == 0x114);
C_ASSERT(sizeof(AlternateEnumerationOwnerPilot) == 0x124);
C_ASSERT(offsetof(AlternateEnumerationCursorPilot, directory) == 0xc);
C_ASSERT(offsetof(AlternateEnumerationCursorPilot, extension) == 0x214);
C_ASSERT(sizeof(AlternateEnumerationCursorPilot) == 0x318);

// Source-level argument lists; original ESI/EDI/EAX contracts are unresolved.
extern "C" int __stdcall NextAlternateEnumerationSourceBoundaryPilot(void*, void*, WIN32_FIND_DATAA*);
extern "C" HANDLE __stdcall OpenAlternateEnumerationSourceBoundaryPilot(const char*, WIN32_FIND_DATAA*);
extern "C" AlternateEnumerationOwnerPilot* __stdcall ResolveAlternateEnumerationOwnerPilot(const char*, char*);
extern "C" AlternateEnumerationEntryPilot* __stdcall FindCabOwnerEntrySourcePilot(
    AlternateEnumerationOwnerPilot*, const char*, DWORD*);
