#pragma once
#include <windows.h>
#include <stddef.h>
struct CabStreamSourcePilot;
struct CabStreamStatObserved72Pilot {
    LPWSTR name;
    DWORD type;
    ULARGE_INTEGER size;
    FILETIME modified, created, accessed;
    DWORD mode;
    BYTE unknown2c[0x1c];
};
struct CabStreamVTableObservedPilot {
    int (__stdcall *query)(CabStreamSourcePilot*, const void*, CabStreamSourcePilot**);
    ULONG (__stdcall *addReference)(CabStreamSourcePilot*);
    ULONG (__stdcall *release)(CabStreamSourcePilot*);
    int (__stdcall *read)(CabStreamSourcePilot*, void*, DWORD, DWORD*);
    int (__stdcall *write)(CabStreamSourcePilot*, const void*, DWORD, DWORD*);
    int (__stdcall *seek)(CabStreamSourcePilot*, DWORD, DWORD, DWORD, void*);
    void* unknown18;
    int (__stdcall *copyTo)(CabStreamSourcePilot*, CabStreamSourcePilot*, DWORD, DWORD,
        ULARGE_INTEGER*, ULARGE_INTEGER*);
    void* unknown20[4];
    int (__stdcall *stat)(CabStreamSourcePilot*, CabStreamStatObserved72Pilot*, DWORD);
    int (__stdcall *clone)(CabStreamSourcePilot*, CabStreamSourcePilot**);
};
struct CabStreamSourcePilot { const CabStreamVTableObservedPilot* vtable; };
C_ASSERT(sizeof(CabStreamVTableObservedPilot) == 56);
C_ASSERT(offsetof(CabStreamVTableObservedPilot, release) == 8);
C_ASSERT(offsetof(CabStreamVTableObservedPilot, read) == 0xc);
C_ASSERT(offsetof(CabStreamVTableObservedPilot, seek) == 0x14);
C_ASSERT(offsetof(CabStreamVTableObservedPilot, copyTo) == 0x1c);
C_ASSERT(offsetof(CabStreamVTableObservedPilot, stat) == 0x30);
C_ASSERT(sizeof(CabStreamStatObserved72Pilot) == 72);
C_ASSERT(offsetof(CabStreamStatObserved72Pilot, modified) == 0x10);
C_ASSERT(offsetof(CabStreamStatObserved72Pilot, created) == 0x18);
C_ASSERT(offsetof(CabStreamStatObserved72Pilot, accessed) == 0x20);
// Observed prefix/slots only, not original names or complete interface recovery.
extern "C" CabStreamSourcePilot* __stdcall OpenCabFileStreamSourcePilot(const char*, DWORD, DWORD, void*, DWORD, DWORD, HANDLE);
extern "C" CabStreamSourcePilot* __stdcall OpenCabEntryStreamPilot(const char*);
extern "C" CabStreamSourcePilot* __stdcall WrapCabStreamSourcePilot(DWORD, DWORD, DWORD, DWORD, CabStreamSourcePilot*, DWORD);
extern "C" int __stdcall ReleaseCabStreamPilot(CabStreamSourcePilot*);
extern "C" int __stdcall ReadCabStreamPilot(CabStreamSourcePilot*, void*, DWORD, DWORD*);
extern "C" int __stdcall SeekCabStreamPilot(CabStreamSourcePilot*, DWORD, DWORD);
extern "C" int __stdcall ReadCabStreamTimesPilot(CabStreamSourcePilot*, FILETIME*, FILETIME*, FILETIME*);
