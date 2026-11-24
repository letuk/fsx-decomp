// Target fsx.exe explicitly. Default is read-only; pass "apply" after backup.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.data.VoidDataType;
import ghidra.program.model.symbol.SourceType;

public class AnnotateStaticInitialization extends GhidraScript {
    private final long[] starts = {0x01002ce0L,0x01002d00L,0x01002d16L,0x01002d40L,0x01002d50L};
    private final int[] lengths = {22,17,18,10,46};
    private final String[] names = {"InitializeGlobalMainWindow","InitializeGlobalFSXApp","InitializeMfcApplicationState","DestroyGlobalMainWindow","DestroyGlobalFSXApp"};
    private final String[] descriptions = {
        "Constructs g_mainWindow through the MFC CWnd constructor, then registers DestroyGlobalMainWindow through atexit. Original C++ initializer slot 0x01001244 precedes the application slot.",
        "Calls the application event constructor at 0x01001660 for g_fsxApp, then registers DestroyGlobalFSXApp through atexit. Original C++ initializer slot 0x01001248 follows the window slot.",
        "MFC static initializer: calls AfxInitialize(0, 0x800), stores AL at 0x0100437c. Original C++ initializer slot 0x01001240. SDK MAP counterpart: dynamic initializer for _afxInitAppState.",
        "Registered global-window atexit callback: sets ECX to g_mainWindow and tail-calls the MFC CWnd destructor.",
        "Registered global-application atexit callback: restores the derived vtable, closes and clears the event at g_fsxApp+0xa4 if nonnull, then tail-calls the MFC CWinApp destructor."
    };
    private void appendComment(long address, String text) {
        Address a=toAddr(address);
        String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,a);
        currentProgram.getListing().setComment(a,CodeUnit.PLATE_COMMENT,old==null?text:old+"\n\n"+text);
    }
    public void run() throws Exception {
        if (!currentProgram.getName().equals("fsx.exe") || currentProgram.getImageBase().getOffset()!=0x01000000L)
            throw new IllegalStateException("Expected original fsx.exe at base 0x01000000");
        if (!"61fae3f2b6f4338bb9d45fcc87a8fcaa1c56876088bfd9dd2bf950b1c5a57735".equalsIgnoreCase(currentProgram.getExecutableSHA256()))
            throw new IllegalStateException("Original executable hash differs");
        FunctionManager fm=currentProgram.getFunctionManager();
        for(int n=0;n<starts.length;n++) {
            long p=starts[n], end=p+lengths[n];
            while(p<end) {
                Instruction i=currentProgram.getListing().getInstructionAt(toAddr(p));
                if(i==null || fm.getFunctionContaining(toAddr(p))!=null)
                    throw new IllegalStateException("Missing instruction or existing function at "+toAddr(p));
                p+=i.getLength();
            }
            if(p!=end)throw new IllegalStateException("Instruction boundary differs");
            println("CHECKED "+toAddr(starts[n])+" "+lengths[n]+" "+names[n]);
        }
        if(getScriptArgs().length==0 || !getScriptArgs()[0].equals("apply")){println("Read-only preflight; no edits");return;}
        int transaction=currentProgram.startTransaction("Document verified static initialization wrappers");
        boolean commit=false;
        try {
            for(int n=0;n<starts.length;n++) {
                Address a=toAddr(starts[n]);
                Function f=fm.createFunction(names[n],a,new AddressSet(a,a.add(lengths[n]-1)),SourceType.USER_DEFINED);
                f.setCallingConvention("__cdecl");
                f.setReturnType(VoidDataType.dataType,SourceType.USER_DEFINED);
                appendComment(starts[n],"2026-11-02 live verified; reconstructed descriptive name, not an original source identifier. "+descriptions[n]+" Complete "+lengths[n]+"-byte body matches the VS2005 SP1 rebuild after target-validated address normalization. See notes/re/fsx.exe/static-initialization-matching.md; no full CRT/runtime equivalence claim.");
                println("CREATED "+f.getSignature());
            }
            appendComment(0x01001238L,"2026-11-02 live verified C++ initializer range [0x01001238,0x0100124c): null sentinel; CRT pre_cpp_init 0x01002313; MFC application-state initializer 0x01002d16; global CWnd initializer 0x01002ce0; global FSX application initializer 0x01002d00. Null end sentinel at 0x0100124c. Called through _initterm by CRT startup. CWnd precedes the application regardless of wrapper address ordering. Current SP1-compiler/RTM-CRT rebuild matches C and C++ initializer table roles/order; full CRT body equality remains unproven. See notes/re/fsx.exe/crt-library-context.md.");
            appendComment(0x01001250L,"2026-11-02 live verified C initializer range [0x01001250,0x01001258): null sentinel and pre_c_init 0x01002547; null end sentinel at 0x01001258. Installed SP1 crtexew.obj additionally registers __CxxSetUnhandledExceptionFilter in .CRT$XIY. Current default uses pinned RTM msvcrt.lib with MFC library precedence and reproduces original table roles/order. Full CRT body equality and original compiler/library revision remain unproven. See notes/re/fsx.exe/crt-library-context.md.");
            commit=true;
        } finally {currentProgram.endTransaction(transaction,commit);}
        println("Five functions and two table comments applied; program not saved by this script");
    }
}
