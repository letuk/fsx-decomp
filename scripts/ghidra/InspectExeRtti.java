// Read-only explicit fsx.exe inspection; no program navigation or changes.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
public class InspectExeRtti extends GhidraScript {
 public void run()throws Exception {
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new Exception("Wrong program");
  for(long va:new long[]{0x01001394L,0x01001398L,0x01001454L,0x01001458L}){
   int word=currentProgram.getMemory().getInt(toAddr(va));println(String.format("%08X word=%08X unit=%s",va,word,String.valueOf(currentProgram.getListing().getCodeUnitContaining(toAddr(va)))));
   for(Symbol s:currentProgram.getSymbolTable().getSymbols(toAddr(va)))println("  symbol="+s.getName(true)+" source="+s.getSource());
  }
  println("Filter predecessor points to: "+String.valueOf(currentProgram.getListing().getInstructionAt(toAddr(0x010022a8L))));
  println("unsaved_changes="+currentProgram.isChanged());
 }
}
