#include "cab_stream_records.h"
#include "heap_boundary.h"
#include <new>

// Reconstructed observed records; unresolved original pool/VTable definitions.
extern "C" CabFileStreamObserved276Pilot* __cdecl AllocateCabFileStreamRecordPilot(); // 20124903
extern "C" CabWrappedStreamObserved48Pilot* __cdecl AllocateCabWrapperRecordPilot(); // 20127a04
extern "C" void* __stdcall AllocateCabBufferSourcePilot(DWORD); // 20127642: EAX input
extern "C" void __stdcall ReturnCabBufferPilot(void*); // 2012772b
extern "C" void __stdcall ReturnCabWrapperRecordPilot(CabWrappedStreamObserved48Pilot*); // 20127c12
extern "C" void __stdcall ReturnFailedCabWrapperSourcePilot(void*); // 20126bd5: EAX input
extern "C" int __stdcall CopyPathSourceBoundaryPilot(const char*, char*, unsigned int);

// Original201246c4, RET28. A successful handle is not closed if pool returnsNULL.
extern "C" CabStreamSourcePilot* __stdcall OpenCabFileStreamSourcePilot(const char* path,
    DWORD access, DWORD sharing, void* security, DWORD disposition, DWORD attributes, HANDLE templateFile)
{
    HANDLE file = CreateFileA(path, access, sharing,
        static_cast<LPSECURITY_ATTRIBUTES>(security), disposition, attributes, templateFile);
    if (file == INVALID_HANDLE_VALUE) return NULL;
    DWORD mode;
    if ((access & GENERIC_WRITE) != 0 && static_cast<LONG>(access) < 0) mode = 2;
    else mode = (access & GENERIC_WRITE) != 0;
    CabFileStreamObserved276Pilot* stream = AllocateCabFileStreamRecordPilot();
    if (stream == NULL) return NULL;
    stream->vtable = &g_cabFileStreamVTablePilot;
    stream->references = 1;
    stream->file = file;
    stream->mode = mode;
    CopyPathSourceBoundaryPilot(path == NULL ? "" : path, stream->path, 260);
    return reinterpret_cast<CabStreamSourcePilot*>(stream);
}

// Original20126be0: ESI receiver, EAX capacity, EDX underlying; four stack args.
extern "C" CabWrappedStreamObserved48Pilot* __stdcall InitializeCabWrappedStreamSourcePilot(
    CabWrappedStreamObserved48Pilot* stream, DWORD capacity, CabStreamSourcePilot* underlying,
    DWORD length, DWORD flags, void* data, DWORD baseOffset)
{
    stream->flags = flags;
    stream->baseOffset = baseOffset;
    stream->vtable = &g_cabWrappedStreamVTablePilot;
    stream->references = 1;
    stream->data = data;
    stream->bufferStart = 0;
    stream->bufferValidBytes = capacity;
    stream->capacity = capacity;
    stream->underlying = underlying;
    stream->length = length;
    stream->position = 0;
    if (data == NULL) {
        if (capacity == 0) stream->capacity = underlying == NULL ? length : 0x1000;
        stream->data = AllocateCabBufferSourcePilot(stream->capacity);
        stream->flags |= 0x04000000;
        stream->bufferValidBytes = 0;
        stream->bufferFromPool = 1;
    } else stream->bufferFromPool = 0;
    if (underlying != NULL) {
        underlying->vtable->addReference(underlying);
        if (stream->length == 0xffffffff) {
            ULARGE_INTEGER end;
            end.QuadPart = 0;
            underlying->vtable->seek(underlying, 0, 0, 2, &end); // Original ignores result.
            stream->length = end.LowPart - stream->baseOffset;
        }
        stream->flags |= 1;
    } else if (stream->length == 0xffffffff) stream->length = stream->capacity;
    return stream;
}

struct CabWrapperStoragePilot { void* memory; };
inline void* operator new(size_t, CabWrapperStoragePilot storage) { return storage.memory; }
inline void operator delete(void* memory, CabWrapperStoragePilot) {
    ReturnFailedCabWrapperSourcePilot(memory);
}
struct ConstructedCabWrapperPilot : CabWrappedStreamObserved48Pilot {
    ConstructedCabWrapperPilot(DWORD capacity, CabStreamSourcePilot* underlying,
        DWORD length, DWORD flags, void* data, DWORD offset) {
        InitializeCabWrappedStreamSourcePilot(this, capacity, underlying, length, flags, data, offset);
    }
};
// Original20127440, RET24, returns constructor EAX despite stored void signature.
// Source cleanup intent is explicit; fixed-profile emitted EH remains unverified.
extern "C" CabStreamSourcePilot* __stdcall WrapCabStreamSourcePilot(
    DWORD length, DWORD flags, DWORD data, DWORD capacity, CabStreamSourcePilot* underlying, DWORD offset)
{
    CabWrapperStoragePilot storage;
    storage.memory = AllocateCabWrapperRecordPilot();
    if (storage.memory == NULL) return NULL;
    return reinterpret_cast<CabStreamSourcePilot*>(new (storage) ConstructedCabWrapperPilot(
        capacity, underlying, length, flags, reinterpret_cast<void*>(data), offset));
}

// Original20126d82: receiver ESI; releases data/underlying and returns pool record.
extern "C" CabWrappedStreamObserved48Pilot* __stdcall DestroyCabWrappedStreamSourcePilot(
    CabWrappedStreamObserved48Pilot* stream)
{
    stream->vtable = &g_cabWrappedStreamVTablePilot;
    if ((stream->flags & 0x04000000) != 0) {
        if (stream->data != NULL) {
            if (stream->bufferFromPool == 0) HeapFree(g_privateHeapPilot, 0, stream->data);
            else ReturnCabBufferPilot(stream->data);
        }
        stream->data = NULL;
    }
    if (stream->underlying != NULL) stream->underlying->vtable->release(stream->underlying);
    ReturnCabWrapperRecordPilot(stream);
    return stream;
}
