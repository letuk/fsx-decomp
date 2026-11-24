#ifndef FSX_ERROR_CALLBACK_PILOT_H
#define FSX_ERROR_CALLBACK_PILOT_H
// Provisional machine-word argument 2; original API.DLL contract still open.
typedef DWORD (__stdcall *ErrorCallbackPilot)(UINT, ULONG_PTR, const char*,
                                             DWORD, WORD, HINSTANCE);
extern "C" DWORD __stdcall ShowErrorMessage(UINT, ULONG_PTR, const char*,
                                           DWORD, WORD, HINSTANCE);
extern const char emptyErrorCaption[];
void RetainErrorCallbackPilot(ErrorCallbackPilot callback);
#endif
