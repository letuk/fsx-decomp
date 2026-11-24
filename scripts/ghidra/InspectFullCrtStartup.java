// Read-only pseudo decode, including undefined listing gaps; no program edits.
import ghidra.app.script.GhidraScript;
import ghidra.app.util.PseudoDisassembler;
import ghidra.app.util.PseudoInstruction;
import java.nio.file.*;
public class InspectFullCrtStartup extends GhidraScript {
 public void run() throws Exception {
  if(!currentProgram.getName().equals("fsx.exe"))throw new IllegalStateException("Expected fsx.exe");
  PseudoDisassembler decoder=new PseudoDisassembler(currentProgram);StringBuilder output=new StringBuilder();
  long[][] ranges={{0x01002363L,0x01002542L},{0x0100262eL,0x01002638L},{0x01002940L,0x01002985L},{0x01002985L,0x01002999L},{0x010029d4L,0x01002a68L}};
  for(long[] range:ranges){long p=range[0];while(p<range[1]){PseudoInstruction i=decoder.disassemble(toAddr(p));if(i==null)throw new Exception("Decode failed");output.append(String.format("%03X %s %s%n",p-range[0],i.getAddress(),i));p+=i.getLength();}if(p!=range[1])throw new Exception("Boundary mismatch");}
  if(getScriptArgs().length!=1)throw new IllegalArgumentException("Output path required");
  Files.write(Paths.get(getScriptArgs()[0]),output.toString().getBytes("UTF-8"));println(output.toString());
 }
}
