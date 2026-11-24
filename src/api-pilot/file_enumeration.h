#pragma once
#include <windows.h>
#include <stddef.h>
// Observed allocation/fields only; original structure names unproven.
struct FileEnumerationStatePilot {
    HANDLE handle;
    WIN32_FIND_DATAA current;
    int suffixSelector;
    unsigned int flags;
};
struct FileEnumerationRecordPilot {
    char name[260];
    int suffixSelector;
    DWORD sizeLow, sizeHigh, writable;
    WORD localTime[6];
    WORD utcTime[6];
};
C_ASSERT(sizeof(WIN32_FIND_DATAA) == 320);
C_ASSERT(offsetof(FileEnumerationStatePilot, current) == 4);
C_ASSERT(offsetof(FileEnumerationStatePilot, suffixSelector) == 0x144);
C_ASSERT(offsetof(FileEnumerationStatePilot, flags) == 0x148);
C_ASSERT(sizeof(FileEnumerationStatePilot) == 0x14c);
C_ASSERT(offsetof(FileEnumerationRecordPilot, suffixSelector) == 0x104);
C_ASSERT(offsetof(FileEnumerationRecordPilot, localTime) == 0x114);
C_ASSERT(offsetof(FileEnumerationRecordPilot, utcTime) == 0x120);
C_ASSERT(sizeof(FileEnumerationRecordPilot) == 0x12c);
extern "C" int __stdcall OpenFileEnumerationPilot(FileEnumerationStatePilot**, const char*, const char*, int);
extern "C" int __stdcall NextFileEnumerationPilot(FileEnumerationStatePilot*, FileEnumerationRecordPilot*);
extern "C" int __stdcall CloseFileEnumerationPilot(FileEnumerationStatePilot**);
