#include <ctype.h>
#include <mbstring.h>
#include <new>
#include <string.h>

// 20143ebb. Skip from the delimiter to the next line, coalescing CR and LF.
static char* SkipStreamParserLinePilot(char* cursor)
{
    cursor = CharNextA(cursor);
    while (*cursor != '\0' && *cursor != '\r' && *cursor != '\n')
        cursor = CharNextA(cursor);
    while (*cursor == '\r' || *cursor == '\n') cursor = CharNextA(cursor);
    return cursor;
}

// 20143eff. Windows character stepping preserves the original DBCS traversal.
static char* TrimStreamParserTokenEndPilot(char* start, char* end)
{
    char* last = CharPrevA(start, end);
    while (*last != '\0' && isspace(static_cast<unsigned char>(*last)))
        last = CharPrevA(start, last);
    return CharNextA(last);
}

// 20143aeb. Both output pointers refer into the mutable input buffer.
extern "C" char* __stdcall SplitStreamParserLinePilot(char* cursor, char** key, char** value)
{
    *key = NULL;
    *value = NULL;
    char* token = NULL;
    bool quoted = false;
    while (*cursor != '\0') {
        if (*cursor == '"') quoted = !quoted;
        const bool newline = *cursor == '\r' || *cursor == '\n';
        const bool comment = !quoted && (*cursor == ';' ||
            (*cursor == '/' && *CharNextA(cursor) == '/'));
        if (newline || comment) {
            char* next = newline ? cursor : SkipStreamParserLinePilot(cursor);
            if (newline) {
                do { next = CharNextA(next); } while (*next == '\r' || *next == '\n');
            }
            *(token == NULL ? cursor : TrimStreamParserTokenEndPilot(token, cursor)) = '\0';
            cursor = next;
            break;
        }
        if (token == NULL) {
            if (!isspace(static_cast<unsigned char>(*cursor))) {
                token = cursor;
                if (*key == NULL) *key = cursor;
                else *value = cursor;
            }
        } else if (*cursor == '=') {
            if (*key == NULL || *value != NULL) {
                *key = NULL;
                *value = NULL;
                return SkipStreamParserLinePilot(cursor);
            }
            char* next = CharNextA(cursor);
            *TrimStreamParserTokenEndPilot(token, cursor) = '\0';
            token = NULL;
            cursor = next;
            continue;
        }
        cursor = CharNextA(cursor);
    }
    if (*value != NULL && **value == '"') {
        *value = CharNextA(*value);
        char* last = CharPrevA(*value, *value + strlen(*value));
        if (*last == '"') *last = '\0';
    }
    return cursor;
}

static void* StreamParserSortedRecordPilot(const StreamParserIndexPilot& index, int rank)
{
    DWORD offset = static_cast<DWORD*>(index.order)[rank] * index.stride;
    return static_cast<BYTE*>(index.records) + (static_cast<LONG>(offset) < 0 ? 0 : offset);
}

// 20143dcc. A miss returns the complement of the insertion rank.
extern "C" int __stdcall FindStreamParserNamePilot(const StreamParserIndexPilot* index, const char* name)
{
    int low = 0;
    int high = static_cast<int>(index->count);
    while (low < high) {
        int rank = (low + high) / 2;
        const char* existing = *static_cast<const char**>(StreamParserSortedRecordPilot(*index, rank));
        int order = _mbsicmp(reinterpret_cast<const unsigned char*>(name),
            reinterpret_cast<const unsigned char*>(existing));
        if (order == 0) return rank;
        if (order < 0) high = rank;
        else low = rank + 1;
    }
    return ~low;
}

// 20143e41. Only the sorted index shifts. Records are appended in input order.
extern "C" void* __stdcall InsertStreamParserNamePilot(
    StreamParserIndexPilot* index, DWORD rank, const char* name)
{
    if (index->capacity <= index->count) {
        index->capacity += 50;
        ReallocatePrivateHeapMemoryBoundaryPilot(1, index->capacity * sizeof(DWORD), &index->order);
        ReallocatePrivateHeapMemoryBoundaryPilot(1, index->capacity * index->stride, &index->records);
    }
    DWORD* slot = static_cast<DWORD*>(index->order) + rank;
    if (rank < index->count)
        memmove(slot + 1, slot, (index->count - rank) * sizeof(*slot));
    *slot = index->count;
    void* record = static_cast<BYTE*>(index->records) + index->count * index->stride;
    ++index->count;
    *static_cast<const char**>(record) = name;
    return record;
}

