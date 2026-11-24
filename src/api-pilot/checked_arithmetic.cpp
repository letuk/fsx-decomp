#include <windows.h>

#pragma optimize("s", on)
// Original: api.dll 0x2014e674
extern "C" HRESULT __stdcall AddUnsignedCheckedPilot(DWORD left, DWORD right, DWORD* result)
{
    const DWORD sum = left + right;
    HRESULT status;
    if (sum >= left) {
        *result = sum;
        status = S_OK;
    } else {
        status = HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
    }
    return status;
}

// Original: api.dll 0x2015347a
extern "C" HRESULT __stdcall SubtractUnsignedCheckedPilot(DWORD left, DWORD right, DWORD* result)
{
    const DWORD difference = left - right;
    HRESULT status;
    if (difference <= left) {
        *result = difference;
        status = S_OK;
    } else {
        status = HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
    }
    return status;
}

// Original: api.dll 0x20171131
extern "C" __declspec(noinline) HRESULT __stdcall SubtractUnsigned64CheckedPilot(
    ULONGLONG left, ULONGLONG right, ULONGLONG* result)
{
    const ULONGLONG difference = left - right;
    HRESULT status;
    if (difference <= left) {
        *result = difference;
        status = S_OK;
    } else {
        status = HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
    }
    return status;
}

// Original: api.dll 0x201711d2
extern "C" HRESULT __stdcall ForwardUnsigned64SubtractionPilot(ULONGLONG left, ULONGLONG right, ULONGLONG* result)
{
    return SubtractUnsigned64CheckedPilot(left, right, result);
}

// Original: api.dll 0x201513ba. The second stack argument is unused.
extern "C" HRESULT __stdcall CheckNonnegativeLongPilot(LONG value, LONG)
{
    HRESULT status = S_OK;
    if (value < 0)
        status = HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
    return status;
}
#pragma optimize("", on)

// Original: api.dll 0x20153784
extern "C" int __stdcall CompareUnsigned32Pilot(const DWORD* left, const DWORD* right)
{
    const DWORD rightValue = *right;
    const DWORD leftValue = *left;
    return static_cast<int>(leftValue > rightValue) - static_cast<int>(leftValue < rightValue);
}
