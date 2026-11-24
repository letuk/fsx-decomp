#include <windows.h>
extern "C" HDC g_startupMemoryDcPilot;
extern "C" HGDIOBJ g_startupImagePilot;
extern "C" HGDIOBJ g_startupPreviousObjectPilot;
extern "C" HWND g_startupWindowPilot;
// Original2013b19a takes rectangle in EAX; explicit source-only ABI boundary.
extern "C" void __stdcall GetSplashRectangleSourceBoundaryPilot(RECT*);
extern "C" LRESULT CALLBACK StartupWindowProcedurePilot(HWND window, UINT message,
                                                       WPARAM word, LPARAM parameter)
{
    switch (message) {
    case WM_CREATE: {
        HDC dc = GetDC(window);
        g_startupMemoryDcPilot = CreateCompatibleDC(dc);
        g_startupPreviousObjectPilot = SelectObject(g_startupMemoryDcPilot, g_startupImagePilot);
        ReleaseDC(window, dc);
        break;
    }
    case WM_DESTROY:
        SelectObject(g_startupMemoryDcPilot, g_startupPreviousObjectPilot);
        DeleteObject(g_startupImagePilot);
        DeleteDC(g_startupMemoryDcPilot);
        g_startupPreviousObjectPilot = NULL;
        g_startupImagePilot = NULL;
        g_startupMemoryDcPilot = NULL;
        g_startupWindowPilot = NULL;
        PostQuitMessage(0);
        break;
    case WM_PAINT: {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(window, &paint);
        BitBlt(dc, paint.rcPaint.left, paint.rcPaint.top,
            paint.rcPaint.right - paint.rcPaint.left,
            paint.rcPaint.bottom - paint.rcPaint.top, g_startupMemoryDcPilot,
            paint.rcPaint.left, paint.rcPaint.top, SRCCOPY);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_ERASEBKGND: return 0;
    case WM_MOUSEACTIVATE: return MA_NOACTIVATEANDEAT;
    case WM_DISPLAYCHANGE:
        if (IsWindowVisible(window)) {
            RECT bounds;
            GetSplashRectangleSourceBoundaryPilot(&bounds);
            SetWindowPos(window, NULL, bounds.left, bounds.top, 800, 600, 0x2455);
        }
        break;
    case WM_NCCALCSIZE:
        if (word != 0) {
            NCCALCSIZE_PARAMS* bounds = reinterpret_cast<NCCALCSIZE_PARAMS*>(parameter);
            bounds->rgrc[2] = bounds->rgrc[0];
        }
        return 0;
    case WM_NCHITTEST: return HTCLIENT;
    }
    return DefWindowProcA(window, message, word, parameter);
}
