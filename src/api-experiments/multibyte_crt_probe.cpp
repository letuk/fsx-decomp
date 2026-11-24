#include <mbstring.h>

// Satisfy the VC8 static CRT startup member selected by the linker.
// The comparison DLL has no entry point.
int main()
{
    return 0;
}

// Keep each CRT function in the linked image with a distinct exported caller.
extern "C" __declspec(dllexport) unsigned char* ProbeMultibyteByteCopy(
    unsigned char* destination, const unsigned char* source, size_t count)
{
    return _mbsnbcpy(destination, source, count);
}

extern "C" __declspec(dllexport) unsigned char* ProbeMultibyteCharacterCopy(
    unsigned char* destination, const unsigned char* source, size_t count)
{
    return _mbsncpy(destination, source, count);
}
