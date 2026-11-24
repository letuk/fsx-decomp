// Correct only this audit's own comment; preserve all other existing comments.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.CodeUnit;
public class CorrectPeDebugExtent extends GhidraScript {
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getMemory().getInt(toAddr(0x01001270L))!=32||currentProgram.getMemory().getByte(toAddr(0x01001537L))!=0)throw new Exception("Debug extent anchor differs");
  String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,toAddr(0x01001260L));String phrase="Existing Ghidra PDB data model31 bytes excludes one record-padding byte.";
  if(old==null||!old.contains(phrase))throw new Exception("Expected own audit comment absent");
  int tx=currentProgram.startTransaction("Correct own debug terminator description");boolean ok=false;try{currentProgram.getListing().setComment(toAddr(0x01001260L),CodeUnit.PLATE_COMMENT,old.replace(phrase,"Existing Ghidra PDB data model31 bytes does not include the path null terminator at01001537, which is included in the32-byte raw record."));ok=true;}finally{currentProgram.endTransaction(tx,ok);}
  println("Corrected own comment:32-byte record includes final path NUL; no extra padding inferred.");
 }
}
