#include "application_boundary.h"
HWND ApplicationBoundaryPilot::GetWindowHandle(int windowId)
{
    HWND window = NULL;
    if (windowId == 0)
        window = g_mainWindowHandlePilot;
    return window;
}
