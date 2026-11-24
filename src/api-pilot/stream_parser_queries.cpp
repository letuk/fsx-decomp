#include <stdlib.h>
#include <string.h>

// 201316e6 / 62 and 20131708 / 181.
extern "C" int __stdcall HasSelectedConfigurationSectionPilot(
    const char* section, const char* path, int selector)
{
    return SelectStreamParserSectionPilot(AcquireStreamParserPilot(path, selector), section);
}

extern "C" int __stdcall HasSelectedConfigurationKeyPilot(
    const char* section, const char* key, const char* path, int selector)
{
    StreamParserPrefixObserved44Pilot* parser = AcquireStreamParserPilot(path, selector);
    if (SelectStreamParserSectionPilot(parser, section) == 0) return 0;
    return FindStreamParserNamePilot(&parser->currentSection->keys, key) >= 0;
}

// 2013174d / 49. A valid request with no value returns zero without writing.
extern "C" int __stdcall ReadSelectedConfigurationIntPilot(const char* section, const char* key,
    int* output, const char* path, int selector)
{
    if (section == NULL || key == NULL || output == NULL) return -1;
    char value[1024];
    StreamParserPrefixObserved44Pilot* parser = AcquireStreamParserPilot(path, selector);
    if (ReadStreamParserValuePilot(parser, section, key, value, sizeof(value)) != 0)
        *output = atoi(value);
    return 0;
}

// 201318cd / 53.
extern "C" int __stdcall ReadSelectedConfigurationDoublePilot(const char* section, const char* key,
    double* output, const char* path, int selector)
{
    if (section == NULL || key == NULL || output == NULL) return -1;
    char value[1024];
    StreamParserPrefixObserved44Pilot* parser = AcquireStreamParserPilot(path, selector);
    if (ReadStreamParserValuePilot(parser, section, key, value, sizeof(value)) != 0)
        *output = strtod(value, NULL);
    return 0;
}

// 201317dd / 51 and 2013195f / 55. A nonempty value drives exactly count conversions.
extern "C" int __stdcall ReadSelectedConfigurationUIntListPilot(const char* section, const char* key,
    unsigned long* output, DWORD count, const char* path, int selector)
{
    if (section == NULL || key == NULL || output == NULL || count == 0) return -1;
    char value[1024];
    StreamParserPrefixObserved44Pilot* parser = AcquireStreamParserPilot(path, selector);
    if (ReadStreamParserValuePilot(parser, section, key, value, sizeof(value)) != 0) {
        char* cursor = value;
        for (DWORD i = 0; i < count; ++i) {
            output[i] = strtoul(cursor, &cursor, 0);
            cursor += strspn(cursor, ", ");
        }
    }
    return 0;
}

extern "C" int __stdcall ReadSelectedConfigurationDoubleListPilot(const char* section, const char* key,
    double* output, DWORD count, const char* path, int selector)
{
    if (section == NULL || key == NULL || output == NULL || count == 0) return -1;
    char value[1024];
    StreamParserPrefixObserved44Pilot* parser = AcquireStreamParserPilot(path, selector);
    if (ReadStreamParserValuePilot(parser, section, key, value, sizeof(value)) != 0) {
        char* cursor = value;
        for (DWORD i = 0; i < count; ++i) {
            output[i] = strtod(cursor, &cursor);
            cursor += strspn(cursor, ", ");
        }
    }
    return 0;
}

// 20143744. Walk the physical section array in input order. Reserve one byte
// for the final separator when truncating, and return the count including it.
extern "C" DWORD __stdcall ListStreamParserSectionsPilot(
    StreamParserPrefixObserved44Pilot* parser, char* output, DWORD capacity)
{
    const StreamParserSectionObserved24Pilot* sections =
        static_cast<const StreamParserSectionObserved24Pilot*>(parser->sections.records);
    DWORD used = 0;
    for (DWORD i = 0; i < parser->sections.count; ++i) {
        const char* name = sections[i].name;
        DWORD next = used + (name == NULL ? 0 : static_cast<DWORD>(strlen(name))) + 1;
        if (capacity <= next) {
            DWORD available = capacity - used - 1;
            CopyResolvedPathPilot(output + used, name, available);
            used += available;
            break;
        }
        CopyResolvedPathPilot(output + used, name, capacity);
        used = next;
    }
    if (used != 0) output[used++] = '\0';
    return used;
}

