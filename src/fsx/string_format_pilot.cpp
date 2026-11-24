// SDK provenance experiment for fsx.exe 0x01001590 (92 bytes).
// The original header revision and Cch/Cb spelling are not established.
#define WIN32_LEAN_AND_MEAN
#define STRSAFE_NO_DEPRECATE
// Select the SDK's documented-in-header legacy formatting branch, matching
// the original _vsnprintf import; do not patch the SDK implementation.
#define _STRSAFE_USE_SECURE_CRT 0
#include <windows.h>
#include <strsafe.h>

volatile HRESULT g_pilotFormatStatus;

// Pilot-only caller context; no reconstructed original function-name claim.
__declspec(noinline) void RunFormatterPilot(const char* format, const char* detail,
                                          unsigned int errorId)
{
    char message[1024];
    g_pilotFormatStatus = StringCchPrintfA(message, 1024, format, detail);
    g_pilotFormatStatus = StringCchPrintfA(message, 1024,
                                         "Error ID = %d\r\n%s", errorId, detail);
}
