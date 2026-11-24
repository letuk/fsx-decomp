#include "cab_stream.h"
// Original 201333ac. NULL -> -1; ignore release result and return0 otherwise.
extern "C" int __stdcall ReleaseCabStreamPilot(CabStreamSourcePilot* stream)
{
    if (stream == NULL) return -1;
    stream->vtable->release(stream);
    return 0;
}
// Original 201333cd.
extern "C" int __stdcall ReadCabStreamPilot(CabStreamSourcePilot* stream,
    void* output, DWORD length, DWORD* readCount)
{
    int result = -1;
    if (stream != NULL) result = stream->vtable->read(stream, output, length, readCount);
    return result;
}
// Original 20133592. Offset high DWORD and new-position output are zero/NULL.
extern "C" int __stdcall SeekCabStreamPilot(CabStreamSourcePilot* stream,
    DWORD offsetLow, DWORD origin)
{
    if (stream == NULL) return -1;
    return stream->vtable->seek(stream, offsetLow, 0, origin, NULL);
}
// Original 20135378. Outputs untouched on failure.
extern "C" int __stdcall ReadCabStreamTimesPilot(CabStreamSourcePilot* stream,
    FILETIME* created, FILETIME* accessed, FILETIME* modified)
{
    if (stream != NULL) {
        CabStreamStatObserved72Pilot status;
        if (stream->vtable->stat(stream, &status, 1) == 0) {
            if (created != NULL) *created = status.created;
            if (accessed != NULL) *accessed = status.accessed;
            if (modified != NULL) *modified = status.modified;
            return 0;
        }
    }
    return -1;
}
