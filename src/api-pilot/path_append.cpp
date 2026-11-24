#include <string.h>
extern "C" char* __stdcall CopyResolvedPathPilot(char*, const char*, unsigned int);
extern "C" unsigned int __stdcall NullablePathLengthPilot(const char* text)
{
    return text == 0 ? 0 : static_cast<unsigned int>(strlen(text));
}
// Original20137ec0, observed capacity/NULL behavior retained.
extern "C" char* __stdcall AppendResolvedPathPilot(char* destination,
    const char* source, unsigned int capacity)
{
    unsigned int length = destination == 0 ? 0 : static_cast<unsigned int>(strlen(destination));
    if (length < capacity)
        CopyResolvedPathPilot(destination + length, source, capacity - length);
    return destination;
}
