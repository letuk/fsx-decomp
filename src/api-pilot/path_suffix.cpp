#include <string.h>
// Source-level storage and ABI boundaries, not linked original data/targets.
extern "C" char g_builtinSuffixesPilot[][8];
extern "C" int g_dynamicSuffixCountPilot;
extern "C" const char** g_dynamicSuffixesPilot;
extern "C" int __stdcall CopyPathSourceBoundaryPilot(const char*, char*, unsigned int);
extern "C" int __stdcall AppendPathSourceBoundaryPilot(char*, unsigned int, const char*);

// Original20134fc4: EAX selector, ECX input, stack output/RET4.
// This ordinary stdcall3 source model does not match that custom ABI.
extern "C" int __stdcall SelectSuffixSourceBoundaryPilot(int selector,
                                                        const char* input,
                                                        char* output)
{
    const char* suffix = "";
    if (selector != -2) {
        if (selector < 70)
            suffix = g_builtinSuffixesPilot[selector];
        else if (selector < g_dynamicSuffixCountPilot + 70)
            suffix = g_dynamicSuffixesPilot[selector - 70];
    }
    unsigned int inputLength = static_cast<unsigned int>(strlen(input));
    unsigned int suffixLength = static_cast<unsigned int>(strlen(suffix));
    if (inputLength > suffixLength &&
        _stricmp(input + inputLength - suffixLength, suffix) == 0)
        suffixLength = 0;
    if (CopyPathSourceBoundaryPilot(input, output, 260) < 0)
        return -1;
    if (suffixLength != 0 && AppendPathSourceBoundaryPilot(output, 260, suffix) < 0)
        return -1;
    return 0;
}
