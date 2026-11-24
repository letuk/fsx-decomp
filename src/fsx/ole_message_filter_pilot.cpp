// Reconstruction from fsx.exe 0x010018e0; derived name is provisional.
#include <afxwin.h>
#include <afxole.h>
#include <stddef.h>

class FSXMessageFilterPilot : public COleMessageFilter {
public:
    FSXMessageFilterPilot()
    {
        // Require installed headers to agree with original observed layout.
        C_ASSERT(sizeof(FSXMessageFilterPilot) == 0x44 &&
                 offsetof(FSXMessageFilterPilot, m_bEnableBusy) == 0x28 &&
                 offsetof(FSXMessageFilterPilot, m_bEnableNotResponding) == 0x2c);
        EnableBusyDialog(FALSE);
        EnableNotRespondingDialog(FALSE);
    }
    // Original vtable +0x60 -> 0x010018d0: MOV EAX,1; RET 4.
    virtual BOOL OnMessagePending(const MSG*) { return TRUE; }
};

BOOL InitializeOleAndMessageFilterPilot()
{
    if (FAILED(CoInitialize(NULL)))
        return FALSE;
    if (FAILED(CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL)))
        return FALSE;
    if (!AfxOleInit())
        return FALSE;
    CWinThread* thread = AfxGetThread();
    if (thread != NULL) {
        delete thread->m_pMessageFilter;
        thread->m_pMessageFilter = NULL;
        thread->m_pMessageFilter = new FSXMessageFilterPilot;
        if (AfxGetThread()->m_pMessageFilter != NULL)
            return AfxGetThread()->m_pMessageFilter->Register();
    }
    return FALSE;
}
