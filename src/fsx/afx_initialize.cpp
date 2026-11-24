#include <afxwin.h>
#include <mbctype.h>
#include <stddef.h>

C_ASSERT(offsetof(AFX_MODULE_STATE, m_bDLL) == 0x14);
C_ASSERT(offsetof(AFX_MODULE_STATE, m_dwVersion) == 0x44);

// Original: fsx.exe 0x01002c0d
BOOL AFXAPI InitializeMfcModuleState(BOOL isDll, DWORD mfcVersion)
{
    AFX_MODULE_STATE* state = AfxGetModuleState();
    state->m_bDLL = static_cast<BYTE>(isDll);
    state->m_dwVersion = mfcVersion;
    if (!isDll)
        _setmbcp(_MB_CP_ANSI);
    return TRUE;
}
