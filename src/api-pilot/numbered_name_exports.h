#pragma once

// Select the VC8 SDK path that calls the original CRT import, _vsnprintf.
#define _STRSAFE_USE_SECURE_CRT 0
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#undef STRSAFE_NO_DEPRECATE
#undef _STRSAFE_USE_SECURE_CRT

// api.dll ordinals 207 and 208 return the SDK formatter's HRESULT.
extern "C" int __stdcall FormatNumberedNamePilot(
    const char* prefix, int number, char* output, unsigned int capacity)
{
    return StringCchPrintfA(output, capacity, "%s.%d", prefix, number);
}

extern "C" int __stdcall FormatDoubleNumberedNamePilot(
    const char* prefix, int number, int subnumber, char* output, unsigned int capacity)
{
    return StringCchPrintfA(output, capacity, "%s.%d.%d", prefix, number, subnumber);
}
