#include <windows.h>
#include <stdio.h>
extern "C" char g_configurationFilePilot[];
extern "C" char g_savedWindowLocationPilot[];
extern "C" int __stdcall SelectStartupMonitorsPilot(MONITORINFOEXA*, MONITORINFOEXA*);
extern "C" unsigned int __stdcall HandleGeometryErrorPilot(unsigned int status);
// Original monitor is in ECX, output RECT on stack/RET4. This source boundary
// uses ordinary stack arguments; it is not an ABI-equivalent original caller.
extern "C" int __stdcall ResolveWindowGeometrySourceBoundaryPilot(HMONITOR monitor, RECT* output)
{
    RECT bounds;
    MONITORINFOEXA requested, fallback;
    MONITORINFOEXA* selected = &requested;
    GetPrivateProfileStringA("MAIN", "Location", "", g_savedWindowLocationPilot,
                             260, g_configurationFilePilot);
    requested.cbSize = sizeof(requested);
    if (sscanf_s(g_savedWindowLocationPilot, "%d,%d,%d,%d,%s",
        &bounds.left, &bounds.top, &bounds.right, &bounds.bottom,
        requested.szDevice, 32u) != 5) {
        SetRectEmpty(&bounds);
        if (monitor == NULL) requested.cbSize = 0;
        else GetMonitorInfoA(monitor, reinterpret_cast<LPMONITORINFO>(&requested));
    }
    fallback.cbSize = 0;
    int status = SelectStartupMonitorsPilot(&requested, &fallback);
    if (status != 0) return status;
    bool keepSavedBounds = false;
    if (requested.cbSize == 0) {
        selected = &fallback;
    } else {
        OffsetRect(&bounds, requested.rcWork.left, requested.rcWork.top);
        RECT virtualScreen;
        SetRect(&virtualScreen, GetSystemMetrics(0x4c), GetSystemMetrics(0x4d),
                GetSystemMetrics(0x4e), GetSystemMetrics(0x4f));
        POINT origin = {bounds.left, bounds.top};
        keepSavedBounds = PtInRect(&virtualScreen, origin) != FALSE;
        if (!keepSavedBounds && (requested.dwFlags & MONITORINFOF_PRIMARY) == 0 &&
            fallback.cbSize == 72 && (fallback.dwFlags & MONITORINFOF_PRIMARY) != 0)
            selected = &fallback;
    }
    if (!keepSavedBounds) SetRectEmpty(&bounds);
    if (IsRectEmpty(&bounds)) {
        int monitorWidth = selected->rcMonitor.right - selected->rcMonitor.left;
        int monitorHeight = selected->rcMonitor.bottom - selected->rcMonitor.top;
        int width = 1024, height = 720;
        if (monitorWidth <= 1024) {
            width = monitorWidth * 3 / 4;
            height = monitorHeight * 3 / 4;
        }
        bounds.left = (monitorWidth - width) / 2 + selected->rcWork.left;
        bounds.top = (monitorHeight - height) / 2 + selected->rcWork.top;
        bounds.right = bounds.left + width;
        bounds.bottom = bounds.top + height;
        AdjustWindowRect(&bounds, 0xcf0000, TRUE);
    }
    *output = bounds;
    return 0;
}

// Monitor geometry phase of 2012205d at 20122412.
extern "C" BOOL __stdcall ResolveMainWindowStartupBoundsPilot(HMONITOR monitor, RECT* output)
{
    for (;;) {
        int status = ResolveWindowGeometrySourceBoundaryPilot(monitor, output);
        if (status == 0) return TRUE;
        if (HandleGeometryErrorPilot(static_cast<unsigned int>(status)) != 0) return FALSE;
    }
}
