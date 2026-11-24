#include "heap_boundary.h"
HANDLE g_privateHeapPilot = NULL;
int g_heapInitializedPilot = 0;
int g_heapInitCallCountPilot = 0;
HINSTANCE g_errorResourceInstancePilot = NULL;
char g_heapErrorTextPilot[260] = {0};
