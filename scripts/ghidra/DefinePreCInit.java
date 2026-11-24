// Explicit fsx.exe target. Default preflight is read-only; "apply" is bounded.
import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.util.PseudoDisassembler;
import ghidra.app.util.PseudoInstruction;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.data.IntegerDataType;
import java.security.MessageDigest;

public class DefinePreCInit extends GhidraScript {
    public void run() throws Exception {
        if(!currentProgram.getName().equals("fsx.exe") || currentProgram.getImageBase().getOffset()!=0x01000000L)
            throw new IllegalStateException("Expected fsx.exe at original base");
        Address start=toAddr(0x01002547L), end=toAddr(0x01002628L);
        byte[] bytes=new byte[226];currentProgram.getMemory().getBytes(start,bytes);
        StringBuilder hash=new StringBuilder();for(byte b:MessageDigest.getInstance("SHA-256").digest(bytes))hash.append(String.format("%02X",b&255));
        if(!hash.toString().equals("F8AAE926BA1E3E8B1E26F59B5CD479CD90336AC0B8EDE531C9AE0DD27A21CCBD"))throw new IllegalStateException("Live original body differs");
        if(currentProgram.getMemory().getInt(toAddr(0x01001254L))!=0x01002547)throw new IllegalStateException("Initializer slot changed");
        for(long p=0x01002547L;p<=0x01002628L;p++) {
            Address a=toAddr(p);
            if(currentProgram.getFunctionManager().getFunctionContaining(a)!=null || currentProgram.getListing().getDefinedDataContaining(a)!=null)
                throw new IllegalStateException("Existing function or defined data at "+a);
            for(Symbol s:currentProgram.getSymbolTable().getSymbols(a))if(s.getSource()==SourceType.USER_DEFINED)
                throw new IllegalStateException("Preserve existing user symbol "+s.getName(true));
        }
        PseudoDisassembler decoder=new PseudoDisassembler(currentProgram);
        int count=0;Address p=start;
        while(p.compareTo(end)<=0){PseudoInstruction i=decoder.disassemble(p);if(i==null)throw new IllegalStateException("Decode failed");println(p+" "+i);count++;p=p.add(i.getLength());}
        if(!p.equals(end.add(1)))throw new IllegalStateException("Instruction boundary mismatch");
        println("Read-only preflight: "+count+" instructions, 226 bytes, pinned live hash");
        if(getScriptArgs().length==0 || !getScriptArgs()[0].equals("apply"))return;
        int tx=currentProgram.startTransaction("Define verified CRT pre_c_init");boolean commit=false;
        try {
            AddressSet body=new AddressSet(start,end);
            DisassembleCommand command=new DisassembleCommand(start,body,true);
            if(!command.applyTo(currentProgram,monitor))throw new IllegalStateException(command.getStatusMsg());
            p=start;int actual=0;while(p.compareTo(end)<=0){Instruction i=currentProgram.getListing().getInstructionAt(p);if(i==null)throw new IllegalStateException("Disassembly gap");p=p.add(i.getLength());actual++;}
            if(!p.equals(end.add(1)) || actual!=count)throw new IllegalStateException("Disassembly boundary/count changed");
            Function f=currentProgram.getFunctionManager().createFunction("pre_c_init",start,body,SourceType.USER_DEFINED);
            f.setCallingConvention("__cdecl");f.setReturnType(IntegerDataType.dataType,SourceType.USER_DEFINED);
            String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,start);
            String note="2026-11-02 live verified CRT library identification: pre_c_init counterpart in msvcrt:crtexew.obj. Complete 226-byte body normalized-exact with 27 validated address fields; dependencies RTC Initialize, matherr fallback, and default precision separately checked. Checks own PE headers for a CLR directory, stores managed-image state, sets GUI app type, initializes encoded onexit pointers, file I/O globals, floating-point and optional thread-locale settings; returns 0. Called from C initializer slot 0x01001254 before C++ constructors. Name identifies the library counterpart, not an original FSX source filename. No full CRT/external/runtime equality claim. See notes/re/fsx.exe/pre-c-init.md.";
            currentProgram.getListing().setComment(start,CodeUnit.PLATE_COMMENT,old==null?note:old+"\n\n"+note);
            println("DEFINED "+f.getSignature()+" bounded to "+body);
            commit=true;
        } finally {currentProgram.endTransaction(tx,commit);}
        println("Applied; save program separately");
    }
}
