// Comparison harness only. This is not FSX startup code.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "error_callback_pilot.h"

BOOL __stdcall CheckWindows51SP2OrLater();
BOOL __stdcall CheckWindows52SP1OrLater();
HRESULT CopyErrorText_Pilot(char* destination, const char* source);
void RunFormatterPilot(const char* format, const char* detail, unsigned int errorId);
volatile BOOL g_pilot51;
volatile BOOL g_pilot52;
volatile HRESULT g_pilotCopyStatus;

int main()
{
    char message[1024];
    g_pilot51 = CheckWindows51SP2OrLater();
    g_pilot52 = CheckWindows52SP1OrLater();
    g_pilotCopyStatus = CopyErrorText_Pilot(message, GetCommandLineA());
    const char* format = (GetTickCount() & 1) ? "%s" : "Detail: %s";
    RunFormatterPilot(format, GetCommandLineA(), GetLastError());
    return 0;
}
