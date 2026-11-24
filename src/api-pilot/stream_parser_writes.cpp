#include <string.h>

// 20130748. Re-resolve at selector zero and stop at the first empty slot.
extern "C" void __stdcall InvalidateStreamParserPilot(const char* path)
{
    char resolved[260];
    if (path == NULL) {
        CopyResolvedPathPilot(resolved, g_configurationFilePilot, sizeof(resolved));
    } else if (ResolvePathPilot(resolved, sizeof(resolved), path, 0) == 0) {
        ResolvePathPilot(resolved, sizeof(resolved), resolved, 0);
    }
    // An unsuccessful first resolution can leave the original key unwritten.
    for (int slot = 0; slot < 5; ++slot) {
        StreamParserPrefixObserved44Pilot* parser = g_streamParserSlotsPilot[slot];
        if (parser == NULL) return;
        if (_mbsicmp(reinterpret_cast<const unsigned char*>(parser->path),
            reinterpret_cast<const unsigned char*>(resolved)) == 0) {
            ReleaseStreamParserInputPilot(parser);
            HeapFree(g_privateHeapPilot, 0, parser);
            if (g_lastStreamParserPilot == parser) {
                g_lastStreamParserPilot = NULL;
                g_streamParserSelectionPilot = -1;
                g_streamParserRequestPathPilot[0] = '\0';
            }
            for (; slot < 4; ++slot) g_streamParserSlotsPilot[slot] = g_streamParserSlotsPilot[slot + 1];
            g_streamParserSlotsPilot[4] = NULL;
            return;
        }
    }
}

// 20131545. The retry preserves the original attribute word, including the
// bit-test on INVALID_FILE_ATTRIBUTES, and ignores flush/attribute results.
extern "C" int __stdcall WriteConfigurationProfilePilot(const char* section, const char* key,
    const char* value, const char* path, int selector)
{
    char resolved[260];
    if (path == NULL) {
        CopyResolvedPathPilot(resolved, g_configurationFilePilot, sizeof(resolved));
    } else {
        if (ResolvePathPilot(resolved, sizeof(resolved), path, selector) != 0) return -1;
        ResolvePathPilot(resolved, sizeof(resolved), resolved, selector);
    }
    InvalidateStreamParserPilot(resolved);
    BOOL result = WritePrivateProfileStringA(section, key, value, resolved);
    if (!result) {
        DWORD attributes = GetFileAttributesA(resolved);
        if ((attributes & FILE_ATTRIBUTE_READONLY) != 0) {
            WritePrivateProfileStringA(NULL, NULL, NULL, resolved);
            SetFileAttributesA(resolved, attributes & ~FILE_ATTRIBUTE_READONLY);
            WritePrivateProfileStringA(NULL, NULL, NULL, resolved);
            result = WritePrivateProfileStringA(section, key, value, resolved);
            WritePrivateProfileStringA(NULL, NULL, NULL, resolved);
            SetFileAttributesA(resolved, attributes | FILE_ATTRIBUTE_READONLY);
            WritePrivateProfileStringA(NULL, NULL, NULL, resolved);
        }
    }
    return result ? 0 : -1;
}

// 2013222d / 185. Flushes without attempting the write retry.
extern "C" int __stdcall FlushSelectedConfigurationPilot(const char* path, int selector)
{
    char resolved[260];
    if (path == NULL) {
        CopyResolvedPathPilot(resolved, g_configurationFilePilot, sizeof(resolved));
    } else {
        int status = ResolvePathPilot(resolved, sizeof(resolved), path, selector);
        if (status == -1) return -1;
        if (status == 0) ResolvePathPilot(resolved, sizeof(resolved), resolved, selector);
    }
    InvalidateStreamParserPilot(resolved);
    return WritePrivateProfileStringA(NULL, NULL, NULL, resolved) ? 0 : -1;
}

// 20131d24 / 60 and 201321fb / 58.
extern "C" int __stdcall DeleteSelectedConfigurationSectionPilot(
    const char* section, const char* path, int selector)
{
    if (selector == 0x27) return -1;
    return WriteConfigurationProfilePilot(section, NULL, NULL, path, selector);
}

extern "C" int __stdcall WriteSelectedConfigurationStringPilot(const char* section,
    const char* key, const char* value, const char* path, int selector)
{
    if (selector == 0x27) return -1;
    return WriteConfigurationProfilePilot(section, key, value, path, selector);
}

