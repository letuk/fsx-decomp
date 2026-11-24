#include <windows.h>
#include <mbctype.h>

// api.dll 20137e17. The multibyte path reserves one byte for a terminator.
extern "C" char* __stdcall CopyResolvedPathPilot(
    char* destination, const char* source, unsigned int capacity)
{
    if (_getmbcp() == 0)
        return lstrcpynA(destination, source, capacity);
    if (capacity == 0)
        return destination;

    char* cursor = destination;
    unsigned int remaining = capacity - 1;
    while (remaining != 0) {
        const bool lead = _ismbblead(static_cast<signed char>(*source)) != 0;
        const char first = *source++;
        *cursor++ = first;
        --remaining;
        if (lead) {
            if (remaining == 0) {
                cursor[-1] = '\0';
                break;
            }
            const char trail = *source++;
            *cursor++ = trail;
            --remaining;
            if (trail == '\0') {
                cursor[-2] = '\0';
                break;
            }
        } else if (first == '\0') {
            break;
        }
    }
    *cursor = '\0';
    return destination;
}
