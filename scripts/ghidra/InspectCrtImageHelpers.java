// Read-only pseudo decoding of the full regions, including embedded EH code.
import ghidra.app.script.GhidraScript;
import ghidra.app.util.PseudoDisassembler;
import ghidra.app.util.PseudoInstruction;
import java.nio.file.*;
public class InspectCrtImageHelpers extends GhidraScript {
 public void run() throws Exception {
  if(!currentProgram.getName().equals("fsx.exe"))throw new Exception("Wrong program");
  PseudoDisassembler d=new PseudoDisassembler(currentProgram);StringBuilder text=new StringBuilder();
  long[][] ranges={{0x01002768L,35},{0x010028b7L,108},{0x0100263dL,15}};
  for(long[] r:ranges){long p=r[0];while(p<r[0]+r[1]){PseudoInstruction i=d.disassemble(toAddr(p));if(i==null)throw new Exception("Decode failed");text.append(String.format("%03X %s %s%n",p-r[0],i.getAddress(),i));p+=i.getLength();}if(p!=r[0]+r[1])throw new Exception("Boundary mismatch");}
  Files.write(Paths.get(getScriptArgs()[0]),text.toString().getBytes("UTF-8"));println(text.toString());
 }
}