// 20131ab8 / 188. The original forwards the enumeration byte count in EAX.
extern "C" DWORD __stdcall ListSelectedConfigurationSectionsPilot(
    char* output, DWORD capacity, const char* path, int selector)
{
    return ListStreamParserSectionsPilot(AcquireStreamParserPilot(path, selector), output, capacity);
}

// 201437eb and 20131adb / 225. Stop at the first absent zero-based suffix.
extern "C" int __stdcall CountSelectedConfigurationSectionsPilot(
    const char* prefix, const char* path, int selector)
{
    if (prefix == NULL) return 0;
    StreamParserPrefixObserved44Pilot* parser = AcquireStreamParserPilot(path, selector);
    for (int count = 0;; ++count) {
        char name[256];
        FormatNumberedNamePilot(prefix, count, name, sizeof(name));
        if (SelectStreamParserSectionPilot(parser, name) == 0) return count;
    }
}

// 20131b08. Re-scan a successful replacement from its first byte.
extern "C" void __stdcall ExpandConfigurationStringPilot(char* output, DWORD capacity, const char* path)
{
    char* cursor = output;
    while (char* start = strchr(cursor, '%')) {
        char* key = start + 1;
        char* end = strchr(key, '%');
        if (end == NULL) break;
        *end = '\0';
        char replacement[2048];
        DWORD length = GetPrivateProfileStringA("Strings", key, "", replacement, sizeof(replacement), path);
        if (length == 0) {
            // Keep the original 512-byte copy limit. The extra byte holds
            // the terminator after adding a closing quote to a full key.
            char quotedKey[513] = "\"";
            CopyPathSourceBoundaryPilot(key, quotedKey + 1, 511);
            int endQuote = lstrlenA(quotedKey);
            quotedKey[endQuote] = '"';
            quotedKey[endQuote + 1] = '\0';
            length = GetPrivateProfileStringA("Strings", quotedKey, "", replacement, sizeof(replacement), path);
        }
        if (length == 0) {
            *end = '%';
            cursor = end + 1;
        } else {
            lstrcpynA(replacement + length, end + 1, sizeof(replacement) - length);
            lstrcpynA(start, replacement, capacity - (start - output));
            cursor = start;
        }
    }
}

// 20131cea. The result counts emitted bytes after collapsing quote pairs.
extern "C" DWORD __stdcall CollapseConfigurationQuotesPilot(char* output, DWORD capacity)
{
    char* source = output;
    DWORD count = 0;
    while (*source != '\0' && count < capacity) {
        if (source[0] == '"' && source[1] == '"') ++source;
        output[count++] = *source++;
    }
    output[count] = '\0';
    return count;
}

// 20131a4d / 57. Selector 0x2f bypasses both postprocessing steps.
extern "C" DWORD __stdcall ReadSelectedConfigurationStringPilot(const char* section, const char* key,
    char* output, DWORD capacity, const char* path, int selector)
{
    if (section == NULL || key == NULL || output == NULL || capacity == 0) return 0;
    StreamParserPrefixObserved44Pilot* parser = AcquireStreamParserPilot(path, selector);
    DWORD length = ReadStreamParserValuePilot(parser, section, key, output, capacity);
    if (length == 0 || selector == 0x2f) return length;
    ExpandConfigurationStringPilot(output, capacity, parser->path);
    return CollapseConfigurationQuotesPilot(output, capacity);
}

// Default-file exports 39, 43, 47, 200 and 202.
extern "C" int __stdcall ReadConfigurationIntPilot(const char* section, const char* key, int* output)
{
    return ReadSelectedConfigurationIntPilot(section, key, output, NULL, 0);
}

extern "C" int __stdcall ReadConfigurationDoublePilot(const char* section, const char* key, double* output)
{
    return ReadSelectedConfigurationDoublePilot(section, key, output, NULL, 0);
}

extern "C" DWORD __stdcall ReadConfigurationStringPilot(const char* section, const char* key,
    char* output, DWORD capacity)
{
    return ReadSelectedConfigurationStringPilot(section, key, output, capacity, NULL, 0);
}

extern "C" int __stdcall ReadConfigurationWordPilot(const char* section, const char* key, WORD* output)
{
    int value = *output;
    int status = ReadSelectedConfigurationIntPilot(section, key, &value, NULL, 0);
    if (status == 0) *output = static_cast<WORD>(value);
    return status;
}

extern "C" int __stdcall ReadConfigurationBytePilot(const char* section, const char* key, BYTE* output)
{
    int value = *output;
    int status = ReadSelectedConfigurationIntPilot(section, key, &value, NULL, 0);
    if (status == 0) *output = static_cast<BYTE>(value);
    return status;
}
