// Explicit fsx.exe target. Default preflight is read-only; apply is bounded.
import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.util.PseudoDisassembler;
import ghidra.app.util.PseudoInstruction;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.security.MessageDigest;

public class CompleteCrtImageHelpers extends GhidraScript {
 void hash(long address,int length,String expected)throws Exception{byte[] b=new byte[length];currentProgram.getMemory().getBytes(toAddr(address),b);StringBuilder h=new StringBuilder();for(byte x:MessageDigest.getInstance("SHA-256").digest(b))h.append(String.format("%02X",x&255));if(!h.toString().equals(expected))throw new IllegalStateException("Live bytes differ at "+toAddr(address));}
 void comment(long address,String text){Address a=toAddr(address);String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,a);currentProgram.getListing().setComment(a,CodeUnit.PLATE_COMMENT,old==null?text:old+"\n\n"+text);}
 public void run()throws Exception {
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new IllegalStateException("Wrong program");
  hash(0x01002768L,35,"F1B97630A8BFF5CC740E9F76C8EB3D1F19A43AFF913E988EE51EC343F92C770B");
  hash(0x010028b7L,108,"7C54646A29C9427A40D1300794D4AEE440DEE3003AFFCB8947CDDA5E7B9E975E");
  hash(0x01002dc8L,28,"4E63F6E4D602686DABED4CC15E6D044A0A9C155901C83767117CFDE774820BE3");
  FunctionManager fm=currentProgram.getFunctionManager();Function parent=fm.getFunctionAt(toAddr(0x010028b7L)),dispatch=fm.getFunctionAt(toAddr(0x01002768L));
  if(parent==null||!parent.getName().equals("__IsNonwritableInCurrentImage")||parent.getBody().getNumAddresses()!=85)throw new IllegalStateException("Stored image-helper parent changed");
  if(dispatch!=null)throw new IllegalStateException("Existing dispatch function must be preserved");
  AddressSet parentBody=new AddressSet(parent.getBody());PseudoDisassembler decoder=new PseudoDisassembler(currentProgram);long[][] ranges={{0x01002768L,35},{0x010028b7L,108}};int missing=0;
  for(long[] r:ranges){long p=r[0];while(p<r[0]+r[1]){Address a=toAddr(p);PseudoInstruction i=decoder.disassemble(a);if(i==null)throw new IllegalStateException("Decode failed");Function f=fm.getFunctionContaining(a);if(f!=null&&f!=parent)throw new IllegalStateException("Function overlap");if(currentProgram.getListing().getDefinedDataContaining(a)!=null)throw new IllegalStateException("Defined data overlap");if(currentProgram.getListing().getInstructionAt(a)==null&&r[0]==0x010028b7L)missing+=i.getLength();p+=i.getLength();}if(p!=r[0]+r[1])throw new IllegalStateException("Boundary mismatch");}
  if(missing!=23)throw new IllegalStateException("Expected 23 missing embedded EH bytes");
  for(long a:new long[]{0x01002768L,0x010028fdL,0x01002911L})for(Symbol s:currentProgram.getSymbolTable().getSymbols(toAddr(a)))if(s.getSource()==SourceType.USER_DEFINED)throw new IllegalStateException("Existing user label");
  println("CHECKED dispatch35, image-helper region108/parent85, missing EH23 and scope28");
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only preflight; no edits");return;}
  int tx=currentProgram.startTransaction("Complete checked CRT dispatch and PE helper listing");boolean commit=false;
  try {
   for(long[] r:ranges){AddressSet allowed=new AddressSet(toAddr(r[0]),toAddr(r[0]+r[1]-1));long p=r[0];while(p<r[0]+r[1]){Address a=toAddr(p);PseudoInstruction expected=decoder.disassemble(a);if(currentProgram.getListing().getInstructionAt(a)==null){DisassembleCommand command=new DisassembleCommand(a,allowed,true);if(!command.applyTo(currentProgram,monitor))throw new IllegalStateException(command.getStatusMsg());}Instruction actual=currentProgram.getListing().getInstructionAt(a);if(actual==null||actual.getLength()!=expected.getLength())throw new IllegalStateException("Listing mismatch");p+=actual.getLength();}}
   parent.setBody(parentBody);
   fm.createFunction("_except_handler4",toAddr(0x01002768L),new AddressSet(toAddr(0x01002768L),toAddr(0x0100278aL)),SourceType.USER_DEFINED);
   currentProgram.getSymbolTable().createLabel(toAddr(0x010028fdL),"PE_ImageAccessViolationFilter",SourceType.USER_DEFINED);
   currentProgram.getSymbolTable().createLabel(toAddr(0x01002911L),"PE_ImageAccessViolationHandler",SourceType.USER_DEFINED);
   comment(0x01002768L,"2026-11-02 live verified CRT chandler4gs.obj counterpart. Complete35-byte wrapper normalized-exact with three checked references: cookie-check callback, cookie address and import-identified MSVCR80 _except_handler4_common. Forwards four incoming stack words plus the two CRT prefix arguments. Minimum ABI prototype is documented separately; no full EXCEPTION_RECORD/CONTEXT layout or original parameter-name claim. Imported dispatcher implementation is outside scope. See notes/re/fsx.exe/crt-image-helpers.md.");
   Function leaf=fm.getFunctionAt(toAddr(0x01002840L));if(leaf==null||leaf.getBody().getNumAddresses()!=41)throw new IllegalStateException("Unexpected leaf image validator");leaf.setInline(true);
   comment(0x01002840L,"2026-11-02 complete41-byte body raw-exact against CRT pesect.obj counterpart. Checks MZ, PE signature and PE32 optional-header magic0x10b; not a general proof of all image bounds/content. Existing name/prototype preserved. Analysis-only inline flag enabled so the decompiler observes the actual untouched EDX register across this leaf call; this removes a false uninitialized pImageBase in the caller. This flag is not evidence of original source/compiler inlining and changes no bytes. See notes/re/fsx.exe/crt-image-helpers.md.");
   comment(0x01002870L,"2026-11-02 complete66-byte body raw-exact against CRT pesect.obj counterpart. Searches NumberOfSections using 40-byte section headers and [VirtualAddress,VirtualAddress+VirtualSize) membership. Existing name/prototype preserved. See notes/re/fsx.exe/crt-image-helpers.md.");
   comment(0x010028b7L,"2026-11-02 full108-byte CRT image-helper region normalized-exact with six checked address fields and verified28-byte EH4 scope table. Stored85-byte normal-flow body/prototype preserved; embedded23-byte filter/handler code separately listed/labeled. Checks own image section write permission; access-violation handler returns FALSE. See notes/re/fsx.exe/crt-image-helpers.md.");
   comment(0x010028fdL,"2026-11-02 verified embedded EH4 filter. Uses the shared EBP frame's exception pointers and returns1 only for exception code0xc0000005 (access violation), otherwise0. Descriptive reconstructed label, no guessed standalone signature. See notes/re/fsx.exe/crt-image-helpers.md.");
   comment(0x01002911L,"2026-11-02 verified embedded EH4 handler entry. Restores saved shared-frame ESP and joins the FALSE-return path. Descriptive reconstructed label, no standalone signature. See notes/re/fsx.exe/crt-image-helpers.md.");
   comment(0x01002dc8L,"2026-11-02 verified28-byte EH4 scope table. Header and enclosing-level bytes match selected CRT; record filter010028fd/handler01002911 independently checked. See notes/re/fsx.exe/crt-image-helpers.md.");
   currentProgram.getReferenceManager().addMemoryReference(toAddr(0x01002ddcL),toAddr(0x010028fdL),RefType.DATA,SourceType.USER_DEFINED,0);
   currentProgram.getReferenceManager().addMemoryReference(toAddr(0x01002de0L),toAddr(0x01002911L),RefType.DATA,SourceType.USER_DEFINED,0);
   comment(0x0100263dL,"2026-11-02 complete15-byte cookie checker normalized-exact with two validated address fields: cookie global and mapped report_gsfailure target. Failure reporter body remains separately unverified. Existing name, signature and caller injection preserved. See notes/re/fsx.exe/crt-image-helpers.md.");
   commit=true;
  }finally{currentProgram.endTransaction(tx,commit);}
  println("Applied bounded dispatch function,23 embedded listing bytes, two labels/scope refs and comments; parent85 preserved. Save separately.");
 }
}
