import ghidra.app.script.GhidraScript;
import ghidra.app.util.*;
import com.google.gson.*;

// Read-only snapshot. No disassembly, function, type or comment mutations.
public class InventoryApiInitializationEvidence extends GhidraScript {
 public void run() throws Exception {
  if(!currentProgram.getName().equals("api.dll"))throw new Exception("Wrong program");
  JsonObject out=new JsonObject();out.addProperty("program","api.dll");
  JsonArray spans=new JsonArray();
  long[][] memory={{0x20120869L,1644},{0x201ecc2fL,176},{0x201e6c4dL,5468},{0x200e1db8L,21},{0x201f6000L,40},{0x201ee510L,46},{0x201ec94bL,6}};
  for(long[] s:memory){byte[] b=new byte[(int)s[1]];currentProgram.getMemory().getBytes(toAddr(s[0]),b);StringBuilder hex=new StringBuilder();for(byte v:b)hex.append(String.format("%02X",v&255));JsonObject row=new JsonObject();row.addProperty("address",String.format("%08X",s[0]));row.addProperty("length",s[1]);row.addProperty("hex",hex.toString());spans.add(row);}out.add("memory",spans);
  PseudoDisassembler d=new PseudoDisassembler(currentProgram);JsonArray instructions=new JsonArray();
  long[][] ranges={{0x201ecc2fL,0x201eccdfL},{0x201e7347L,0x201e73cbL},{0x201e7ac8L,0x201e7aedL},{0x201e8192L,0x201e81a9L},{0x201e731fL,0x201e7347L},{0x200e1db8L,0x200e1dcdL},{0x201ee510L,0x201ee53eL},{0x201ec94bL,0x201ec951L}};
  for(long[] r:ranges){long a=r[0];while(a<r[1]){PseudoInstruction i=d.disassemble(toAddr(a));if(i==null||a+i.getLength()>r[1])throw new Exception("Invalid probe boundary");JsonObject row=new JsonObject();row.addProperty("address",i.getAddress().toString());row.addProperty("instruction",i.toString());instructions.add(row);a+=i.getLength();}}
  out.add("instructions",instructions);println(new GsonBuilder().setPrettyPrinting().create().toJson(out));
 }
}
