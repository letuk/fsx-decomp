// Original fsx.exe only. Default read-only, pass apply for comment-only update.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
public class DocumentCodeViewGap extends GhidraScript {
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new Exception("Wrong program");
  if(currentProgram.getMemory().getInt(toAddr(0x01001270L))!=32||currentProgram.getMemory().getInt(toAddr(0x01001510L))!=0x01001560||currentProgram.getMemory().getInt(toAddr(0x01001560L))!=0x2768)throw new Exception("Anchor changed");
  for(long a=0x01001538L;a<0x01001560L;a++)if(currentProgram.getMemory().getByte(toAddr(a))!=0||currentProgram.getListing().getInstructionContaining(toAddr(a))!=null||currentProgram.getReferenceManager().getReferencesTo(toAddr(a)).hasNext())throw new Exception("Zero-gap model changed");
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only zero-gap preflight; unsaved_changes="+currentProgram.isChanged());return;}
  int tx=currentProgram.startTransaction("Document zero-gap layout probe without inventing original data");boolean ok=false;try{
   String text="2026-11-03 live verified:40 zero bytes after the32-byte CodeView record (end-exclusive01001538), before SafeSEH table01001560. No instructions or incoming references in current model. Baseline37 has8 zero bytes here with SafeSEH01001540. A deliberately synthetic32-byte .rdata$r source-object probe restores original table/RTC placement and .text virtual9654, preserving all prior checks; it also changes ordering of four symbols representing three SDK leaf bodies, so it is not a pure uniform code shift. Probe is optional diagnostic only, default/golden37 unchanged. No original variable/type/identifier or release/linker cause recovered. Do not infer an original char array from zero bytes. See notes/re/fsx.exe/codeview-gap.md.";
   String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,toAddr(0x01001538L));currentProgram.getListing().setComment(toAddr(0x01001538L),CodeUnit.PLATE_COMMENT,old==null?text:old+"\n\n"+text);ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
  println("Appended evidence only; no symbols/types/bytes/functions changed. Save separately.");
 }
}
