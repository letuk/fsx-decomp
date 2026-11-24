#include "file_enumeration.h"
extern "C" unsigned int __stdcall NullablePathLengthPilot(const char*);
extern "C" char* __stdcall AppendResolvedPathPilot(char*, const char*, unsigned int);
extern "C" char* __stdcall CopyResolvedPathPilot(char*, const char*, unsigned int);
extern "C" int __stdcall LoadSplashStringPilot(unsigned int, char*, int);
extern "C" void __stdcall TrimTrailingSlashSourceBoundaryPilot(char*);
typedef int (WINAPI *LegacyFontOperationPilot)(LPCSTR);
typedef int (WINAPI *PrivateFontOperationPilot)(LPCSTR, DWORD, PVOID);
extern "C" void __stdcall ManageFontResourcesPilot(int add)
{
    LegacyFontOperationPilot legacy;
    const char* procedure;
    if (add != 0) {
        legacy = AddFontResourceA;
        procedure = "AddFontResourceExA";
    } else {
        legacy = RemoveFontResourceA;
        procedure = "RemoveFontResourceExA";
    }
    HMODULE gdi = GetModuleHandleA("GDI32");
    PrivateFontOperationPilot privateOperation = reinterpret_cast<PrivateFontOperationPilot>(GetProcAddress(gdi, procedure));
    FileEnumerationStatePilot* enumeration;
    if (OpenFileEnumerationPilot(&enumeration, "fonts", NULL, -1) == 0) {
        FileEnumerationRecordPilot entry;
        while (NextFileEnumerationPilot(enumeration, &entry) == 0) {
            if (entry.suffixSelector != -2) {
                char combined[260] = {0};
                char baseResource[256] = {0};
                char nameResource[256] = {0};
                char path[260];
                const char* base = "fonts";
                const char* name = entry.name;
                if ((reinterpret_cast<ULONG_PTR>(base) & 0xffff0000u) == 0) {
                    LoadSplashStringPilot(static_cast<unsigned int>(reinterpret_cast<ULONG_PTR>(base) & 0xffffu), baseResource, 256);
                    base = baseResource;
                }
                if ((reinterpret_cast<ULONG_PTR>(name) & 0xffff0000u) == 0) {
                    LoadSplashStringPilot(static_cast<unsigned int>(reinterpret_cast<ULONG_PTR>(name) & 0xffffu), nameResource, 256);
                    name = nameResource;
                }
                const char* selected = name;
                if (name[0] == '\0' || (name[1] != ':' && (name[0] != '\\' || name[1] != '\\'))) {
                    if (NullablePathLengthPilot(base) != 0) {
                        if (NullablePathLengthPilot(combined) != 0)
                            AppendResolvedPathPilot(combined, "\\", 260);
                        AppendResolvedPathPilot(combined, base, 260);
                        TrimTrailingSlashSourceBoundaryPilot(combined);
                    }
                    if (NullablePathLengthPilot(name) != 0) {
                        if (name[0] != '\\' && NullablePathLengthPilot(combined) != 0)
                            AppendResolvedPathPilot(combined, "\\", 260);
                        AppendResolvedPathPilot(combined, name, 260);
                    }
                    selected = combined;
                }
                CopyResolvedPathPilot(path, selected, 260);
                AppendResolvedPathPilot(path, ".ttf", 260);
                if (privateOperation != NULL) privateOperation(path, 0x10, NULL);
                else legacy(path);
            }
        }
        CloseFileEnumerationPilot(&enumeration);
    }
}