// 20143c7a. A duplicate selects the existing section and disables its following entries.
extern "C" int __stdcall BeginStreamParserSectionPilot(
    StreamParserPrefixObserved44Pilot* parser, char* header)
{
    if (header == NULL) return 0;
    size_t length = strlen(header);
    if (length < 3 || header[length - 1] != ']') return 0;
    header[length - 1] = '\0';
    const char* name = CharNextA(header);
    int rank = FindStreamParserNamePilot(&parser->sections, name);
    if (rank >= 0) {
        parser->currentSection = static_cast<StreamParserSectionObserved24Pilot*>(
            StreamParserSortedRecordPilot(parser->sections, rank));
        return 0;
    }
    parser->currentSection = static_cast<StreamParserSectionObserved24Pilot*>(
        InsertStreamParserNamePilot(&parser->sections, ~rank, name));
    StreamParserIndexPilot& keys = parser->currentSection->keys;
    keys.count = keys.capacity = 0;
    keys.stride = 8;
    keys.order = keys.records = NULL;
    return 1;
}

// 20143d21. The first occurrence of a key wins, even for a null value.
extern "C" int __stdcall AddStreamParserValuePilot(
    StreamParserPrefixObserved44Pilot* parser, const char* key, const char* value)
{
    if (parser->currentSection == NULL) return 0;
    StreamParserIndexPilot& keys = parser->currentSection->keys;
    int rank = FindStreamParserNamePilot(&keys, key);
    if (rank >= 0) return 0;
    const char** record = static_cast<const char**>(InsertStreamParserNamePilot(&keys, ~rank, key));
    record[1] = value;
    return 1;
}

// 20143891. Line count follows tokenizer iterations, which can coalesce CR/LF.
extern "C" int __stdcall LoadStreamParserPilot(StreamParserPrefixObserved44Pilot* parser)
{
    if (ReadStreamParserInputPilot(parser) == 0) return 0;
    char* cursor = parser->contents;
    char* end = cursor + parser->bytesRead;
    parser->lineCount = 0;
    int acceptValues = 1;
    while (cursor < end && *cursor != '\0') {
        char* key;
        char* value;
        cursor = SplitStreamParserLinePilot(cursor, &key, &value);
        if (key != NULL) {
            if (*key == '[') acceptValues = BeginStreamParserSectionPilot(parser, key);
            else if (acceptValues != 0) AddStreamParserValuePilot(parser, key, value);
        }
        ++parser->lineCount;
    }
    return 1;
}

extern "C" char g_configurationFilePilot[];
extern "C" char* __stdcall CopyResolvedPathPilot(char*, const char*, unsigned int);
extern "C" char* __stdcall DuplicateHeapStringPilot(const char*);
extern int g_factoryZeroInitializeBoundaryPilot;

struct StreamParserStoragePilot { void* memory; };
inline void* operator new(size_t, StreamParserStoragePilot storage) { return storage.memory; }
inline void operator delete(void* memory, StreamParserStoragePilot)
{
    if (memory != NULL) HeapFree(g_privateHeapPilot, 0, memory);
}
struct ConstructedStreamParserPilot : StreamParserPrefixObserved44Pilot {
    explicit ConstructedStreamParserPilot(const char* resolved) {
        static_cast<StreamParserPrefixObserved44Pilot&>(*this) = StreamParserPrefixObserved44Pilot();
        sections.stride = sizeof(StreamParserSectionObserved24Pilot);
        path = DuplicateHeapStringPilot(resolved);
        LoadStreamParserPilot(this);
    }
};
C_ASSERT(sizeof(ConstructedStreamParserPilot) == 0x2c);

