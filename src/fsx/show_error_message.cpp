// Reconstruction from fsx.exe 0x01001d30..0x01001ee4, not original source text.
// Parameter 2 is one machine-word formatting value or a direct ANSI string.
// Callback callers in API.DLL have not yet established a narrower common type.
#include <afxwin.h>
#define STRSAFE_NO_DEPRECATE
#define _STRSAFE_USE_SECURE_CRT 0
#include <strsafe.h>
#include "application_interface_pilot.h"

// Reconstruction-only storage for a localized format string.
class ErrorFormatString {
public:
    const char* Load(HINSTANCE module, UINT id)
    {
        text[0] = '\0';
        memset(text + 1, 0, sizeof(text) - 1);
        LoadStringA(module, id, text, sizeof(text));
        return text;
    }

private:
    char text[512];
};

// Original: fsx.exe 0x01001d30
extern "C" DWORD __stdcall ShowErrorMessage(UINT errorId, ULONG_PTR messageArgument,
    const char* caption, DWORD messageBoxStyle, WORD iconResourceId,
    HINSTANCE hStringResourceModule)
{
    MSGBOXPARAMSA parameters = {0};
    DWORD result = 0;
    ErrorFormatString resource;
    char messageText[1024];
    HRESULT formatStatus = S_OK;

    if (hStringResourceModule != NULL) {
        if (errorId != 0) {
            const char* format = resource.Load(hStringResourceModule, errorId);
            formatStatus = StringCchPrintfA(messageText, sizeof(messageText),
                                            format, messageArgument);
        } else {
            // Original ignores truncation status on this direct-copy path.
            StringCchCopyA(messageText, sizeof(messageText),
                           reinterpret_cast<const char*>(messageArgument));
        }
    } else {
        formatStatus = StringCchPrintfA(messageText, sizeof(messageText),
            "Error ID = %d\r\n%s", errorId, messageArgument);
    }
    const bool directText = hStringResourceModule != NULL && errorId == 0;
    if (directText || (static_cast<DWORD>(formatStatus) & 0x80000000UL) == 0) {
        parameters.cbSize = sizeof(parameters);
        if (pfnGetApplication != NULL) {
            ApplicationPilot* application = pfnGetApplication();
            parameters.hwndOwner = GetLastActivePopup(application->GetWindowHandle(0));
        } else {
            parameters.hwndOwner = NULL;
        }
        parameters.lpszText = messageText;
        parameters.lpszCaption = (*caption != '\0') ? caption : NULL;
        parameters.dwStyle = messageBoxStyle | MB_SETFOREGROUND | MB_USERICON;
        parameters.hInstance = AfxGetModuleState()->m_hCurrentInstanceHandle;
        parameters.lpszIcon = MAKEINTRESOURCEA(iconResourceId);
        parameters.hwndOwner = NULL;
        parameters.dwStyle = (parameters.dwStyle & ~MB_USERICON) | MB_TOPMOST | MB_ICONWARNING;

        HANDLE thread = CreateThread(NULL, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(&MessageBoxIndirectA),
            &parameters, 0, NULL);
        if (thread != NULL) {
            WaitForSingleObject(thread, INFINITE);
            GetExitCodeThread(thread, &result);
            CloseHandle(thread);
        }
    }
    return result;
}
