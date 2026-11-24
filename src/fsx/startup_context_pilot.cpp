// Reconstructed MFC application startup for fsx.exe.
// The linked InitInstance comparison uses the module matching profile.
#define _WIN32_WINNT 0x0501
#include <afxwin.h>
#include <afxdisp.h>
#include <stdlib.h>
#include "application_interface_pilot.h"
#include "version_checks.cpp"

BOOL InitializeOleAndMessageFilterPilot();

class FSXAppPilot : public CWinApp {
public:
    FSXAppPilot();
    virtual ~FSXAppPilot()
    {
        if (singleInstanceEvent != NULL) {
            CloseHandle(singleInstanceEvent);
            singleInstanceEvent = NULL;
        }
    }
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    virtual int Run();
    HANDLE singleInstanceEvent;
    DECLARE_MESSAGE_MAP()

private:
    static BOOL HasRequiredD3DXVersion(HMODULE module, const char* exportName);
};
BEGIN_MESSAGE_MAP(FSXAppPilot, CWinApp)
END_MESSAGE_MAP()

// Original: fsx.exe 0x01001660
inline FSXAppPilot::FSXAppPilot() : singleInstanceEvent(NULL)
{
    singleInstanceEvent = CreateEventA(NULL, FALSE, FALSE, "FS98MAIN");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(singleInstanceEvent);
        singleInstanceEvent = NULL;
    }
}

// Original CRT XCU table constructs the window before the application.
// This order also determines their reverse atexit destruction order.
CWnd g_mainWindowPilot;
FSXAppPilot g_fsxAppPilot;

static MSG* WINAPI GetCurrentMessagePilot()
{
    return AfxGetCurrentMessage();
}

int FSXAppPilot::Run()
{
    pfnGetApplication()->RunMessageLoop(&GetCurrentMessagePilot);
    return ExitInstance();
}
int FSXAppPilot::ExitInstance()
{
    if (g_fsxAppPilot.m_pMainWnd != NULL) {
        g_fsxAppPilot.m_pMainWnd->Detach();
        g_fsxAppPilot.m_pMainWnd = NULL;
    }
    if (pfnGetApplication != NULL)
        pfnGetApplication()->UnknownSlot04();
    return CWinApp::ExitInstance();
}
// Reconstruction-only helper for the D3DX version check in InitInstance.
BOOL FSXAppPilot::HasRequiredD3DXVersion(HMODULE module, const char* exportName)
{
    typedef BOOL (WINAPI *CheckVersionProc)(UINT, UINT);
    if (module == NULL)
        return FALSE;
    CheckVersionProc checkVersion = reinterpret_cast<CheckVersionProc>(
        GetProcAddress(module, exportName));
    const UINT d3dSdkVersion = 32;
    const UINT d3dxSdkVersion = 34;
    return checkVersion != NULL && checkVersion(d3dSdkVersion, d3dxSdkVersion);
}

// Original: fsx.exe 0x010019e0
BOOL FSXAppPilot::InitInstance()
{
    AfxEnableControlContainer();
    HMODULE language = LoadLibraryA("language.dll");
    if (!CheckWindows51SP2OrLater() && !CheckWindows52SP1OrLater()) {
        ShowErrorMessage(0x17, 0, emptyErrorCaption, 0, 0x65, language);
        return FALSE;
    }

    HMODULE d3dx = LoadLibraryA("D3DX9_34.DLL");
    if (!HasRequiredD3DXVersion(d3dx, "D3DXCheckVersion")) {
        ShowErrorMessage(0x10, 0, emptyErrorCaption, 0, 0x65, language);
        return FALSE;
    }
    FreeLibrary(d3dx);

    char drive[_MAX_DRIVE];
    char directory[_MAX_DIR];
    char exeDirectory[MAX_PATH] = "";
    char exePath[MAX_PATH] = "";
    if (GetModuleFileNameA(GetModuleHandleA(NULL), exePath, _countof(exePath) - 1) != 0) {
        _splitpath_s(exePath, drive, _countof(drive), directory, _countof(directory),
                     NULL, 0, NULL, 0);
        _makepath_s(exeDirectory, _countof(exeDirectory), drive, directory, NULL, NULL);
    }

    char manifestPath[MAX_PATH] = "";
    _makepath_s(manifestPath, _countof(manifestPath), NULL, exeDirectory, "msxml4.manifest", NULL);
    ACTCTXA context = { 0 };
    context.cbSize = sizeof(context);
    context.lpSource = manifestPath;
    HANDLE activation = CreateActCtxA(&context);
    if (activation != INVALID_HANDLE_VALUE) {
        ReleaseActCtx(activation);

        char apiDllPath[MAX_PATH] = "";
        _makepath_s(apiDllPath, _countof(apiDllPath), NULL, exeDirectory, "API.DLL", NULL);
        HMODULE api = LoadLibraryA(apiDllPath);
        if (api != NULL)
            pfnGetApplication = reinterpret_cast<GetApplicationPilotFunc>(GetProcAddress(api, "GetApplication"));
        if (pfnGetApplication == NULL) {
            ShowErrorMessage(0x18, 0, emptyErrorCaption, 0, 0x65, language);
            return FALSE;
        }
        BOOL oleInitialized = InitializeOleAndMessageFilterPilot();
        if (oleInitialized == FALSE) {
            ShowErrorMessage(0x1c, reinterpret_cast<ULONG_PTR>("Unable to install message filter"),
                             emptyErrorCaption, 0, 0x65, NULL);
            return oleInitialized;
        }

        ApplicationPilot* application = pfnGetApplication();
        HINSTANCE currentInstance = AfxGetModuleState()->m_hCurrentInstanceHandle;
        BOOL startupResult = application->Initialize(currentInstance, static_cast<ULONG_PTR>(m_nCmdShow),
                                                    1, &ShowErrorMessage, singleInstanceEvent);
        if (startupResult != 0) {
            g_fsxAppPilot.m_pMainWnd = &g_mainWindowPilot;
            HWND window = pfnGetApplication()->GetWindowHandle(0);
            g_fsxAppPilot.m_pMainWnd->Attach(window);
        }
        return startupResult;
    }
    ShowErrorMessage(0x19, 0, emptyErrorCaption, 0, 0x65, language);
    return FALSE;
}

// Keep the original target callback and its actual caller in the same TU.
// This packaging is an experiment, not proof of original file boundaries.
#include "show_error_message.cpp"
#include "ole_message_filter_pilot.cpp"