// 20130410. Original path in ECX, selector on the stack.
extern "C" StreamParserPrefixObserved44Pilot* __stdcall AcquireStreamParserPilot(
    const char* path, int selector)
{
    const char* request = path == NULL ? "" : path;
    if (selector == g_streamParserSelectionPilot && g_lastStreamParserPilot != NULL &&
        strcmp(request, g_streamParserRequestPathPilot) == 0)
        return g_lastStreamParserPilot;

    char resolved[260];
    if (path == NULL) {
        CopyResolvedPathPilot(resolved, g_configurationFilePilot, sizeof(resolved));
    } else if (ResolvePathPilot(resolved, sizeof(resolved), path, selector) == 0) {
        ResolvePathPilot(resolved, sizeof(resolved), resolved, selector);
    }
    // First resolution failure can leave the buffer unwritten in the original.
    int slot = 0;
    for (; slot < 5; ++slot) {
        StreamParserPrefixObserved44Pilot* candidate = g_streamParserSlotsPilot[slot];
        if (candidate == NULL || _mbsicmp(reinterpret_cast<const unsigned char*>(candidate->path),
            reinterpret_cast<const unsigned char*>(resolved)) == 0) break;
    }
    bool hit = slot < 5 && g_streamParserSlotsPilot[slot] != NULL;
    if (slot == 5) slot = 4;
    StreamParserPrefixObserved44Pilot* parser = g_streamParserSlotsPilot[slot];
    if (!hit && parser != NULL) {
        ReleaseStreamParserInputPilot(parser);
        HeapFree(g_privateHeapPilot, 0, parser);
    }
    for (; slot > 0; --slot) g_streamParserSlotsPilot[slot] = g_streamParserSlotsPilot[slot - 1];
    if (!hit) {
        WritePrivateProfileStringA(NULL, NULL, NULL, resolved);
        StreamParserStoragePilot storage;
        AllocatePrivateHeapMemoryPilot(1, sizeof(ConstructedStreamParserPilot), &storage.memory,
            g_factoryZeroInitializeBoundaryPilot);
        parser = storage.memory == NULL ? NULL : new (storage) ConstructedStreamParserPilot(resolved);
    }
    g_streamParserSlotsPilot[0] = parser;
    if (parser != NULL) {
        CopyPathSourceBoundaryPilot(request, g_streamParserRequestPathPilot,
            sizeof(g_streamParserRequestPathPilot));
        g_streamParserSelectionPilot = selector;
        g_lastStreamParserPilot = parser;
    }
    return parser;
}

// 20143d6d. Failed lookup leaves the current section in place.
extern "C" int __stdcall SelectStreamParserSectionPilot(
    StreamParserPrefixObserved44Pilot* parser, const char* name)
{
    if (parser->currentSection != NULL &&
        _mbsicmp(reinterpret_cast<const unsigned char*>(name),
            reinterpret_cast<const unsigned char*>(parser->currentSection->name)) == 0) return 1;
    int rank = FindStreamParserNamePilot(&parser->sections, name);
    if (rank < 0) return 0;
    parser->currentSection = static_cast<StreamParserSectionObserved24Pilot*>(
        StreamParserSortedRecordPilot(parser->sections, rank));
    return 1;
}

// 2014369a. Missing section/key leaves output untouched. A null value clears it.
extern "C" DWORD __stdcall ReadStreamParserValuePilot(StreamParserPrefixObserved44Pilot* parser,
    const char* section, const char* key, char* output, DWORD capacity)
{
    if (SelectStreamParserSectionPilot(parser, section) == 0) return 0;
    const StreamParserIndexPilot& keys = parser->currentSection->keys;
    int rank = FindStreamParserNamePilot(&keys, key);
    if (rank < 0) return 0;
    const char* value = static_cast<const char**>(StreamParserSortedRecordPilot(keys, rank))[1];
    if (value == NULL) {
        *output = '\0';
        return 0;
    }
    DWORD length = static_cast<DWORD>(strlen(value));
    CopyResolvedPathPilot(output, value, capacity);
    return length < capacity ? length : capacity;
}

// 2013086a and 20130889, default configuration section/key existence exports.
extern "C" int __stdcall HasConfigurationSectionPilot(const char* section)
{
    return SelectStreamParserSectionPilot(AcquireStreamParserPilot(NULL, 0), section);
}

extern "C" int __stdcall HasConfigurationKeyPilot(const char* section, const char* key)
{
    StreamParserPrefixObserved44Pilot* parser = AcquireStreamParserPilot(NULL, 0);
    if (SelectStreamParserSectionPilot(parser, section) == 0) return 0;
    return FindStreamParserNamePilot(&parser->currentSection->keys, key) >= 0;
}
