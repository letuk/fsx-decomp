#include <string.h>

// Original: api.dll 0x20137fd2 (ordinal 160)
extern "C" int __stdcall CompareStringsPilot(const char* left, const char* right)
{
    return strcmp(left, right);
}
