#include <windows.h>
#include <shlwapi.h>
// Source-level boundaries only. Original selector/copy helpers use custom
// register storage; these standard stack interfaces are NOT ABI-equivalent.
extern "C" int __stdcall SelectSuffixSourceBoundaryPilot(int, const char*, char*);
extern "C" int __stdcall CopyPathSourceBoundaryPilot(const char*, char*, unsigned int);
extern "C" char* __stdcall CopyResolvedPathPilot(char*, const char*, unsigned int);
extern "C" char g_resolverBasePathPilot[];

extern "C" int __stdcall ResolvePathPilot(char* output, unsigned int capacity,
                                         const char* input, int selector)
{
    char combined[260];
    char selected[260];
    char canonical[260];
    if (selector != 0) {
        if (SelectSuffixSourceBoundaryPilot(selector, input, selected) != 0)
            return -1;
        input = selected;
    } else if (input == NULL)
        return -1;
    if (PathIsRelativeA(input)) {
        CopyPathSourceBoundaryPilot(g_resolverBasePathPilot, canonical, 260);
        if (PathCombineA(combined, canonical, input) == NULL)
            return -1;
        input = combined;
    }
    if (capacity < 260 || !PathCanonicalizeA(canonical, input))
        return -1;
    CopyResolvedPathPilot(output, canonical, capacity);
    return 0;
}
