// Provenance experiment for fsx.exe FUN_01001600 (0x01001600, 72 bytes).
// StringCchCopyA identity/version is a hypothesis, not an original-name claim.
// Use the installed period SDK implementation rather than transcribing assembly.
#define WIN32_LEAN_AND_MEAN
#define STRSAFE_NO_DEPRECATE
#include <windows.h>
#include <strsafe.h>

// Pilot-only noinline boundary keeps the candidate available for comparison.
// It does not establish a corresponding original source-level wrapper.
__declspec(noinline) HRESULT CopyErrorText_Pilot(char* destination, const char* source)
{
    return StringCchCopyA(destination, 1024, source);
}
