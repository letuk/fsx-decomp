#include <windows.h>
#include <string.h>
extern "C" int __stdcall CopyPathSourceBoundaryPilot(const char*, char*, unsigned int);
extern "C" int __stdcall ResolvePathPilot(char*, unsigned int, const char*, int);
extern "C" char* __stdcall CopyResolvedPathPilot(char*, const char*, unsigned int);
extern "C" int __stdcall SplitVolumePathPilot(const char*, char*, unsigned int,
    char*, unsigned int, char*, unsigned int, unsigned int*);
extern "C" char g_resolverBasePathPilot[];
extern "C" BOOL (WINAPI *g_diskSpaceQueryPilot)(LPCSTR, PULARGE_INTEGER,
    PULARGE_INTEGER, PULARGE_INTEGER);
extern "C" int __stdcall QueryVolumeSpacePilot(const char* path,
    PULARGE_INTEGER totalOutput, PULARGE_INTEGER availableOutput)
{
    char resolved[260];
    char root[260];
    ULARGE_INTEGER available, total;
    if (path == NULL)
        CopyPathSourceBoundaryPilot(g_resolverBasePathPilot, resolved, 260);
    else
        ResolvePathPilot(resolved, 260, path, 0);
    SplitVolumePathPilot(resolved, root, 256, NULL, 0, NULL, 0, NULL);
    unsigned int length = static_cast<unsigned int>(strlen(root));
    if (length < 256)
        CopyResolvedPathPilot(root + length, "\\", 256 - length);
    if (totalOutput != NULL) {
        totalOutput->HighPart = 0;
        totalOutput->LowPart = 0;
    }
    if (availableOutput != NULL) {
        availableOutput->HighPart = 0;
        availableOutput->LowPart = 0;
    }
    if (!g_diskSpaceQueryPilot(root, &available, &total, NULL))
        return -1;
    if (totalOutput != NULL) *totalOutput = total;
    if (availableOutput != NULL) *availableOutput = available;
    return 0;
}
