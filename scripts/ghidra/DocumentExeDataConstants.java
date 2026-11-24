// Explicit fsx.exe; default read-only preflight, pass apply to append evidence.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
public class DocumentExeDataConstants extends GhidraScript {
 void check(long a,int word)throws Exception{if(currentProgram.getMemory().getInt(toAddr(a))!=word)throw new Exception("Anchor changed at "+toAddr(a));}
 void append(long a,String note){String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,toAddr(a));currentProgram.getListing().setComment(toAddr(a),CodeUnit.PLATE_COMMENT,old==null?note:old+"\n\n"+note);}
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new Exception("Wrong program");
  int[] targets={0x010012ac,0x010012a8,0x010012a4,0x010012a0,0x0100129c,0x01001290,0x01001284,0x0100127c};for(int n=0;n<8;n++)check(0x010012b0L+4*n,targets[n]);
  check(0x010014d0L,72);check(0x0100150cL,0x01004004);check(0x01001510L,0x01001560);check(0x01001514L,2);check(0x01001560L,0x2768);check(0x01001564L,0x2ca8);
  Data config=currentProgram.getListing().getDataAt(toAddr(0x010014d0L));if(config==null||config.getLength()!=72||!config.getDataType().getName().equals("IMAGE_LOAD_CONFIG_DIRECTORY32"))throw new Exception("Preserve existing load-config type");
  for(long[] r:new long[][]{{0x01004388L,88},{0x010043e0L,168},{0x01004384L,4}})for(int i=0;i<r[1];i++)if(currentProgram.getMemory().getByte(toAddr(r[0]+i))!=0)throw new Exception("Initial storage differs");
  for(long a:new long[]{0x010012b0L,0x010014d0L,0x01001560L,0x01004384L,0x01004388L,0x010043e0L}){Symbol s=currentProgram.getSymbolTable().getPrimarySymbol(toAddr(a));println(toAddr(a)+" symbol="+(s==null?"none":s.getName(true))+" source="+(s==null?"none":s.getSource()));}
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only data anchor preflight; unsaved_changes="+currentProgram.isChanged());return;}
  int tx=currentProgram.startTransaction("Document verified EXE constants and initial data");boolean ok=false;try{
   append(0x010012b0L,"2026-11-02 complete observed eight-pointer group32 bytes target-verified against rebuilt004032b0. Targets S/M/D/B/Val/ForceRemove/NoRemove/Delete have exact terminated bytes; rebuilt literal provenance mfcs80:stdafx.obj. Original table name/full extent and parser semantics remain unproven. Suggested descriptive name only: g_mfcTokenStringPointers; no rename or array/type change applied. See notes/re/fsx.exe/exe-data-constants.md.");
   append(0x010014d0L,"2026-11-02 all72 bytes of load configuration match after validating/normalizing only SecurityCookie(+60) and SEHandlerTable(+64) pointers. Internal Size=72, but PE data-directory Size=64 in BOTH original and rebuild: preserve this distinction. Count=2 and actual SafeSEH targets independently checked. Existing IMAGE_LOAD_CONFIG_DIRECTORY32 type preserved. See notes/re/fsx.exe/exe-data-constants.md.");
   append(0x01001560L,"2026-11-02 complete two-entry SafeSEH table8 bytes: RVAs2768 and2ca8 target EH4 forwarding helper and OLE C++ EH handler. Rebuilt linker table has same validated handler roles. Entries are RVAs, not absolute pointers; existing ImageBaseOffset32 types preserved. See notes/re/fsx.exe/exe-data-constants.md.");
   for(long[] r:new long[][]{{0x01004388L,88},{0x010043e0L,168},{0x01004384L,4}})append(r[0],"2026-11-02 complete observed initial zero span"+r[1]+" bytes agrees with source-symbol-mapped rebuilt storage, including virtual zero fill where applicable. This is pre-initializer image state, not post-constructor state. Observed extent is not proof of complete original class size; existing names/types preserved. See notes/re/fsx.exe/exe-data-constants.md.");
   ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
  println("Applied comments only; no names/types/code changed. Save separately.");
 }
}
