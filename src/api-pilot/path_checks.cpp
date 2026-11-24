// Reconstructed API export75 wrapper. Names are provisional.
// Original2013341b returns0/-1, stdcall3; downstream dependencies unresolved.
#include <windows.h>
extern "C" int __stdcall ResolvePathPilot(char*, unsigned int, const char*, int);
extern "C" unsigned long __stdcall QueryFallbackAttributesPilot(const char*);
extern "C" int __stdcall QueryPathAttributesPilot(unsigned int*, const char*, int);
extern "C" int __stdcall CheckPathExistsPilot(const char* path, int selector)
{
    if (QueryPathAttributesPilot(reinterpret_cast<unsigned int*>(&selector), path, selector) != 0)
        return -1;
    return selector == -1 ? -1 : 0;
}
extern "C" int __stdcall QueryPathAttributesPilot(unsigned int* output, const char* path, int selector)
{
    char resolved[260];
    if (ResolvePathPilot(resolved, sizeof(resolved), path, selector) != 0)
        return -1;
    unsigned long attributes = GetFileAttributesA(resolved);
    if (attributes == INVALID_FILE_ATTRIBUTES)
        attributes = QueryFallbackAttributesPilot(resolved);
    else if (selector == -2 && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        attributes = QueryFallbackAttributesPilot(resolved);
        if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            return -1;
    }
    if (attributes == INVALID_FILE_ATTRIBUTES)
        return -1;
    *output = 0;
    if ((signed char)attributes < 0) *output = 1;
    if (attributes & 0x20) *output |= 4;
    if (attributes & 1) *output |= 2;
    if (attributes & 2) *output |= 8;
    if (attributes & 0x10) *output |= 0x10;
    return 0;
}
