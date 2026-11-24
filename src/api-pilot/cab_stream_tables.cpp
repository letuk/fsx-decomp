#include "cab_stream_records.h"
#include "heap_boundary.h"
#include <string.h>

extern "C" void __stdcall ReturnCabFileStreamRecordPilot(CabFileStreamObserved276Pilot*);
extern "C" void __stdcall ReturnCabWrapperRecordPilot(CabWrappedStreamObserved48Pilot*);
extern "C" CabWrappedStreamObserved48Pilot* __stdcall DestroyCabWrappedStreamSourcePilot(CabWrappedStreamObserved48Pilot*);

static const GUID CabSpecialStreamId = {0x64e3910f,0x513f,0x45e5,{0xb7,0x34,0x87,0x76,0xac,0x78,0xe3,0xfa}};
static const GUID CabStreamIds[] = {
    {0x0000000c,0,0,{0xc0,0,0,0,0,0,0,0x46}},
    {0x0c733a30,0x2a1c,0x11ce,{0xad,0xe5,0,0xaa,0,0x44,0x77,0x3d}},
    {0,0,0,{0xc0,0,0,0,0,0,0,0x46}}
};
static bool IsCabCommonStreamId(const void* id)
{
    for (unsigned index = 0; index < sizeof(CabStreamIds) / sizeof(CabStreamIds[0]); ++index)
        if (memcmp(id, &CabStreamIds[index], sizeof(GUID)) == 0) return true;
    return false;
}
extern "C" int __stdcall QueryCabFileStreamPilot(CabStreamSourcePilot* self, const void* id, CabStreamSourcePilot** output)
{
    if (memcmp(id, &CabSpecialStreamId, sizeof(GUID)) != 0 && !IsCabCommonStreamId(id)) return 0x80004002;
    *output = self;
    ++reinterpret_cast<CabFileStreamObserved276Pilot*>(self)->references;
    return 0;
}
extern "C" int __stdcall QueryCabWrappedStreamPilot(CabStreamSourcePilot* self, const void* id, CabStreamSourcePilot** output)
{
    CabWrappedStreamObserved48Pilot* stream = reinterpret_cast<CabWrappedStreamObserved48Pilot*>(self);
    if (memcmp(id, &CabSpecialStreamId, sizeof(GUID)) == 0) {
        if (stream->underlying != NULL) {
            CabStreamSourcePilot* queried;
            if (stream->underlying->vtable->query(stream->underlying, id, &queried) != 0) return 0x80004002;
            queried->vtable->release(queried);
        }
    } else if (!IsCabCommonStreamId(id)) return 0x80004002;
    *output = self;
    ++stream->references;
    return 0;
}
extern "C" ULONG __stdcall AddCabStreamReferencePilot(CabStreamSourcePilot* self)
{
    return InterlockedIncrement(reinterpret_cast<volatile LONG*>(self + 1));
}
extern "C" ULONG __stdcall ReleaseCabFileMethodPilot(CabStreamSourcePilot* self)
{
    CabFileStreamObserved276Pilot* stream = reinterpret_cast<CabFileStreamObserved276Pilot*>(self);
    LONG count = InterlockedDecrement(&stream->references);
    if (count > 0) return count;
    if (stream != NULL) {
        stream->vtable = &g_cabFileStreamVTablePilot;
        CloseHandle(stream->file);
        ReturnCabFileStreamRecordPilot(stream);
    }
    return 0;
}
extern "C" ULONG __stdcall ReleaseCabWrappedMethodPilot(CabStreamSourcePilot* self)
{
    CabWrappedStreamObserved48Pilot* stream = reinterpret_cast<CabWrappedStreamObserved48Pilot*>(self);
    LONG count = InterlockedDecrement(&stream->references);
    if (count > 0) return count;
    if (stream != NULL) DestroyCabWrappedStreamSourcePilot(stream);
    return 0;
}
extern "C" int __stdcall WriteCabFileMethodPilot(CabStreamSourcePilot* self, const void* buffer, DWORD bytes, DWORD* written)
{
    DWORD ignored;
    if (written == NULL) written = &ignored;
    return WriteFile(reinterpret_cast<CabFileStreamObserved276Pilot*>(self)->file, buffer, bytes, written, NULL) ? 0 : 0x80004005;
}
extern "C" int __stdcall WriteCabWrappedMethodPilot(CabStreamSourcePilot* self, const void* buffer, DWORD bytes, DWORD* written)
{
    CabWrappedStreamObserved48Pilot* stream = reinterpret_cast<CabWrappedStreamObserved48Pilot*>(self);
    if (stream->underlying == NULL && stream->data != NULL && (stream->flags & 1) == 0 &&
        stream->bufferStart <= stream->position && stream->position < stream->bufferStart + stream->capacity) {
        DWORD offset = stream->position - stream->bufferStart;
        DWORD count = stream->capacity - offset;
        if (bytes <= count) count = bytes;
        memcpy(static_cast<BYTE*>(stream->data) + offset, buffer, count);
        stream->position += count;
        DWORD end = stream->position - stream->bufferStart;
        if (end > stream->bufferValidBytes) stream->bufferValidBytes = end;
        if (written != NULL) *written = count;
        return 0;
    }
    if (written != NULL) *written = 0;
    return 0x80004005;
}
extern "C" int __stdcall ResizeCabFilePilot(CabStreamSourcePilot* self, LONG low, DWORD)
{
    HANDLE file = reinterpret_cast<CabFileStreamObserved276Pilot*>(self)->file;
    DWORD position = SetFilePointer(file, 0, NULL, FILE_CURRENT);
    SetFilePointer(file, low, NULL, FILE_BEGIN);
    SetEndOfFile(file);
    SetFilePointer(file, static_cast<LONG>(position), NULL, FILE_BEGIN);
    return 0;
}
extern "C" int __stdcall ResizeCabWrappedPilot(CabStreamSourcePilot*, DWORD, DWORD) { return 0x80004001; }
extern "C" int __stdcall CommitCabStreamPilot(CabStreamSourcePilot*, DWORD) { return 0; }
extern "C" int __stdcall RevertCabStreamPilot(CabStreamSourcePilot*) { return 0x80004001; }
extern "C" int __stdcall RegionCabStreamPilot(CabStreamSourcePilot*, DWORD, DWORD, DWORD, DWORD, DWORD) { return 0x80004001; }

