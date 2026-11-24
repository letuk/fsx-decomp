extern "C" __declspec(dllimport) BOOL WINAPI IsDebuggerPresent();
DWORD g_mainWindowReportErrorsPilot; // 2020e0c0

// Profile phase of 2012205d, beginning at 20122332.
extern "C" BOOL __cdecl ReadMainWindowStartupProfilePilot()
{
    g_mainWindowReportErrorsPilot = IsDebuggerPresent() ? 0 : 1;
    g_mainWindowReportErrorsPilot = GetPrivateProfileIntA(
        "MAIN", "ReportErrors", g_mainWindowReportErrorsPilot, g_configurationFilePilot);
    if (!LoadLanguageModuleSourceBoundaryPilot()) return FALSE;

    g_idleUnthrottledPilot = 1;
    g_panelMenuOptionNormalPilot = GetPrivateProfileIntA(
        "MAIN", "HideMenuNormal", 0, g_configurationFilePilot);
    g_panelMenuOptionAlternatePilot = GetPrivateProfileIntA(
        "MAIN", "HideMenuFullscreen", 1, g_configurationFilePilot);
    g_panelMenuOptionOverridePilot = GetPrivateProfileIntA(
        "MAIN", "AlwaysHideMenuFullscreen", 0, g_configurationFilePilot);
    g_cursorSuppressionEnabledPilot = GetPrivateProfileIntA(
        "MAIN", "CursorAutoHide", 1, g_configurationFilePilot);
    return TRUE;
}
