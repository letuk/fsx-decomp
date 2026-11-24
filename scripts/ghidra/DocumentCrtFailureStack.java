// Target fsx.exe explicitly; default preflight is read-only. No code/type ABI edits.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.data.*;
import ghidra.program.model.symbol.*;
import java.util.*;
import java.security.MessageDigest;

public class DocumentCrtFailureStack extends GhidraScript {
 DataType structure(String name,int size)throws Exception{List<DataType> all=new ArrayList<>();currentProgram.getDataTypeManager().findDataTypes(name,all);DataType found=null;for(DataType dt:all)if(dt instanceof Structure&&dt.getLength()==size){if(found!=null)throw new Exception("Ambiguous structure "+name);found=dt;}if(found==null)throw new Exception("Missing structure "+name);return found;}
 void append(long a,String text){Address address=toAddr(a);String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,address);currentProgram.getListing().setComment(address,CodeUnit.PLATE_COMMENT,old==null?text:old+"\n\n"+text);}
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new Exception("Wrong program");
  byte[] code=new byte[260];currentProgram.getMemory().getBytes(toAddr(0x01002a6dL),code);StringBuilder h=new StringBuilder();for(byte b:MessageDigest.getInstance("SHA-256").digest(code))h.append(String.format("%02X",b&255));if(!h.toString().equals("1213A48277E1B8E78E96F08B56F6D3EF41EFC0315AF2E080287C08D23715D3CD"))throw new Exception("Live reporter differs");
  long[] addresses={0x01004058L,0x010040b0L,0x010012d0L};int[] sizes={80,716,8};String[] types={"_EXCEPTION_RECORD","_CONTEXT","_EXCEPTION_POINTERS"};DataType[] definitions=new DataType[3];
  for(int n=0;n<3;n++){definitions[n]=structure(types[n],sizes[n]);for(int i=0;i<sizes[n];i++){Address a=toAddr(addresses[n]+i);if(currentProgram.getListing().getInstructionContaining(a)!=null)throw new Exception("Instruction overlaps data");for(Symbol s:currentProgram.getSymbolTable().getSymbols(a))if(s.getSource()==SourceType.USER_DEFINED)throw new Exception("Preserve user symbol "+s.getName(true));if(n<2&&currentProgram.getMemory().getByte(a)!=0)throw new Exception("Expected zero initial record data");}}
  if(currentProgram.getMemory().getInt(toAddr(0x010012d0L))!=0x01004058||currentProgram.getMemory().getInt(toAddr(0x010012d4L))!=0x010040b0)throw new Exception("Pointer pair differs");
  long[] functions={0x01002a6dL,0x01002790L,0x010027a6L,0x01002bb0L};int[] lengths={260,22,22,43};
  for(int n=0;n<functions.length;n++){Function f=currentProgram.getFunctionManager().getFunctionAt(toAddr(functions[n]));if(f==null||f.getBody().getNumAddresses()!=lengths[n])throw new Exception("Function boundaries differ");}
  println("CHECKED reporter260, probes22/22/43, Windows structures80/716/8 and zero initial data/pointer links");
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only preflight; no edits");return;}
  int tx=currentProgram.startTransaction("Document verified CRT failure records and specialized stack helpers");boolean commit=false;
  try{
   for(int n=0;n<3;n++){DataUtilities.createData(currentProgram,toAddr(addresses[n]),definitions[n],-1,false,DataUtilities.ClearDataMode.CLEAR_ALL_CONFLICT_DATA);append(addresses[n],"2026-11-02 verified CRT GS-failure data role modeled using the existing standard x86 Windows "+types[n]+" definition ("+sizes[n]+" bytes). Register/exception writes, full initial zero-filled record storage and actual exception/context pointer links checked against the rebuilt CRT. No user-defined symbols overwritten; default display names now follow the applied datatype. This is not an original FSX source-type/name claim. See notes/re/fsx.exe/crt-failure-stack.md.");}
   append(0x01002a6dL,"2026-11-02 complete260-byte CRT gs_report.obj reporter normalized-exact with33 validated references. Saves x86 context, sets GS failure code0xc0000409, consults debugger/hook and unhandled-exception APIs, then calls TerminateProcess. Context716/exception80 initial storage and exception-pointer pair8 verified. Existing name/noreturn/prototype retained; trailing machine epilogue remains present. No external DLL implementation/runtime or entire EXE equality claim. See notes/re/fsx.exe/crt-failure-stack.md.");
   append(0x01002bb0L,"2026-11-02 complete43-byte CRT chkstk.obj stack probe raw-exact. Specialized compiler-helper ABI: requested allocation in EAX, probes stack pages in0x1000 steps, adjusts ESP and transfers the return address. Not an ordinary C function; existing unknown prototype preserved instead of guessing stack arguments/returns. See notes/re/fsx.exe/crt-failure-stack.md.");
   append(0x01002790L,"2026-11-02 complete22-byte CRT alloca16.obj wrapper normalized-exact after validating tail-JMP to the checked base probe. Accounts for16-byte alignment using EAX/ESP; specialized compiler-helper ABI, not a standard C signature. Existing name/type retained. See notes/re/fsx.exe/crt-failure-stack.md.");
   append(0x010027a6L,"2026-11-02 complete22-byte CRT alloca16.obj wrapper normalized-exact after validating tail-JMP to the checked base probe. Accounts for8-byte alignment using EAX/ESP; specialized compiler-helper ABI, not a standard C signature. Existing name/type retained. See notes/re/fsx.exe/crt-failure-stack.md.");
   commit=true;
  }finally{currentProgram.endTransaction(tx,commit);}
  println("Applied Windows record types and evidence/ABI comments; automatic default data names reflect types. No explicit symbol rename, function-prototype or instruction-byte changes. Save separately.");
 }
}
