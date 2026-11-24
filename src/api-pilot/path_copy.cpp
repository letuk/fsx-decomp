// Source boundary for original200f79d5. Original uses EAX source, EDX
// destination, stack capacity/RET4; this stdcall3 body is not ABI-equivalent.
extern "C" int __stdcall CopyPathSourceBoundaryPilot(const char* source,
                                                      char* destination,
                                                      unsigned int capacity)
{
    int result = 0;
    if (capacity == 0)
        return static_cast<int>(0x80070057u);
    while (*source != '\0') {
        *destination++ = *source++;
        if (--capacity == 0) {
            --destination;
            result = static_cast<int>(0x8007007au);
            break;
        }
    }
    *destination = '\0';
    return result;
}