// 2012451d forwards to the copy body at 20127084 with a private register ABI.
extern "C" int __stdcall CopyCabStreamPilot(CabStreamSourcePilot* source, CabStreamSourcePilot* destination,
    DWORD low, DWORD high, ULARGE_INTEGER* totalRead, ULARGE_INTEGER* totalWritten)
{
    if (destination == NULL) return 0x80030009;
    ULONGLONG remaining = (static_cast<ULONGLONG>(high) << 32) | low;
    ULONGLONG read = 0, written = 0;
    int status = 0;
    BYTE buffer[4096];
    while (remaining != 0) {
        DWORD request = remaining < sizeof(buffer) ? static_cast<DWORD>(remaining) : sizeof(buffer);
        DWORD count;
        status = source->vtable->read(source, buffer, request, &count);
        read += count;
        remaining -= count;
        if (status < 0 || count == 0) break;
        DWORD outputCount;
        status = destination->vtable->write(destination, buffer, count, &outputCount);
        written += outputCount;
        if (status < 0) break;
    }
    if (totalRead != NULL) totalRead->QuadPart = read;
    if (totalWritten != NULL) totalWritten->QuadPart = written;
    return status;
}
extern "C" int __stdcall CloneCabFileStreamPilot(CabStreamSourcePilot* self, CabStreamSourcePilot** output)
{
    if (output == NULL) return 0x80030009;
    CabFileStreamObserved276Pilot* stream = reinterpret_cast<CabFileStreamObserved276Pilot*>(self);
    DWORD access;
    switch (stream->mode) {
    case 0: access = GENERIC_READ; break;
    case 1: access = GENERIC_WRITE; break;
    case 2: access = GENERIC_READ | GENERIC_WRITE; break;
    default: return 0x800300fd;
    }
    *output = OpenCabFileStreamSourcePilot(stream->path, access, stream->mode + 1, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return *output != NULL ? 0 : 0x80004005;
}
extern "C" int __stdcall CloneCabWrappedStreamPilot(CabStreamSourcePilot* self, CabStreamSourcePilot** output)
{
    if (output == NULL) return 0x80030009;
    CabWrappedStreamObserved48Pilot* stream = reinterpret_cast<CabWrappedStreamObserved48Pilot*>(self);
    if (stream->underlying == NULL) {
        if (stream->length != stream->bufferValidBytes) return 0x80004001;
        *output = WrapCabStreamSourcePilot(stream->length, 0, reinterpret_cast<DWORD>(stream->data),
            stream->bufferValidBytes, self, 0);
    } else {
        CabStreamSourcePilot* cloned = NULL;
        int status = stream->underlying->vtable->clone(stream->underlying, &cloned);
        if (status < 0) return status;
        if (cloned == NULL) return 0x80004005;
        *output = WrapCabStreamSourcePilot(0xffffffff, 0, 0, 0, cloned, stream->baseOffset);
        cloned->vtable->release(cloned);
    }
    return *output != NULL ? 0 : 0x8007000e;
}

const CabStreamVTableObservedPilot g_cabFileStreamVTablePilot = {
    QueryCabFileStreamPilot, AddCabStreamReferencePilot, ReleaseCabFileMethodPilot,
    reinterpret_cast<int (__stdcall *)(CabStreamSourcePilot*, void*, DWORD, DWORD*)>(ReadCabFileMethodPilot),
    WriteCabFileMethodPilot,
    reinterpret_cast<int (__stdcall *)(CabStreamSourcePilot*, DWORD, DWORD, DWORD, void*)>(SeekCabFileMethodPilot),
    reinterpret_cast<void*>(ResizeCabFilePilot), CopyCabStreamPilot,
    {reinterpret_cast<void*>(CommitCabStreamPilot), reinterpret_cast<void*>(RevertCabStreamPilot),
     reinterpret_cast<void*>(RegionCabStreamPilot), reinterpret_cast<void*>(RegionCabStreamPilot)},
    reinterpret_cast<int (__stdcall *)(CabStreamSourcePilot*, CabStreamStatObserved72Pilot*, DWORD)>(StatCabFileMethodPilot),
    CloneCabFileStreamPilot
};
const CabStreamVTableObservedPilot g_cabWrappedStreamVTablePilot = {
    QueryCabWrappedStreamPilot, AddCabStreamReferencePilot, ReleaseCabWrappedMethodPilot,
    reinterpret_cast<int (__stdcall *)(CabStreamSourcePilot*, void*, DWORD, DWORD*)>(ReadCabWrappedMethodPilot),
    WriteCabWrappedMethodPilot,
    reinterpret_cast<int (__stdcall *)(CabStreamSourcePilot*, DWORD, DWORD, DWORD, void*)>(SeekCabWrappedMethodPilot),
    reinterpret_cast<void*>(ResizeCabWrappedPilot), CopyCabStreamPilot,
    {reinterpret_cast<void*>(CommitCabStreamPilot), reinterpret_cast<void*>(RevertCabStreamPilot),
     reinterpret_cast<void*>(RegionCabStreamPilot), reinterpret_cast<void*>(RegionCabStreamPilot)},
    reinterpret_cast<int (__stdcall *)(CabStreamSourcePilot*, CabStreamStatObserved72Pilot*, DWORD)>(StatCabWrappedMethodPilot),
    CloneCabWrappedStreamPilot
};
