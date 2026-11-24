#include <windows.h>
#include <mbctype.h>
#ifdef COPY_RETURN_BARRIER
#include <intrin.h>
#pragma intrinsic(_ReadWriteBarrier)
#endif

extern "C" char* __stdcall CopyResolvedPathPilot(
#ifdef COPY_PARAMETER_CURSOR
#ifdef COPY_CAPACITY_PARAMETER
    char* cursor, const char* source, unsigned int remaining)
#else
    char* cursor, const char* source, unsigned int capacity)
#endif
#else
    char* destination, const char* source, unsigned int capacity)
#endif
{
#ifdef COPY_CAPACITY_PARAMETER
    const unsigned int capacity = remaining;
#endif
#ifdef COPY_SHARED_CURSOR
#ifdef COPY_PARAMETER_CURSOR
    char* const destination = cursor;
#else
    char* cursor = destination;
#endif
#endif
    if (_getmbcp() == 0) {
#ifdef COPY_RETURN_BARRIER
#ifdef COPY_SHARED_CURSOR
        char* result = lstrcpynA(cursor, source, capacity);
#else
        char* result = lstrcpynA(destination, source, capacity);
#endif
        _ReadWriteBarrier();
        return result;
#else
        return lstrcpynA(destination, source, capacity);
#endif
    }
#ifndef COPY_SHARED_CURSOR
    char* cursor = destination;
#endif
#ifdef COPY_SHARED_ZERO_RETURN
    if (capacity != 0) {
#else
    if (capacity == 0)
        return destination;
#endif
#ifdef COPY_CAPACITY_PARAMETER
    --remaining;
#else
    unsigned int remaining = capacity - 1;
#endif
    if (remaining != 0) {
        do {
            --remaining;
            const int lead = _ismbblead(static_cast<signed char>(*source));
#ifndef COPY_BRANCH_STORES
            const char first = *source;
            *cursor = first;
#endif
            if (lead != 0) {
#ifdef COPY_BRANCH_STORES
                *cursor = *source;
#endif
#ifdef COPY_TRAIL_ADVANCE
#ifdef COPY_DELAY_LEAD_ADVANCE
#if defined(COPY_SOURCE_BEFORE_CAPACITY)
                ++source;
#elif defined(COPY_DESTINATION_BEFORE_CAPACITY)
                ++cursor;
#endif
                if (remaining == 0) {
#ifdef COPY_DESTINATION_BEFORE_CAPACITY
                    cursor[-1] = '\0';
                    *cursor = '\0';
#else
                    *cursor = '\0';
                    cursor[1] = '\0';
#endif
                    return destination;
                }
#ifndef COPY_DESTINATION_BEFORE_CAPACITY
                ++cursor;
#endif
#ifndef COPY_SOURCE_BEFORE_CAPACITY
                ++source;
#endif
#else
                ++cursor;
                ++source;
                if (remaining == 0) {
                    cursor[-1] = '\0';
                    *cursor = '\0';
                    return destination;
                }
#endif
                const char trail = *source;
                *cursor = trail;
                --remaining;
                ++cursor;
                ++source;
                if (trail == '\0') {
                    cursor[-2] = '\0';
#ifdef COPY_SHARE_NUL
                    break;
#else
                    *cursor = '\0';
                    return destination;
#endif
                }
#else
                if (remaining == 0) {
                    *cursor = '\0';
                    cursor[1] = '\0';
                    return destination;
                }
                const char trail = source[1];
                cursor[1] = trail;
                --remaining;
                if (trail == '\0') {
                    *cursor = '\0';
                    cursor[2] = '\0';
                    return destination;
                }
                source += 2;
                cursor += 2;
#endif
            } else {
#ifdef COPY_BRANCH_STORES
                const char first = *source;
                *cursor = first;
#endif
#ifdef COPY_SEPARATE_SINGLE_EXIT
#ifdef COPY_SHARE_NUL
#if defined(COPY_DELAY_LEAD_ADVANCE)
                ++cursor;
                ++source;
                if (first == '\0')
                    break;
#elif defined(COPY_STAGGER_NUL)
                ++cursor;
                if (first == '\0')
                    break;
                ++source;
#else
                if (first == '\0') {
                    ++cursor;
                    break;
                }
                ++source;
                ++cursor;
#endif
#else
                if (first == '\0') {
                    cursor[1] = '\0';
                    return destination;
                }
                ++source;
                ++cursor;
#endif
#else
                ++source;
                ++cursor;
                if (first == '\0')
                    break;
#endif
            }
        } while (remaining != 0);
    }
    *cursor = '\0';
#ifdef COPY_SHARED_ZERO_RETURN
    }
#endif
    return destination;
}
