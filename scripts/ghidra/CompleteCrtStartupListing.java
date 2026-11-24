// Read-only preflight by default; apply fills only the checked CRT listing region.
import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.util.PseudoDisassembler;
import ghidra.app.util.PseudoInstruction;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.data.IntegerDataType;
import java.security.MessageDigest;

public class CompleteCrtStartupListing extends GhidraScript {
 void hash(long start,int length,String expected)throws Exception{byte[] bytes=new byte[length];currentProgram.getMemory().getBytes(toAddr(start),bytes);StringBuilder h=new StringBuilder();for(byte b:MessageDigest.getInstance("SHA-256").digest(bytes))h.append(String.format("%02X",b&255));if(!h.toString().equals(expected))throw new IllegalStateException("Live bytes differ at "+toAddr(start));}
 void append(long address,int kind,String text){Address a=toAddr(address);String old=currentProgram.getListing().getComment(kind,a);currentProgram.getListing().setComment(a,kind,old==null?text:old+"\n\n"+text);}
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new IllegalStateException("Wrong program");
  hash(0x01002363L,479,"9CD375D120EB882AF1C96350A006B23D582450A8B1B05BDFA326F35F7899106A");
  hash(0x01002d80L,40,"D252B4AD2CAB11D01EA748406A2295BAB95864D066BD0CA020689C2C1C493884");
  Function parent=currentProgram.getFunctionManager().getFunctionAt(toAddr(0x01002363L));
  if(parent==null||!parent.getName().equals("___tmainCRTStartup")||parent.getBody().getNumAddresses()!=425)throw new IllegalStateException("Stored parent changed");
  AddressSet oldBody=new AddressSet(parent.getBody());AddressSet allowed=new AddressSet(toAddr(0x01002363L),toAddr(0x01002541L));
  PseudoDisassembler decoder=new PseudoDisassembler(currentProgram);long p=0x01002363L;int count=0,missing=0;
  while(p<0x01002542L){Address a=toAddr(p);PseudoInstruction i=decoder.disassemble(a);if(i==null)throw new IllegalStateException("Decode failed");if(currentProgram.getListing().getDefinedDataContaining(a)!=null)throw new IllegalStateException("Defined data in code");Function f=currentProgram.getFunctionManager().getFunctionContaining(a);if(f!=null&&f!=parent)throw new IllegalStateException("Other function in region");if(currentProgram.getListing().getInstructionAt(a)==null)missing+=i.getLength();count++;p+=i.getLength();}
  if(p!=0x01002542L||missing!=54)throw new IllegalStateException("Expected the original two gaps totalling 54 bytes");
  long[] addresses={0x010024ddL,0x010024f1L,0x01002529L,0x0100252dL};
  String[] names={"CRT_MainExceptionFilter","CRT_MainExceptionHandler","CRT_StartupFailureFilter","CRT_StartupFailureHandler"};
  String[] descriptions={"Reads exception pointers through the shared EBP frame, stores the exception code, then calls MSVCR80 _XcptFilter. Entered through EH4 scope record1, not ordinary fall-through.","Restores the saved shared-frame ESP and stores the exception code as the CRT return code; follows managed/unmanaged exit handling.","XOR EAX,EAX; INC EAX; RET: returns1 for the startup failure scope filter. This is embedded EH4 code, not a separately modeled standard C function.","Restores shared-frame ESP and enters the return255 failure path. Entered through EH4 scope record0."};
  for(long a:addresses)for(Symbol s:currentProgram.getSymbolTable().getSymbols(toAddr(a)))if(s.getSource()==SourceType.USER_DEFINED)throw new IllegalStateException("Existing user label at "+toAddr(a));
  println("CHECKED 479-byte region, "+count+" instructions, 54 undecoded bytes, EH4 table and parent425-byte body");
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only preflight; no edits");return;}
  int tx=currentProgram.startTransaction("Complete verified embedded CRT exception-code listing");boolean commit=false;
  try{
   p=0x01002363L;
   while(p<0x01002542L){Address a=toAddr(p);PseudoInstruction expected=decoder.disassemble(a);if(currentProgram.getListing().getInstructionAt(a)==null){DisassembleCommand command=new DisassembleCommand(a,allowed,true);if(!command.applyTo(currentProgram,monitor))throw new IllegalStateException(command.getStatusMsg());}Instruction actual=currentProgram.getListing().getInstructionAt(a);if(actual==null||actual.getLength()!=expected.getLength())throw new IllegalStateException("Listing decode mismatch");p+=actual.getLength();}
   // Keep normal-flow function ownership distinct from embedded EH4 funclets.
   parent.setBody(oldBody);parent.setCallingConvention("__cdecl");parent.setReturnType(IntegerDataType.dataType,SourceType.USER_DEFINED);
   for(int n=0;n<addresses.length;n++){currentProgram.getSymbolTable().createLabel(toAddr(addresses[n]),names[n],SourceType.USER_DEFINED);append(addresses[n],CodeUnit.PLATE_COMMENT,"2026-11-02 live verified embedded CRT SEH code. "+descriptions[n]+" Descriptive reconstructed label; no standalone prototype or original source-name claim. Complete enclosing479-byte region normalized-exact; see notes/re/fsx.exe/full-crt-startup.md.");}
   int[] slots={0x14,0x18,0x20,0x24};long[] targets={0x01002529L,0x0100252dL,0x010024ddL,0x010024f1L};
   for(int n=0;n<slots.length;n++)currentProgram.getReferenceManager().addMemoryReference(toAddr(0x01002d80L+slots[n]),toAddr(targets[n]),RefType.DATA,SourceType.USER_DEFINED,0);
   append(0x01002d80L,CodeUnit.PLATE_COMMENT,"2026-11-02 verified 40-byte EH4 scope table for CRT startup. Header bytes unchanged against selected library counterpart. Two records each have enclosing level -2: record0 filter01002529/handler0100252d; record1 filter010024dd/handler010024f1. Four pointers independently mapped and checked before normalization. See notes/re/fsx.exe/full-crt-startup.md.");
   append(0x01002363L,CodeUnit.PLATE_COMMENT,"2026-11-02 complete contiguous479-byte CRT startup region01002363..01002541 normalized-exact with39 validated address fields, including54 previously undecoded EH4 filter/handler bytes. Stored425-byte normal-flow function body preserved; embedded code is labeled separately, not assigned guessed standalone signatures. EH4 scope table40 bytes, PE entry10, cookie initializer148, SEH prologue69/epilogue20 and MFC WinMain wrapper separately checked. Imported DLL implementations, complete CRT/image-helper/exception-dispatch bodies, entire EXE/layout and runtime behavior excluded. See notes/re/fsx.exe/full-crt-startup.md.");
   append(0x0100262eL,CodeUnit.PLATE_COMMENT,"2026-11-02 verified10-byte PE entry wrapper: calls the checked148-byte security-cookie initializer, then tail-jumps to CRT startup. Entry name/prototype preserved. No executable was run. See notes/re/fsx.exe/full-crt-startup.md.");
   commit=true;
  }finally{currentProgram.endTransaction(tx,commit);}
  println("Filled54 listing bytes; four labels/EH4 references/comments added; parent body425 preserved. Save separately.");
 }
}
