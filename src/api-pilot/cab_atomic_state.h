#pragma once
#include <windows.h>

struct CabSlotAllocatorObservedPilot {
    DWORD alignment;
    void* context;
    void* (__stdcall *allocate)(void*, DWORD);
    void (__stdcall *release)(void*, void*);
};

extern volatile signed char g_cabSlotAtomicModePilot;
extern "C" void* __stdcall AllocateCabSlotAlignedSourcePilot(CabSlotAllocatorObservedPilot*, DWORD, void**, LONG*);
void InitializeCabSlotAtomicMode();
void FinishCabSlotMutation();
bool CompareExchangeCabPair(void* address, DWORD& low, DWORD& high, DWORD nextLow, DWORD nextHigh, BOOL locked);
