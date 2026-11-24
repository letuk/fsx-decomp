#include <windows.h>
// Preserve observed 32-bit product/sign extension; do not replace with an
// idealized unsigned64 disk-size calculation or add output NULL checks.
extern "C" BOOL WINAPI DiskSpaceFallbackPilot(LPCSTR path,
    PULARGE_INTEGER available, PULARGE_INTEGER total, PULARGE_INTEGER freeBytes)
{
    DWORD bytesPerSector, freeClusters, totalClusters;
    BOOL result = GetDiskFreeSpaceA(path, reinterpret_cast<LPDWORD>(&path),
        &bytesPerSector, &freeClusters, &totalClusters);
    if (result) {
        LONG clusterBytes = static_cast<LONG>(bytesPerSector * reinterpret_cast<DWORD>(path));
        available->QuadPart = static_cast<LONGLONG>(static_cast<LONG>(freeClusters)) * clusterBytes;
        if (freeBytes != NULL)
            *freeBytes = *available;
        total->QuadPart = static_cast<LONGLONG>(static_cast<LONG>(totalClusters)) * clusterBytes;
    }
    return result;
}
