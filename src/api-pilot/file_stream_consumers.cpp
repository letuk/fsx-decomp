#include "alternate_enumeration.h"
#include "cab_stream.h"
#include "heap_boundary.h"

extern "C" int __stdcall ResolvePathPilot(char*, unsigned int, const char*, int);
extern "C" int __stdcall ReadFileStreamSizeLowPilot(CabStreamSourcePilot*, DWORD*);
extern "C" int __stdcall OpenSelectedFileStreamPilot(CabStreamSourcePilot**, const char*, int, int);
extern "C" CabStreamSourcePilot* __stdcall ExtractCabEntryToStreamPilot(
    AlternateEnumerationOwnerPilot*, const char*, CabStreamSourcePilot*);

// 20132cbf, export 73. Failed resolution leaves the output alone.
extern "C" int __stdcall CreateSelectedFileStreamPilot(
    CabStreamSourcePilot** output, const char* path, int selector)
{
    char resolved[260];
    if (ResolvePathPilot(resolved, sizeof(resolved), path, selector) != 0) return -1;
    *output = OpenCabFileStreamSourcePilot(resolved, GENERIC_READ | GENERIC_WRITE,
        0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    return *output == NULL ? -1 : 0;
}

// 2014501e. The original destination is in ESI.
extern "C" int __stdcall ExtractResolvedCabFilePilot(const char* path, CabStreamSourcePilot* destination)
{
    char relativePath[260];
    AlternateEnumerationOwnerPilot* owner = ResolveAlternateEnumerationOwnerPilot(path, relativePath);
    if (owner == NULL) return 0;
    return ExtractCabEntryToStreamPilot(owner, relativePath, destination) == destination;
}

// 201349f0, export 72. Creates the destination before resolving the source.
extern "C" int __stdcall CopySelectedFilePilot(const char* destinationPath, int destinationSelector,
    const char* sourcePath, int sourceSelector)
{
    CabStreamSourcePilot* destination = NULL;
    CabStreamSourcePilot* source = NULL;
    char resolved[260];
    int status = -1;
    if (CreateSelectedFileStreamPilot(&destination, destinationPath, destinationSelector) == 0 &&
        ResolvePathPilot(resolved, sizeof(resolved), sourcePath, sourceSelector) == 0) {
        if (GetFileAttributesA(resolved) == INVALID_FILE_ATTRIBUTES) {
            if (ExtractResolvedCabFilePilot(resolved, destination) != 0) status = 0;
        } else if (OpenSelectedFileStreamPilot(&source, sourcePath, sourceSelector, 2) == 0) {
            DWORD sizeLow = 0;
            ReadFileStreamSizeLowPilot(source, &sizeLow);
            status = source->vtable->copyTo(source, destination, sizeLow, 0, NULL, NULL);
        }
    }
    if (destination != NULL) destination->vtable->release(destination);
    if (source != NULL) source->vtable->release(source);
    return status;
}

struct StreamParserIndexPilot {
    DWORD count, capacity, stride;
    void* order;
    void* records;
};
struct StreamParserSectionObserved24Pilot {
    const char* name;
    StreamParserIndexPilot keys;
};
struct StreamParserPrefixObserved44Pilot {
    char* path;
    char* contents;
    DWORD unknown08;
    DWORD bytesRead;
    DWORD lineCount;
    StreamParserSectionObserved24Pilot* currentSection;
    StreamParserIndexPilot sections;
};
C_ASSERT(sizeof(StreamParserSectionObserved24Pilot) == 0x18);
C_ASSERT(sizeof(StreamParserPrefixObserved44Pilot) == 0x2c);
C_ASSERT(offsetof(StreamParserPrefixObserved44Pilot, sections) == 0x18);
C_ASSERT(offsetof(StreamParserIndexPilot, records) == 0x10);

// 2014391b. The original parser pointer is in ESI.
extern "C" int __stdcall ReadStreamParserInputPilot(StreamParserPrefixObserved44Pilot* parser)
{
    char path[260];
    if (ResolvePathPilot(path, sizeof(path), parser->path, 0) != 0) return 0;
    CabStreamSourcePilot* stream = OpenCabFileStreamSourcePilot(path, GENERIC_READ,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (stream == NULL) {
        stream = OpenCabEntryStreamPilot(path);
        if (stream == NULL) return 0;
    } else {
        CabStreamSourcePilot* wrapped = WrapCabStreamSourcePilot(0xffffffff, 0, 0, 0x1000, stream, 0);
        if (wrapped != NULL) {
            ReleaseCabStreamPilot(stream);
            stream = wrapped;
        }
    }
    DWORD size;
    ReadFileStreamSizeLowPilot(stream, &size);
    if (size == 0) {
        stream->vtable->release(stream);
        return 0;
    }
    void* contents;
    AllocatePrivateHeapMemoryPilot(1, size + 1, &contents, 0);
    parser->contents = static_cast<char*>(contents);
    int status = stream->vtable->read(stream, parser->contents, size, &parser->bytesRead);
    stream->vtable->release(stream);
    if (status != 0) return 0; // Contents stay on the parser for teardown.
    parser->contents[parser->bytesRead] = '\0';
    return 1;
}

template<class T>
static void FreeStreamParserAllocation(T*& allocation)
{
    if (allocation != NULL) {
        HeapFree(g_privateHeapPilot, 0, allocation);
        allocation = NULL;
    }
}

// 201435c1. The original parser pointer is in EDI.
extern "C" void __stdcall ReleaseStreamParserInputPilot(StreamParserPrefixObserved44Pilot* parser)
{
    StreamParserSectionObserved24Pilot* sections =
        static_cast<StreamParserSectionObserved24Pilot*>(parser->sections.records);
    for (DWORD i = 0; i < parser->sections.count; ++i) {
        FreeStreamParserAllocation(sections[i].keys.order);
        FreeStreamParserAllocation(sections[i].keys.records);
    }
    FreeStreamParserAllocation(parser->sections.order);
    FreeStreamParserAllocation(parser->sections.records);
    FreeStreamParserAllocation(parser->path);
    FreeStreamParserAllocation(parser->contents);
}

static StreamParserPrefixObserved44Pilot* g_streamParserSlotsPilot[5]; // 2020deec
static StreamParserPrefixObserved44Pilot* g_lastStreamParserPilot; // 2020e004
static int g_streamParserSelectionPilot = -1; // 201f755c
static char g_streamParserRequestPathPilot[260]; // 2020df00

// 201306ee.
extern "C" void __cdecl ClearStreamParserSlotsPilot()
{
    for (int i = 0; i < 5; ++i) {
        StreamParserPrefixObserved44Pilot* parser = g_streamParserSlotsPilot[i];
        if (parser != NULL) {
            ReleaseStreamParserInputPilot(parser);
            HeapFree(g_privateHeapPilot, 0, parser);
            g_streamParserSlotsPilot[i] = NULL;
        }
    }
    g_lastStreamParserPilot = NULL;
    g_streamParserSelectionPilot = -1;
    g_streamParserRequestPathPilot[0] = '\0';
}
