// Explicit fsx.exe target; default preflight is read-only, pass "apply" to edit.
import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.util.PseudoDisassembler;
import ghidra.app.util.PseudoInstruction;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.data.*;
import java.security.MessageDigest;

public class CompleteCrtInitializers extends GhidraScript {
    long[] starts={0x01002313L,0x010027d9L,0x0100299eL,0x010029ccL,0x01002802L};
    int[] lengths={75,36,41,3,36};
    String[] names={"pre_cpp_init","_RTC_Initialize","_setdefaultprecision","CrtReturnZeroFallback","_RTC_Terminate"};
    String[] hashes={"D32C33CC057767EB299B2D8E4AA452ECE0C68AF7F63AD0B874CC63374DBE774B","36715E3508F94FB7185AC2FC7E0ADA1968EE2ABBFAF5600674216A857FBC861E","E9B132A2727AD5D7B892E817350CACFBABD450F2D45AE879DADC60AD84A399A0","4BC724F3B1D0CAF4FE369C18CBA3102E6C4EA057F63FE1587E3973134A7F755E","0C92956BF9FA2038D7199D97B4F973DA838E7DE5DFA930DC811A7CB3A0C26928"};
    String[] notes={
        "CRT crtexew.obj pre_cpp_init counterpart: registers RTC termination through atexit, prepares five __getmainargs arguments (argc, argv, environment, wildcard option, startup info), stores status, and calls _amsg_exit(8) on negative status. Called from C++ initializer slot 0x0100123c before MFC/application construction. Complete 75-byte body matches with 12 validated reference fields.",
        "CRT initsect.obj RTC initialization walker counterpart. Complete 36-byte normalized body checked, including both table pointers. Original effective interval [0x0100156c,0x0100156c) is empty. The presence of this walker does not prove application compilation with /RTC. Bounds now replace an automatic one-byte placeholder.",
        "CRT fp8.obj default precision counterpart. Calls import-identified _controlfp_s(NULL,0x10000,0x30000); on nonzero result calls _invoke_watson with five null/zero arguments. Complete 41-byte normalized body checked. Bounds now replace an automatic one-byte placeholder.",
        "Reconstructed descriptive name for a shared CRT zero-return leaf: XOR EAX,EAX; RET. All three bytes raw-exact against rebuilt __setargv and __matherr counterparts. pre_c_init calls it without explicit arguments and also supplies it as the matherr callback; no unique original source name or parameter list is recoverable from this folded body. The modeled no-argument signature describes the direct call and does not reject unused callback arguments. Bounds now replace an automatic one-byte placeholder.",
        "CRT initsect.obj RTC termination walker counterpart. Complete 36-byte normalized body checked, including empty table interval [0x01001574,0x01001574). Registered through atexit by pre_cpp_init. This function's presence does not prove application compilation with /RTC."
    };
    void comment(Address a,String text){String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,a);currentProgram.getListing().setComment(a,CodeUnit.PLATE_COMMENT,old==null?text:old+"\n\n"+text);}
    public void run() throws Exception {
        if(!currentProgram.getName().equals("fsx.exe") || currentProgram.getImageBase().getOffset()!=0x01000000L)throw new IllegalStateException("Wrong program");
        FunctionManager fm=currentProgram.getFunctionManager();
        PseudoDisassembler decoder=new PseudoDisassembler(currentProgram);
        Symbol managed=currentProgram.getSymbolTable().getPrimarySymbol(toAddr(0x01004030L));
        if(managed==null || !managed.getName().equals("DAT_01004030"))throw new IllegalStateException("Managed-state name changed; preserve current state");
        for(int n=0;n<starts.length;n++) {
            Address start=toAddr(starts[n]);Function old=fm.getFunctionAt(start);
            if(old!=null && (!old.getName().equals(String.format("FUN_%08x",starts[n])) || old.getBody().getNumAddresses()!=1))throw new IllegalStateException("Unexpected existing function "+old);
            byte[] bytes=new byte[lengths[n]];currentProgram.getMemory().getBytes(start,bytes);StringBuilder hash=new StringBuilder();for(byte b:MessageDigest.getInstance("SHA-256").digest(bytes))hash.append(String.format("%02X",b&255));
            if(!hash.toString().equals(hashes[n]))throw new IllegalStateException("Live bytes differ: "+names[n]);
            for(int k=0;k<lengths[n];k++){
                Address a=start.add(k);Function containing=fm.getFunctionContaining(a);
                if(containing!=null && containing!=old)throw new IllegalStateException("Function overlap");
                if(currentProgram.getListing().getDefinedDataContaining(a)!=null)throw new IllegalStateException("Defined data overlap");
                for(Symbol s:currentProgram.getSymbolTable().getSymbols(a))if(s.getSource()==SourceType.USER_DEFINED)throw new IllegalStateException("User symbol overlap: "+s.getName(true));
            }
            Address p=start,end=start.add(lengths[n]);while(p.compareTo(end)<0){PseudoInstruction i=decoder.disassemble(p);if(i==null)throw new IllegalStateException("Decode failed");p=p.add(i.getLength());}
            if(!p.equals(end))throw new IllegalStateException("Invalid instruction boundary");
            println("CHECKED "+start+" "+lengths[n]+" -> "+names[n]);
        }
        if(currentProgram.getMemory().getInt(toAddr(0x0100123cL))!=0x01002313)throw new IllegalStateException("C++ initializer slot changed");
        if(getScriptArgs().length==0 || !getScriptArgs()[0].equals("apply")){println("Read-only preflight; no edits");return;}
        int tx=currentProgram.startTransaction("Complete verified CRT initializer bodies and approved flag name");boolean commit=false;
        try {
            for(int n=0;n<starts.length;n++){
                Address start=toAddr(starts[n]);AddressSet body=new AddressSet(start,start.add(lengths[n]-1));
                DisassembleCommand command=new DisassembleCommand(start,body,true);if(!command.applyTo(currentProgram,monitor))throw new IllegalStateException(command.getStatusMsg());
                Function f=fm.getFunctionAt(start);
                if(f==null)f=fm.createFunction(names[n],start,body,SourceType.USER_DEFINED);else{f.setBody(body);f.setName(names[n],SourceType.USER_DEFINED);}
                f.setCallingConvention("__cdecl");f.setReturnType(n==3?IntegerDataType.dataType:VoidDataType.dataType,SourceType.USER_DEFINED);
                comment(start,"2026-11-02 live verified library identification. "+notes[n]+" Evidence/scope: notes/re/fsx.exe/pre-cpp-init-and-exe-gaps.md. No complete CRT/EXE/runtime equality claim.");
                println("APPLIED "+f.getSignature()+" bytes="+f.getBody().getNumAddresses());
            }
            managed.setName("g_crtManagedImageFlag",SourceType.USER_DEFINED);
            comment(toAddr(0x01004030L),"2026-11-02 approved descriptive rename from DAT_01004030. Stores the CLR-directory presence result computed by pre_c_init; CRT startup tests it in two exit paths. Four-byte existing storage/type preserved. Reconstructed role name, not an original source identifier. See notes/re/fsx.exe/pre-c-init.md.");
            println("RENAMED DAT_01004030 -> g_crtManagedImageFlag; existing type preserved");
            commit=true;
        } finally {currentProgram.endTransaction(tx,commit);}
        println("Applied; save program separately");
    }
}
