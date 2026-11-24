#include <windows.h>
typedef BOOL (WINAPI *DiskSpaceQueryPilot)(LPCSTR, PULARGE_INTEGER,
                                          PULARGE_INTEGER, PULARGE_INTEGER);
extern "C" DiskSpaceQueryPilot g_diskSpaceQueryPilot;
extern "C" BOOL WINAPI DiskSpaceFallbackPilot(LPCSTR, PULARGE_INTEGER,
                                               PULARGE_INTEGER, PULARGE_INTEGER);
extern "C" BOOL WINAPI ResolveDiskSpaceQueryPilot(LPCSTR path,
    PULARGE_INTEGER available, PULARGE_INTEGER total, PULARGE_INTEGER freeBytes)
{
    HMODULE module = GetModuleHandleA("KERNEL32");
    DiskSpaceQueryPilot query = reinterpret_cast<DiskSpaceQueryPilot>(
        GetProcAddress(module, "GetDiskFreeSpaceExA"));
    if (query != NULL) {
        BOOL result = query(path, available, total, freeBytes);
        if (!result && GetLastError() == ERROR_CALL_NOT_IMPLEMENTED) {
            g_diskSpaceQueryPilot = DiskSpaceFallbackPilot;
            return DiskSpaceFallbackPilot(path, available, total, freeBytes);
        }
        g_diskSpaceQueryPilot = query;
        return result;
    }
    g_diskSpaceQueryPilot = DiskSpaceFallbackPilot;
    return DiskSpaceFallbackPilot(path, available, total, freeBytes);
}