// 20131d4d / 50 and 20131fa8 / 54 format before checking the selector.
extern "C" int __stdcall WriteSelectedConfigurationIntPilot(const char* section,
    const char* key, int value, const char* path, int selector)
{
    char text[1024];
    FormatBoundedTextPilot(text, sizeof(text), "%d", value);
    return WriteSelectedConfigurationStringPilot(section, key, text, path, selector);
}

extern "C" int __stdcall WriteSelectedConfigurationDoublePilot(const char* section,
    const char* key, double value, const char* path, int selector)
{
    char text[1024];
    FormatBoundedTextPilot(text, sizeof(text), "%f", value);
    return WriteSelectedConfigurationStringPilot(section, key, text, path, selector);
}

// 201203f4 with the bounded scan used by the numeric-list writers.
static void AppendConfigurationListTextPilot(char* output, const char* text, DWORD capacity)
{
    char* end = static_cast<char*>(memchr(output, '\0', capacity));
    if (end != NULL)
        CopyPathSourceBoundaryPilot(text, end, capacity - static_cast<DWORD>(end - output));
}

template<class Number>
static int WriteSelectedConfigurationListPilot(const char* section, const char* key,
    const Number* values, DWORD count, const char* path, int selector, const char* format)
{
    void* memory;
    AllocatePrivateHeapMemoryPilot(1, 1024, &memory, 0);
    char* text = static_cast<char*>(memory);
    *text = '\0';
    for (DWORD i = 0; i < count; ++i) {
        if (i != 0) AppendConfigurationListTextPilot(text, ",", 1024 - strlen(text));
        char element[1024];
        FormatBoundedTextPilot(element, sizeof(element), format, values[i]);
        AppendConfigurationListTextPilot(text, element, 1024 - strlen(text));
    }
    int status = WriteSelectedConfigurationStringPilot(section, key, text, path, selector);
    HeapFree(g_privateHeapPilot, 0, text);
    return status;
}

// 20131dd9 / 52 and 20132039 / 56.
extern "C" int __stdcall WriteSelectedConfigurationIntListPilot(const char* section,
    const char* key, const int* values, DWORD count, const char* path, int selector)
{
    return WriteSelectedConfigurationListPilot(section, key, values, count, path, selector, "%d");
}

extern "C" int __stdcall WriteSelectedConfigurationDoubleListPilot(const char* section,
    const char* key, const double* values, DWORD count, const char* path, int selector)
{
    return WriteSelectedConfigurationListPilot(section, key, values, count, path, selector, "%f");
}

// Default configuration writers. The original bodies repeat the profile
// write and retry sequence with the default path.
extern "C" int __stdcall DeleteConfigurationSectionPilot(const char* section) // 20130a27 / 59
{
    return DeleteSelectedConfigurationSectionPilot(section, NULL, 0);
}

extern "C" int __stdcall WriteConfigurationStringPilot(
    const char* section, const char* key, const char* value) // 201313e6 / 48
{
    return WriteSelectedConfigurationStringPilot(section, key, value, NULL, 0);
}

extern "C" int __stdcall WriteConfigurationIntPilot(
    const char* section, const char* key, int value) // 20130b6b / 40
{
    return WriteSelectedConfigurationIntPilot(section, key, value, NULL, 0);
}

extern "C" int __stdcall WriteConfigurationWordPilot(
    const char* section, const char* key, WORD value) // 20130cdf / 201
{
    return WriteConfigurationIntPilot(section, key, value);
}

extern "C" int __stdcall WriteConfigurationBytePilot(
    const char* section, const char* key, BYTE value) // 20130cfa / 203
{
    return WriteConfigurationIntPilot(section, key, value);
}

extern "C" int __stdcall WriteConfigurationDoublePilot(
    const char* section, const char* key, double value) // 20130fd9 / 44
{
    return WriteSelectedConfigurationDoublePilot(section, key, value, NULL, 0);
}

extern "C" int __stdcall WriteConfigurationIntListPilot(
    const char* section, const char* key, const int* values, DWORD count) // 20130d15 / 42
{
    return WriteSelectedConfigurationIntListPilot(section, key, values, count, NULL, 0);
}

extern "C" int __stdcall WriteConfigurationDoubleListPilot(
    const char* section, const char* key, const double* values, DWORD count) // 20131152 / 46
{
    return WriteSelectedConfigurationDoubleListPilot(section, key, values, count, NULL, 0);
}
