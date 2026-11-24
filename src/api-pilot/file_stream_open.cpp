#include "cab_stream.h"
#include "heap_boundary.h"

extern "C" int __stdcall ResolvePathPilot(char*, unsigned int, const char*, int);
extern "C" int __stdcall FindCachedFileStreamSourcePilot(CabStreamSourcePilot**, const char*);
extern "C" int __stdcall RememberFileStreamSourcePilot(CabStreamSourcePilot*, const char*);
// util.dll ordinal 195, original 20f4d270.
extern "C" DWORD __stdcall TransformFilePayloadPilot(const void*, DWORD, void*, DWORD);

// 20133611. Seek may leave either position unwritten. Keep those outputs
// unspecified and return the restore status even though the size is copied.
extern "C" int __stdcall ReadFileStreamSizeLowPilot(CabStreamSourcePilot* stream, DWORD* sizeLow)
{
    if (stream == NULL || sizeLow == NULL) return -1;
    ULARGE_INTEGER saved, end;
    stream->vtable->seek(stream, 0, 0, STREAM_SEEK_CUR, &saved);
    stream->vtable->seek(stream, 0, 0, STREAM_SEEK_END, &end);
    int status = stream->vtable->seek(stream, saved.LowPart, saved.HighPart, STREAM_SEEK_SET, NULL);
    *sizeLow = end.LowPart;
    return status;
}

static DWORD ReadFileProbeWordPilot(const BYTE* bytes)
{
    DWORD word;
    memcpy(&word, bytes, sizeof(word));
    return word;
}

// 20132e20. The original uses DWORD arithmetic and does not check the
// payload bounds separately from the probe and size checks.
extern "C" CabStreamSourcePilot* __stdcall DecodeFileStreamSourcePilot(CabStreamSourcePilot* stream)
{
    if (stream == NULL) return NULL;
    BYTE probe[0xbc];
    DWORD bytesRead = 0;
    void* encoded = NULL;
    void* decoded = NULL;
    DWORD streamSize;
    if (stream->vtable->read(stream, probe, sizeof(probe), &bytesRead) == 0 &&
        bytesRead >= sizeof(probe) &&
        ReadFileProbeWordPilot(probe) != 0x19920201 &&
        (ReadFileProbeWordPilot(probe + 0x7a) & 0x80000000) != 0) {
        DWORD prefixSize = ReadFileProbeWordPilot(probe + 0x76) == 0x87654321 ? 0xa8 : 0x80;
        if (ReadFileProbeWordPilot(probe + prefixSize) == 0xabcdef01 &&
            ReadFileStreamSizeLowPilot(stream, &streamSize) == 0 && streamSize != 0xffffffff) {
            AllocatePrivateHeapMemoryPilot(1, streamSize, &encoded, 0);
            SeekCabStreamPilot(stream, 0, STREAM_SEEK_SET);
            if (ReadCabStreamPilot(stream, encoded, streamSize, NULL) == 0) {
                DWORD payloadOffset = prefixSize + 0x14;
                DWORD decodedSize = ReadFileProbeWordPilot(probe + prefixSize + 4);
                AllocatePrivateHeapMemoryPilot(1, prefixSize + decodedSize, &decoded, 0);
                memcpy(decoded, encoded, prefixSize);
                DWORD produced = TransformFilePayloadPilot(
                    static_cast<BYTE*>(encoded) + payloadOffset, streamSize - payloadOffset,
                    static_cast<BYTE*>(decoded) + prefixSize, decodedSize);
                if (produced == decodedSize) {
                    FreePrivateHeapSlotPilot(&encoded);
                    return WrapCabStreamSourcePilot(0xffffffff, 0x04000000,
                        reinterpret_cast<DWORD>(decoded), prefixSize + produced, NULL, 0);
                }
            }
        }
    }
    SeekCabStreamPilot(stream, 0, STREAM_SEEK_SET);
    if (decoded != NULL) HeapFree(g_privateHeapPilot, 0, decoded);
    if (encoded != NULL) HeapFree(g_privateHeapPilot, 0, encoded);
    return NULL;
}

// 20133137, export ordinal 85. A path resolution failure leaves the output untouched.
extern "C" int __stdcall OpenSelectedFileStreamPilot(CabStreamSourcePilot** output,
    const char* path, int selector, int mode)
{
    char resolved[260];
    if (ResolvePathPilot(resolved, sizeof(resolved), path, selector) != 0) return -1;

    const bool allowCabFallback = mode == 0 || mode == 1 || mode == 2 || mode == 6;
    if (selector == 1 && allowCabFallback && FindCachedFileStreamSourcePilot(output, resolved) != 0)
        return 0;

    bool bufferFile = false;
    switch (mode) {
    case 0:
    case 6:
        *output = OpenCabFileStreamSourcePilot(resolved, GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        bufferFile = mode != 6;
        break;
    case 1:
    case 2: {
        DWORD hint = mode == 1 ? FILE_FLAG_RANDOM_ACCESS : FILE_FLAG_SEQUENTIAL_SCAN;
        *output = OpenCabFileStreamSourcePilot(resolved, GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | hint, NULL);
        bufferFile = true;
        break;
    }
    case 3:
        *output = OpenCabFileStreamSourcePilot(resolved, GENERIC_READ | GENERIC_WRITE, 0,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        break;
    case 4:
        *output = OpenCabFileStreamSourcePilot(resolved, GENERIC_WRITE, 0,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        break;
    case 5:
        *output = OpenCabFileStreamSourcePilot(resolved, GENERIC_WRITE, 0,
            NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        SeekCabStreamPilot(*output, 0, FILE_END);
        break;
    default:
        *output = NULL;
        break;
    }

    bool remember = false;
    if (*output == NULL && allowCabFallback && selector != 0x34) {
        bufferFile = false;
        remember = selector == 1;
        *output = OpenCabEntryStreamPilot(resolved);
    }
    if (*output == NULL) return -1;

    if (selector == 1 && mode != 6) {
        CabStreamSourcePilot* decoded = DecodeFileStreamSourcePilot(*output);
        if (decoded != NULL) {
            ReleaseCabStreamPilot(*output);
            *output = decoded;
            RememberFileStreamSourcePilot(decoded, resolved);
            return 0;
        }
    }
    if (bufferFile) {
        CabStreamSourcePilot* wrapped = WrapCabStreamSourcePilot(0xffffffff, 0, 0, 0x1000, *output, 0);
        if (wrapped != NULL) {
            ReleaseCabStreamPilot(*output);
            *output = wrapped;
        }
    }
    if (remember) RememberFileStreamSourcePilot(*output, resolved);
    return 0;
}
