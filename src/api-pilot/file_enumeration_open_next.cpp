#include "file_enumeration.h"
#include "heap_boundary.h"
#include <string.h>
#include <stdlib.h>

extern "C" int __stdcall AppendPathSourceBoundaryPilot(char*, unsigned int, const char*);
extern "C" int __stdcall CopyPathSourceBoundaryPilot(const char*, char*, unsigned int);
extern "C" int __stdcall CopySelectedSuffixPilot(int, char*, unsigned int);
extern "C" int __stdcall FindSuffixSelectorPilot(char*, int*);
extern "C" int __stdcall RegisterSuffixSelectorPilot(char*, int*);
// Original 20145110: stack path + ESI WIN32_FIND_DATAA output, RET4.
// Original 20144df0: stack owner + EDI cursor + ESI output, RET4.
// Ordinary source interfaces explicitly retain all inputs; custom ABI unresolved.
extern "C" HANDLE __stdcall OpenAlternateEnumerationSourceBoundaryPilot(const char*, WIN32_FIND_DATAA*);
extern "C" int __stdcall NextAlternateEnumerationSourceBoundaryPilot(void*, void*, WIN32_FIND_DATAA*);

// Original 20134495. Upper flag bits and failure behavior intentionally retained.
extern "C" int __stdcall OpenFileEnumerationPilot(FileEnumerationStatePilot** output,
    const char* directory, const char* mask, int selector)
{
    if (g_heapInitializedPilot == 0) InitializeHeapManagerPilot();
    FileEnumerationStatePilot* state = static_cast<FileEnumerationStatePilot*>(HeapAlloc(g_privateHeapPilot, 0, 0x14c));
    if (state == NULL) {
        state = static_cast<FileEnumerationStatePilot*>(HeapAlloc(g_privateHeapPilot, 0, 0x14c));
        if (state == NULL) {
            HINSTANCE instance = g_errorResourceInstancePilot;
            if (instance == NULL) instance = GetModuleHandleA(NULL);
            LoadStringA(instance, 8, g_heapErrorTextPilot, 260);
            MessageBoxA(NULL, g_heapErrorTextPilot, NULL, MB_ICONERROR);
            TerminateProcess(GetCurrentProcess(), 1);
        }
    }
    state->flags &= ~3u;
    state->suffixSelector = selector;
    char pattern[260];
    pattern[0] = '\0';
    if (directory != NULL) {
        AppendPathSourceBoundaryPilot(pattern, 260, directory);
        // Original examines the preceding byte even for an empty directory.
        if (pattern[strlen(pattern) - 1] != '\\')
            AppendPathSourceBoundaryPilot(pattern, 260, "\\");
    }
    AppendPathSourceBoundaryPilot(pattern, 260, mask == NULL ? "*" : mask);
    if (selector == -1) AppendPathSourceBoundaryPilot(pattern, 260, ".*");
    else {
        unsigned int length = static_cast<unsigned int>(strlen(pattern));
        if (CopySelectedSuffixPilot(selector, pattern + length, 260 - length) == -1) {
            HeapFree(g_privateHeapPilot, 0, state);
            return -1;
        }
    }
    state->handle = FindFirstFileA(pattern, &state->current);
    if (state->handle == INVALID_HANDLE_VALUE) {
        state->handle = OpenAlternateEnumerationSourceBoundaryPilot(pattern, &state->current);
        if (state->handle == INVALID_HANDLE_VALUE) {
            HeapFree(g_privateHeapPilot, 0, state);
            return -1;
        }
        state->flags |= 2;
    }
    state->flags |= 1;
    *output = state;
    return 0;
}

// Original 2013468c. The directory path does not write the writable field.
extern "C" int __stdcall NextFileEnumerationPilot(FileEnumerationStatePilot* state,
    FileEnumerationRecordPilot* output)
{
    for (;;) {
        if ((state->flags & 1) == 0) return -1;
        bool found = false;
        if ((state->current.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            char extension[260];
            _splitpath(state->current.cFileName, NULL, NULL, output->name, extension);
            if (FindSuffixSelectorPilot(extension, &output->suffixSelector) != 0)
                RegisterSuffixSelectorPilot(extension, &output->suffixSelector);
            output->sizeLow = state->current.nFileSizeLow;
            output->sizeHigh = state->current.nFileSizeHigh;
            output->writable = (~state->current.dwFileAttributes) & 1;
            found = true;
        } else if (strcmp(state->current.cFileName, ".") != 0 &&
                   strcmp(state->current.cFileName, "..") != 0) {
            CopyPathSourceBoundaryPilot(state->current.cFileName, output->name, 260);
            output->suffixSelector = -2;
            output->sizeLow = 0;
            output->sizeHigh = 0;
            found = true;
        }
        if (found) {
            SYSTEMTIME utc = {0};
            FileTimeToSystemTime(&state->current.ftLastWriteTime, &utc);
            output->utcTime[0] = utc.wYear;
            output->utcTime[3] = utc.wHour;
            SYSTEMTIME local = {0};
            output->utcTime[1] = utc.wMonth;
            output->utcTime[2] = utc.wDay;
            output->utcTime[4] = utc.wMinute;
            output->utcTime[5] = utc.wSecond;
            FILETIME localFileTime;
            FileTimeToLocalFileTime(&state->current.ftLastWriteTime, &localFileTime);
            FileTimeToSystemTime(&localFileTime, &local);
            output->localTime[2] = local.wDay;
            output->localTime[0] = local.wYear;
            output->localTime[1] = local.wMonth;
            output->localTime[5] = local.wSecond;
            output->localTime[3] = local.wHour;
            output->localTime[4] = local.wMinute;
        }
        state->flags &= ~1u;
        int advanced = 0;
        if ((state->flags & 2) == 0)
            advanced = FindNextFileA(state->handle, &state->current);
        else if (state->handle != NULL && state->handle != INVALID_HANDLE_VALUE)
            advanced = NextAlternateEnumerationSourceBoundaryPilot(
                *static_cast<void**>(state->handle), state->handle, &state->current);
        if (advanced != 0) state->flags |= 1;
        if (found) return 0;
    }
}
