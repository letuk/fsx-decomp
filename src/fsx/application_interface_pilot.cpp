// Application getter resolved dynamically by the EXE startup implementation.
// Separate translation unit for organization; compiled with /GL by default.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "application_interface_pilot.h"
GetApplicationPilotFunc pfnGetApplication = 0;

// Original: fsx.exe 0x010014c8
const char emptyErrorCaption[] = "";
