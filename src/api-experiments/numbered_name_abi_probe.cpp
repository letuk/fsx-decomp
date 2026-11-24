// COFF probe for api.dll ordinal 207. This external models the original
// EDI/EAX plus stack-varargs entry. A normal C call cannot use this declaration.
extern "C" void __cdecl FormatBoundedTextRegisterBoundary();
extern "C" const char g_numberedNameProbeFormat[] = "%s.%d";

extern "C" __declspec(naked) int __stdcall FormatNumberedNameAbiProbe(
    const char*, int, char*, unsigned int)
{
    __asm {
        mov edi, edi
        push ebp
        mov ebp, esp
        mov eax, [ebp + 0x0c]
        mov ecx, [ebp + 0x08]
        push edi
        mov edi, [ebp + 0x10]
        push eax
        mov eax, [ebp + 0x14]
        push ecx
        push offset g_numberedNameProbeFormat
        call FormatBoundedTextRegisterBoundary
        add esp, 0x0c
        pop edi
        pop ebp
        ret 0x10
    }
}
