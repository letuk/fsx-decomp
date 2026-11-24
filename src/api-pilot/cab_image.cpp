#include "alternate_enumeration.h"
#include "cab_stream.h"
#include "heap_boundary.h"
extern "C" int __stdcall ResolvePathPilot(char*, unsigned int, const char*, int);

#pragma pack(push, 1)
struct CabReserveObserved4Pilot { WORD bytesToSkip; BYTE extra0, extra1; };
#pragma pack(pop)

// Original 2014420b: stdcall path, RET4. Caller owns returned private-heap image.
extern "C" void* __stdcall LoadCabImagePilot(const char* path)
{
    char resolved[260];
    CabStreamSourcePilot* stream = NULL;
    void* image = NULL;
    if (ResolvePathPilot(resolved, 260, path, 0x34) == 0) {
        stream = OpenCabFileStreamSourcePilot(resolved, GENERIC_READ, 1, NULL, 3, 0x80, NULL);
        if (stream == NULL) return NULL;
        CabStreamSourcePilot* wrapped = WrapCabStreamSourcePilot(0xffffffff, 0, 0, 0x1000, stream, 0);
        if (wrapped != NULL) {
            ReleaseCabStreamPilot(stream);
            stream = wrapped;
        }
        CabHeaderObserved36Pilot header;
        bool valid = stream->vtable->read(stream, &header, 0x24, NULL) == 0 &&
            header.signature[0] == 'M' && header.signature[1] == 'S' &&
            header.signature[2] == 'C' && header.signature[3] == 'F';
        if (valid && header.flags != 0) {
            if (header.flags != 4) valid = false;
            else {
                CabReserveObserved4Pilot reserve;
                valid = ReadCabStreamPilot(stream, &reserve, 4, NULL) == 0 &&
                    reserve.extra1 == 0 && reserve.extra0 == 0 &&
                    SeekCabStreamPilot(stream, reserve.bytesToSkip, 1) == 0;
            }
        }
        DWORD folder[2];
        if (valid && header.entryCount != 0 && ReadCabStreamPilot(stream, folder, 8, NULL) == 0) {
            if (g_heapInitializedPilot == 0) InitializeHeapManagerPilot();
            image = HeapAlloc(g_privateHeapPilot, 0, folder[0]);
            if (image == NULL) {
                image = HeapAlloc(g_privateHeapPilot, 0, folder[0]);
                if (image == NULL) {
                    HINSTANCE resources = g_errorResourceInstancePilot;
                    if (resources == NULL) resources = GetModuleHandleA(NULL);
                    LoadStringA(resources, 8, g_heapErrorTextPilot, 260);
                    MessageBoxA(NULL, g_heapErrorTextPilot, NULL, MB_ICONERROR);
                    TerminateProcess(GetCurrentProcess(), 1);
                }
            }
            SeekCabStreamPilot(stream, 0, 0); // Status ignored in original.
            if (ReadCabStreamPilot(stream, image, folder[0], NULL) != 0)
                FreePrivateHeapSlotPilot(&image);
            if (image != NULL) {
                CabHeaderObserved36Pilot* loaded = static_cast<CabHeaderObserved36Pilot*>(image);
                BYTE* entry = static_cast<BYTE*>(image) + loaded->entryOffset;
                for (int index = 0; index < loaded->entryCount; ++index) {
                    char* name = reinterpret_cast<char*>(entry + 0x10);
                    CharUpperA(name);
                    WORD length = static_cast<WORD>(lstrlenA(name));
                    reinterpret_cast<AlternateEnumerationEntryPilot*>(entry)->nameLength = length;
                    entry += 0x11 + length;
                }
            }
        }
    }
    if (stream != NULL) stream->vtable->release(stream);
    return image;
}
