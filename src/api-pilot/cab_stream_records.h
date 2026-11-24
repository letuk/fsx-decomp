#pragma once
#include "cab_stream.h"
// Reconstructed observed storage, not original classes or full interfaces.
struct CabFileStreamObserved276Pilot {
    const CabStreamVTableObservedPilot* vtable;
    LONG references;
    HANDLE file;
    DWORD mode;
    char path[260];
};
struct CabWrappedStreamObserved48Pilot {
    const CabStreamVTableObservedPilot* vtable;
    LONG references;
    void* data;
    DWORD bufferStart, bufferValidBytes, capacity, flags;
    CabStreamSourcePilot* underlying;
    DWORD baseOffset, length, position, bufferFromPool;
};
C_ASSERT(sizeof(CabFileStreamObserved276Pilot) == 0x114);
C_ASSERT(sizeof(CabWrappedStreamObserved48Pilot) == 0x30);
C_ASSERT(offsetof(CabWrappedStreamObserved48Pilot, underlying) == 0x1c);
C_ASSERT(offsetof(CabWrappedStreamObserved48Pilot, position) == 0x28);
extern const CabStreamVTableObservedPilot g_cabFileStreamVTablePilot;
extern const CabStreamVTableObservedPilot g_cabWrappedStreamVTablePilot;
