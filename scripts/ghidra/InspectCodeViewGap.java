// Read-only, explicit original fsx.exe; no navigation/model edits.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
public class InspectCodeViewGap extends GhidraScript {
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new Exception("Wrong program");
  if(currentProgram.getMemory().getInt(toAddr(0x01001270L))!=32||currentProgram.getMemory().getInt(toAddr(0x01001510L))!=0x01001560)throw new Exception("Debug/SafeSEH anchors changed");
  int refs=0;for(long a=0x01001518L;a<0x01001580L;a++){Data d=currentProgram.getListing().getDefinedDataAt(toAddr(a));if(d!=null)println("DATA "+d.getAddress()+" "+d.getDataType().getPathName()+" size="+d.getLength());if(a>=0x01001538L&&a<0x01001560L){if(currentProgram.getMemory().getByte(toAddr(a))!=0)throw new Exception("Gap is nonzero");if(currentProgram.getListing().getInstructionContaining(toAddr(a))!=null)throw new Exception("Instruction overlaps gap");for(Reference r:currentProgram.getReferenceManager().getReferencesTo(toAddr(a))){refs++;println("GAP REFERENCE "+r);}for(Symbol s:currentProgram.getSymbolTable().getSymbols(toAddr(a)))println("GAP SYMBOL "+s.getName(true)+" source="+s.getSource());}}
  println("Gap1538..155f:40 zero bytes; incoming model references="+refs+"; unsaved_changes="+currentProgram.isChanged());
 }
}
