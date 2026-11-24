// Explicit fsx.exe. Default read-only; pass apply to append evidence comments.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
public class DocumentExeRttiAbsence extends GhidraScript {
 public void run()throws Exception {
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new Exception("Wrong program");
  long[] addresses={0x01001394L,0x01001398L,0x01001454L,0x01001458L};int[] words={0x73,0x010020d4,0x010022a8,0x010022b4};
  for(int i=0;i<addresses.length;i++)if(currentProgram.getMemory().getInt(toAddr(addresses[i]))!=words[i])throw new Exception("Vtable anchor changed");
  if(!currentProgram.getSymbolTable().getPrimarySymbol(toAddr(0x01001398L)).getName().equals("FSXApp_vftable")||!currentProgram.getSymbolTable().getPrimarySymbol(toAddr(0x01001458L)).getName().equals("FSXMessageFilter_vftable"))throw new Exception("Established table names changed");
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only anchor preflight; unsaved_changes="+currentProgram.isChanged());return;}
  int tx=currentProgram.startTransaction("Document observed EXE RTTI absence");boolean ok=false;try{
   for(long a:new long[]{0x01001398L,0x01001458L}){String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,toAddr(a));String note="2026-11-02 live RTTI audit: no local x86 complete-object-locator pointer immediately before this known primary vtable. Before application01001398 is00000073, the end of Error ID format string; before filter01001458 is010022a8, the previous application's last observed slot (import JMP). Non-resource original image has no scanned .?AV/.?AU type names. Source-identical /GR- rebuild removes added class RTTI and retains all existing body/slot gates. This supports disabled RTTI for reconstruction, not proof of a unique original flag, external DLL metadata absence or original class names. Existing table names/types preserved. See notes/re/fsx.exe/exe-rtti.md.";currentProgram.getListing().setComment(toAddr(a),CodeUnit.PLATE_COMMENT,old==null?note:old+"\n\n"+note);println("Comment appended at "+toAddr(a));}ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
 }
}
