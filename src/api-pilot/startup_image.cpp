#include <windows.h>
extern "C" char g_resolverBasePathPilot[];
extern "C" int __stdcall CopyPathSourceBoundaryPilot(const char*, char*, unsigned int);
extern "C" char* __stdcall CopyResolvedPathPilot(char*, const char*, unsigned int);
extern "C" char* __stdcall AppendResolvedPathPilot(char*, const char*, unsigned int);
extern "C" unsigned int __stdcall NullablePathLengthPilot(const char*);
// Original trim helper20135114 takes its buffer in EAX, not on the stack.
extern "C" void __stdcall TrimTrailingSlashSourceBoundaryPilot(char*);
extern "C" int __stdcall LoadSplashStringPilot(unsigned int, char*, int);
extern "C" HGDIOBJ __cdecl CreateStartupImagePilot()
{
    char imagePath[260];
    char combined[260] = {0};
    char resourceName[256] = {0};
    const char* resource = "uires\\dlgsplash.bmp";
    CopyPathSourceBoundaryPilot(g_resolverBasePathPilot, imagePath, 260);
    if ((reinterpret_cast<ULONG_PTR>(resource) & 0xffff0000u) == 0) {
        LoadSplashStringPilot(static_cast<unsigned int>(reinterpret_cast<ULONG_PTR>(resource) & 0xffffu), resourceName, 256);
        resource = resourceName;
    }
    if (resource[1] == ':' || (resource[0] == '\\' && resource[1] == '\\')) {
        CopyResolvedPathPilot(imagePath, resource, 260);
    } else {
        if (NullablePathLengthPilot(imagePath) != 0) {
            AppendResolvedPathPilot(combined, imagePath, 260);
            if (NullablePathLengthPilot(combined) == 1)
                AppendResolvedPathPilot(combined, ":", 260);
            TrimTrailingSlashSourceBoundaryPilot(combined);
        }
        if (NullablePathLengthPilot(resource) != 0) {
            if (resource[0] != '\\' && NullablePathLengthPilot(combined) != 0)
                AppendResolvedPathPilot(combined, "\\", 260);
            AppendResolvedPathPilot(combined, resource, 260);
        }
        CopyResolvedPathPilot(imagePath, combined, 260);
    }
    return LoadImageA(NULL, imagePath, IMAGE_BITMAP, 0, 0, 0x2010);
}
