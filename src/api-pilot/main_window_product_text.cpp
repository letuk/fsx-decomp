#include <shellapi.h>

// 20120669..201206e4. The boundary carries the prior status through a
// failed region entry, or returns the protected call's result after leave.
extern "C" int __cdecl QueryProtectedProductValuePilot(int entryFailureValue);

static int AppendProductTextPilot(char* output, unsigned int capacity, const char* text)
{
    if (*text == '\0') return 0;
    if (capacity > 0x7fffffffu) return static_cast<int>(0x80070057u);
    const char* separator = *output == '\0' ? "?" : "&";
    char* end = static_cast<char*>(memchr(output, '\0', capacity));
    if (end == NULL) return static_cast<int>(0x80070057u);
    int status = CopyPathSourceBoundaryPilot(separator, end, capacity - (end - output));
    if (status < 0) return status;
    end += 1;
    return CopyPathSourceBoundaryPilot(text, end, capacity - (end - output));
}

// 201205a6 / export 16. Appends to caller-owned text and keeps partial output
// when a later append fails.
extern "C" int __stdcall QueryProductTextSourcePilot(
    HWND window, DWORD contextId, char* output, unsigned int capacity)
{
    char field[256];
    int status = AppendProductTextPilot(output, capacity, "product=FSX");
    if (status < 0) return status;
    status = AppendProductTextPilot(output, capacity, "ver=10.00");
    if (status < 0) return status;
    status = AppendProductTextPilot(output, capacity, "bld=61637.00");
    if (status < 0) return status;

    int protectedValue = QueryProtectedProductValuePilot(status);
    FormatBoundedTextPilot(field, sizeof(field), "lic=%d", protectedValue);
    status = AppendProductTextPilot(output, capacity, field);
    if (status < 0) return status;

    if ((contextId & 0xff00) <= 0x200 && (contextId & 0xff) <= 4) {
        FormatBoundedTextPilot(field, sizeof(field), "contextID=%04lX", contextId);
        status = AppendProductTextPilot(output, capacity, field);
        if (status < 0) return status;
    }
    RECT bounds = { 0, 0, 0, 0 };
    if (IsWindow(window)) {
        GetClientRect(window, &bounds);
        FormatBoundedTextPilot(field, sizeof(field), "cx=%d&cy=%d",
            bounds.right - bounds.left, bounds.bottom - bounds.top);
        status = AppendProductTextPilot(output, capacity, field);
        if (status < 0) return status;
    }
    FormatBoundedTextPilot(field, sizeof(field), "lcid=%04lX", GetUserDefaultLCID());
    status = AppendProductTextPilot(output, capacity, field);
    if (status < 0) return status;

    TIME_ZONE_INFORMATION timeZone = { 0 };
    if (GetTimeZoneInformation(&timeZone) == TIME_ZONE_ID_UNKNOWN) return status;
    FormatBoundedTextPilot(field, sizeof(field), "tbias=%d", timeZone.Bias);
    return AppendProductTextPilot(output, capacity, field);
}

// 2012629e. The WM_COMMAND caller keeps virtual dispatch at +0x64.
HINSTANCE IdleControllerBoundaryPilot::LaunchShellTargetPilot(HWND window, LPCSTR verb,
    LPCSTR file, LPCSTR parameters, LPCSTR directory, int show)
{
    MainWindowSizingStatePilot& state = ReadMainWindowSizingStatePilot(this);
    if (state.modeCallbacksEnabled != 0)
        SendMessageA(state.window, WM_COMMAND, 0x1501, 0x50524f47);
    if (!IsWindow(window)) window = state.window;
    return ShellExecuteA(window, verb, file, parameters, directory, show);
}
