#include "cab_stream_records.h"
#include <objbase.h>
#include <string.h>

// 20124356, RET16. A successful short read still returns0.
extern "C" int __stdcall ReadCabFileMethodPilot(CabFileStreamObserved276Pilot* stream,
    void* output, DWORD requested, DWORD* readCount)
{
    DWORD ignored;
    if (readCount == NULL) readCount = &ignored;
    return ReadFile(stream->file, output, requested, readCount, NULL) ? 0 : 0x80004005;
}
// 201243c8, RET20. High offset ignored; FFFFFFFF is always failure here.
extern "C" int __stdcall SeekCabFileMethodPilot(CabFileStreamObserved276Pilot* stream,
    DWORD offsetLow, DWORD, DWORD origin, ULARGE_INTEGER* output)
{
    DWORD position = SetFilePointer(stream->file, static_cast<LONG>(offsetLow), NULL, origin);
    if (position == 0xffffffff) return 0x80004005;
    if (output != NULL) { output->LowPart = position; output->HighPart = 0; }
    return 0;
}
// 20124408, RET12. Allocation/conversion failure does not itself change result.
extern "C" int __stdcall StatCabFileMethodPilot(CabFileStreamObserved276Pilot* stream,
    CabStreamStatObserved72Pilot* output, DWORD flags)
{
    int result = 0;
    memset(output, 0, 72);
    output->type = 2;
    output->size.LowPart = GetFileSize(stream->file, &output->size.HighPart);
    if (output->size.LowPart == 0xffffffff && GetLastError() != 0) result = 0x80004005;
    if (GetFileTime(stream->file, &output->created, &output->accessed, &output->modified) == 0)
        result = 0x80004005;
    output->mode = stream->mode;
    if ((flags & 1) == 0) {
        const char* name = stream->path;
        if (name == NULL) output->name = NULL;
        else {
            int length = lstrlenA(name);
            output->name = static_cast<LPWSTR>(CoTaskMemAlloc(length * 2 + 2));
            if (output->name != NULL) MultiByteToWideChar(0, 0, name, -1, output->name, length + 1);
        }
    }
    return result;
}

// 20126dcf, RET16. Preserve original partial-read, zero-progress and flag paths.
extern "C" int __stdcall ReadCabWrappedMethodPilot(CabWrappedStreamObserved48Pilot* stream,
    void* outputInput, DWORD requested, DWORD* readCount)
{
    DWORD ignored;
    if (readCount == NULL) readCount = &ignored;
    *readCount = 0;
    BYTE* output = static_cast<BYTE*>(outputInput);
    if (requested == 0) return 0;
    for (;;) {
        if (stream->bufferStart <= stream->position &&
            stream->position < stream->bufferStart + stream->bufferValidBytes) {
            DWORD copied = stream->bufferValidBytes - stream->position + stream->bufferStart;
            if (requested <= copied) copied = requested;
            memcpy(output, static_cast<BYTE*>(stream->data) + stream->position - stream->bufferStart, copied);
            stream->position += copied;
            output += copied;
            requested -= copied;
            *readCount += copied;
        }
        if (stream->underlying == NULL) requested = 0;
        if (stream->length <= stream->position) requested = 0;
        if ((stream->flags & 0x20000000) != 0 || stream->capacity * 2 < requested) {
            stream->underlying->vtable->seek(stream->underlying, stream->baseOffset + stream->position, 0, 0, NULL);
            DWORD copied;
            int result = stream->underlying->vtable->read(stream->underlying, output, requested, &copied);
            if (result < 0) return result;
            output += copied;
            stream->position += copied;
            requested -= copied;
            *readCount += copied;
        }
        if (stream->length <= stream->position || requested == 0) return 0;
        stream->bufferValidBytes = 0;
        stream->bufferStart = stream->position - stream->position % stream->capacity;
        stream->underlying->vtable->seek(stream->underlying, stream->baseOffset + stream->bufferStart, 0, 0, NULL);
        int result = stream->underlying->vtable->read(stream->underlying, stream->data,
            stream->capacity, &stream->bufferValidBytes);
        if (result < 0) return result;
    }
}
// 20126fc0, RET20. All position arithmetic wraps32, signed range tests follow.
extern "C" int __stdcall SeekCabWrappedMethodPilot(CabWrappedStreamObserved48Pilot* stream,
    DWORD offsetLow, DWORD, DWORD origin, ULARGE_INTEGER* output)
{
    DWORD position;
    switch (origin) {
    case 0: position = offsetLow; break;
    case 1: position = stream->position + offsetLow; break;
    case 2: position = stream->length + offsetLow; break;
    default: return 0x80004005;
    }
    if (static_cast<LONG>(position) < 0) position = 0;
    else if (static_cast<LONG>(position) > static_cast<LONG>(stream->length)) return 0x80004005;
    stream->position = position;
    if (output != NULL) { output->LowPart = position; output->HighPart = 0; }
    return 0;
}
// 2012701c, RET12. Forwarded failure ignored; high size DWORD left as returned.
extern "C" int __stdcall StatCabWrappedMethodPilot(CabWrappedStreamObserved48Pilot* stream,
    CabStreamStatObserved72Pilot* output, DWORD flags)
{
    if (stream->underlying != NULL) {
        stream->underlying->vtable->stat(stream->underlying, output, flags);
        output->size.LowPart = stream->length;
        return 0;
    }
    memset(output, 0, 72);
    output->type = 3;
    output->size.LowPart = stream->bufferValidBytes;
    output->mode = ~(stream->flags * 2) & 2;
    return 0;
}
