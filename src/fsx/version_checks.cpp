// Reconstructed names and source, not recovered original source text.
// Evidence: notes/re/fsx.exe/startup.md, fsx.exe 0x01001760 and 0x01001800.
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <string.h>

// FUNCTION: FSX 0x01001760
BOOL __stdcall CheckWindows51SP2OrLater()
{
    OSVERSIONINFOEXA versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
    versionInfo.dwMajorVersion = 5;
    versionInfo.dwMinorVersion = 1;
    versionInfo.wServicePackMajor = 2;
    ULONGLONG conditionMask = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    conditionMask = VerSetConditionMask(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    conditionMask = VerSetConditionMask(conditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    return VerifyVersionInfoA(&versionInfo,
        VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, conditionMask);
}

// FUNCTION: FSX 0x01001800
BOOL __stdcall CheckWindows52SP1OrLater()
{
    OSVERSIONINFOEXA versionInfo;
    memset(&versionInfo, 0, sizeof(versionInfo));
    versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
    versionInfo.dwMajorVersion = 5;
    versionInfo.dwMinorVersion = 2;
    versionInfo.wServicePackMajor = 1;
    ULONGLONG conditionMask = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    conditionMask = VerSetConditionMask(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    conditionMask = VerSetConditionMask(conditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    return VerifyVersionInfoA(&versionInfo,
        VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, conditionMask);
}
