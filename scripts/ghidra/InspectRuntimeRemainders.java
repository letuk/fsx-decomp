// Read-only inspection of remaining bounded runtime code and import targets.
import ghidra.app.script.GhidraScript;
import ghidra.app.util.PseudoDisassembler;
import ghidra.app.util.PseudoInstruction;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.nio.file.*;
public class InspectRuntimeRemainders extends GhidraScript {
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe"))throw new Exception("Wrong program");
  StringBuilder out=new StringBuilder();PseudoDisassembler decoder=new PseudoDisassembler(currentProgram);
  for(long[] r:new long[][]{{0x0100270cL,72},{0x01002754L,15},{0x01002c50L,52}}){long p=r[0];while(p<r[0]+r[1]){PseudoInstruction i=decoder.disassemble(toAddr(p));if(i==null)throw new Exception("Decode failed");out.append(String.format("%03X %s %s%n",p-r[0],i.getAddress(),i));p+=i.getLength();}if(p!=r[0]+r[1])throw new Exception("Boundary mismatch");}
  for(long iat:new long[]{0x010011ecL,0x010011a0L,0x0100119cL}){out.append("IAT "+toAddr(iat)+"\n");for(Reference r:currentProgram.getReferenceManager().getReferencesFrom(toAddr(iat))){Function f=currentProgram.getFunctionManager().getFunctionAt(r.getToAddress());out.append(r.toString()+" FUNCTION "+(f==null?"none":f.getSignature().toString())+"\n");}}
  Files.write(Paths.get(getScriptArgs()[0]),out.toString().getBytes("UTF-8"));println(out.toString());
 }
}
