#include <fdi.h>

#pragma optimize("s", on)
// Original: api.dll 0x2014a51b
extern "C" void __stdcall ErfSetCodes(PERF error, int operation, int type)
{
    error->erfOper = operation;
    error->erfType = type;
    error->fError = TRUE;
}
#pragma optimize("", on)
