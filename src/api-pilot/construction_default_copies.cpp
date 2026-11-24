#include <windows.h>
#include <intrin.h>

extern "C" {
// Original data: api.dll 0x201fb138
DWORD g_constructionDefaults_201fb138[7];
// Original data: api.dll 0x201fb154
DWORD g_constructionDefaults_201fb154[5];
// Original data: api.dll 0x201fb168
DWORD g_constructionDefaults_201fb168[5];
// Original data: api.dll 0x201fb17c
DWORD g_constructionDefaults_201fb17c[6];
// Original data: api.dll 0x201fb194
DWORD g_constructionDefaults_201fb194[8];
// Original data: api.dll 0x201fb1b4
DWORD g_constructionDefaults_201fb1b4[5];
// Original data: api.dll 0x201fb1c8
DWORD g_constructionDefaults_201fb1c8[8];
// Original data: api.dll 0x201fb1e8
DWORD g_constructionDefaults_201fb1e8[5];
}

// Reconstruction-only helper for the default-copy operations.
template <size_t Count>
static __forceinline void* CopyConstructionDefaults(void* receiver, const DWORD (&defaults)[Count])
{
    DWORD* const firstWord = static_cast<DWORD*>(receiver);
    DWORD* const storage = firstWord == NULL ? NULL : firstWord - 1;
    __movsd(storage, defaults, Count);
    return receiver;
}

#pragma optimize("s", on)
// Original: api.dll 0x20161267
extern "C" __declspec(noinline) void* __fastcall CopyConstructionDefaults_20161267(void* receiver)
{
    return CopyConstructionDefaults(receiver, g_constructionDefaults_201fb138);
}

// Original: api.dll 0x20161285
extern "C" __declspec(noinline) void* __fastcall CopyConstructionDefaults_20161285(void* receiver)
{
    return CopyConstructionDefaults(receiver, g_constructionDefaults_201fb154);
}

// Original: api.dll 0x201612a3
extern "C" __declspec(noinline) void* __fastcall CopyConstructionDefaults_201612a3(void* receiver)
{
    return CopyConstructionDefaults(receiver, g_constructionDefaults_201fb168);
}

// Original: api.dll 0x201612c1
extern "C" __declspec(noinline) void* __fastcall CopyConstructionDefaults_201612c1(void* receiver)
{
    return CopyConstructionDefaults(receiver, g_constructionDefaults_201fb17c);
}

// Original: api.dll 0x201612df
extern "C" __declspec(noinline) void* __fastcall CopyConstructionDefaults_201612df(void* receiver)
{
    return CopyConstructionDefaults(receiver, g_constructionDefaults_201fb194);
}

// Original: api.dll 0x201612fd
extern "C" __declspec(noinline) void* __fastcall CopyConstructionDefaults_201612fd(void* receiver)
{
    return CopyConstructionDefaults(receiver, g_constructionDefaults_201fb1b4);
}

// Original: api.dll 0x2016131b
extern "C" __declspec(noinline) void* __fastcall CopyConstructionDefaults_2016131b(void* receiver)
{
    return CopyConstructionDefaults(receiver, g_constructionDefaults_201fb1c8);
}

// Original: api.dll 0x20161339
extern "C" __declspec(noinline) void* __fastcall CopyConstructionDefaults_20161339(void* receiver)
{
    return CopyConstructionDefaults(receiver, g_constructionDefaults_201fb1e8);
}
#pragma optimize("", on)
