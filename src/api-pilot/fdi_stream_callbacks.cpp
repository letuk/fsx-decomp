#include <windows.h>
#include <objidl.h>
// IStream-ABI-compatible handle model. Original concrete class/COM IID unknown.
__declspec(dllexport) UINT __cdecl FdiReadStreamBoundaryPilot(INT_PTR handle, void* buffer, UINT count)
{
    reinterpret_cast<IStream*>(handle)->Read(buffer, count, reinterpret_cast<ULONG*>(&count));
    return count;
}
__declspec(dllexport) UINT __cdecl FdiWriteStreamBoundaryPilot(INT_PTR handle, void* buffer, UINT count)
{
    reinterpret_cast<IStream*>(handle)->Write(buffer, count, reinterpret_cast<ULONG*>(&count));
    return count;
}
__declspec(dllexport) int __cdecl FdiCloseStreamBoundaryPilot(INT_PTR handle)
{
    reinterpret_cast<IStream*>(handle)->Release();
    return 1;
}
__declspec(dllexport) long __cdecl FdiSeekStreamBoundaryPilot(INT_PTR handle, long distance, int origin)
{
    LARGE_INTEGER position;
    // Observed high DWORD is zero, including for negative low-word bit patterns.
    position.LowPart = static_cast<DWORD>(distance);
    position.HighPart = 0;
    reinterpret_cast<IStream*>(handle)->Seek(position, origin,
        reinterpret_cast<ULARGE_INTEGER*>(&position));
    return position.LowPart;
}
