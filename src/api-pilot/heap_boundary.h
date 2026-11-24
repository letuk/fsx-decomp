#pragma once
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <malloc.h>
extern HANDLE g_privateHeapPilot;
extern int g_heapInitializedPilot;
extern int g_heapInitCallCountPilot;
extern HINSTANCE g_errorResourceInstancePilot;
extern char g_heapErrorTextPilot[260];
__declspec(dllexport) int __cdecl InitializeHeapManagerPilot();
__declspec(dllexport) int __stdcall AllocatePrivateHeapMemoryPilot(int failureMode, SIZE_T sizeBytes, void** outMemory, int zeroInitialize);
__declspec(dllexport) int __stdcall ReallocatePrivateHeapMemoryBoundaryPilot(int failureMode, SIZE_T sizeBytes, void** inOutMemory);
