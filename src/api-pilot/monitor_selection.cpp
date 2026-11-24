#include <windows.h>
#include <stddef.h>
#include <string.h>
// Reconstructed184-byte callback context, not an original type/class name.
struct MonitorSelectionContextPilot {
    char requestedDevice[32];
    DWORD primarySeen;
    DWORD pixelMode;
    MONITORINFOEXA fallback;
    MONITORINFOEXA selected;
};
C_ASSERT(sizeof(MONITORINFOEXA) == 72);
C_ASSERT(offsetof(MonitorSelectionContextPilot, primarySeen) == 0x20);
C_ASSERT(offsetof(MonitorSelectionContextPilot, pixelMode) == 0x24);
C_ASSERT(offsetof(MonitorSelectionContextPilot, fallback) == 0x28);
C_ASSERT(offsetof(MonitorSelectionContextPilot, selected) == 0x70);
C_ASSERT(sizeof(MonitorSelectionContextPilot) == 184);
extern "C" BOOL CALLBACK EnumerateStartupMonitorPilot(HMONITOR monitor, HDC dc,
                                                       LPRECT, LPARAM parameter)
{
    MonitorSelectionContextPilot* context = reinterpret_cast<MonitorSelectionContextPilot*>(parameter);
    MONITORINFOEXA info;
    info.cbSize = sizeof(info);
    GetMonitorInfoA(monitor, reinterpret_cast<LPMONITORINFO>(&info));
    if (context->pixelMode == 1) {
        int planes = GetDeviceCaps(dc, PLANES);
        if (planes * GetDeviceCaps(dc, BITSPIXEL) == 24) {
            context->pixelMode = 2;
            return FALSE;
        }
    }
    if ((info.dwFlags & MONITORINFOF_PRIMARY) != 0 || context->fallback.cbSize == 0) {
        if ((info.dwFlags & MONITORINFOF_PRIMARY) != 0)
            context->primarySeen = 1;
        if (info.rcMonitor.right - info.rcMonitor.left >= 1024 &&
            info.rcMonitor.bottom - info.rcMonitor.top >= 720)
            context->fallback = info;
    }
    if (context->requestedDevice[0] == '\0') {
        if (context->primarySeen != 0 && context->fallback.cbSize != 0 && context->pixelMode != 1)
            return FALSE;
    } else if (lstrcmpA(info.szDevice, context->requestedDevice) == 0) {
        context->requestedDevice[0] = '\0';
        if (info.rcMonitor.right - info.rcMonitor.left >= 1024 &&
            info.rcMonitor.bottom - info.rcMonitor.top >= 720) {
            context->selected = info;
            if (context->pixelMode != 1)
                return FALSE;
        }
    }
    return TRUE;
}
extern "C" int __stdcall SelectStartupMonitorsPilot(MONITORINFOEXA* requested,
                                                     MONITORINFOEXA* fallback)
{
    MonitorSelectionContextPilot context = {0};
    if (requested->cbSize != 0) {
        memcpy(context.requestedDevice, requested->szDevice, 32);
        requested->cbSize = 0;
    }
    HDC dc = GetDC(NULL);
    if (dc == NULL) return -1;
    if (GetSystemMetrics(0x51) == 0)
        context.pixelMode = 1;
    else {
        int planes = GetDeviceCaps(dc, PLANES);
        context.pixelMode = planes * GetDeviceCaps(dc, BITSPIXEL) == 24 ? 2 : 0;
    }
    EnumDisplayMonitors(dc, NULL, EnumerateStartupMonitorPilot, reinterpret_cast<LPARAM>(&context));
    ReleaseDC(NULL, dc);
    if (context.pixelMode == 2) return 0xb0001;
    *requested = context.selected;
    *fallback = context.fallback;
    if (requested->cbSize != 72 && fallback->cbSize != 72) return 0xb0002;
    return 0;
}
