#include "heap_boundary.h"

// Original: api.dll 0x200f40a9
extern "C" __declspec(noinline) void __cdecl FreeOwnedPrivateHeapPointerPilot(void* pointer)
{
    if (pointer != NULL)
        HeapFree(g_privateHeapPilot, 0, pointer);
}

#pragma optimize("s", on)
// Original: api.dll 0x2014e744
extern "C" void* __stdcall AllocateProcessHeapStoragePilot(SIZE_T bytes)
{
    return HeapAlloc(GetProcessHeap(), 0, bytes);
}

// Original: api.dll 0x2014e75f
extern "C" __declspec(noinline) BOOL __stdcall FreeProcessHeapStoragePilot(void* pointer)
{
    return HeapFree(GetProcessHeap(), 0, pointer);
}

// Original: api.dll 0x20150c15
extern "C" __declspec(noinline) HRESULT __stdcall FreePrefixedBufferStoragePilot(void* pointer)
{
    if (pointer != NULL)
        FreeProcessHeapStoragePilot(static_cast<BYTE*>(pointer) - sizeof(DWORD));
    return S_OK;
}
#pragma optimize("", on)
