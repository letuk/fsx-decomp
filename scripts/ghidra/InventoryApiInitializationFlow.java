import ghidra.app.script.GhidraScript;
import ghidra.app.util.PseudoDisassembler;
import ghidra.app.util.PseudoInstruction;
import ghidra.program.model.address.Address;
import com.google.gson.*;
import java.util.*;

// Read-only bounded reachable walk. Calls are inventoried, never traversed.
public class InventoryApiInitializationFlow extends GhidraScript {
 // Recognize only observed stack-transfer instructions; PUSH r may be expanded
 // into LEA ESP,[ESP-4]; MOV [ESP],r. No arithmetic/flag-changing substitutes.
 // Region gateways skip8 descriptor bytes; integrity gateway skips16 on success.
 private long[] gateway(long ret) throws Exception {
  for(int r=0;r<8;r++)if(r!=4)for(int form=0;form<8;form++)for(int displacementForm=0;displacementForm<4;displacementForm++){
   ArrayList<Integer> pattern=new ArrayList<>();
   pushPattern(pattern,r,(form&1)!=0);pushPattern(pattern,r,(form&2)!=0);
   int first=pattern.size();leaPair(pattern,r,(displacementForm&1)!=0);append(pattern,0x89,0x44+8*r,0x24,4);
   int second=pattern.size();leaPair(pattern,r,(displacementForm&2)!=0);
   pushPattern(pattern,r,(form&4)!=0);append(pattern,0x8b,0x44+8*r,0x24,4,0xc2,4,0);
   long start=ret+3-pattern.size();byte[] b=new byte[pattern.size()];currentProgram.getMemory().getBytes(toAddr(start),b);
   boolean match=true;for(int j=0;j<b.length;j++)if(pattern.get(j)>=0&&(b[j]&255)!=pattern.get(j)){match=false;break;}if(!match)continue;
   long continuation=(Integer.toUnsignedLong(currentProgram.getMemory().getInt(toAddr(start+first+2)))+displacement(start+first+8,(displacementForm&1)!=0))&0xffffffffL;
   long target=(Integer.toUnsignedLong(currentProgram.getMemory().getInt(toAddr(start+second+2)))+displacement(start+second+8,(displacementForm&2)!=0))&0xffffffffL;
   if(continuation==ret+3&&(target==0x201ecc2fL||target==0x201ecc87L||target==0x201eccdfL))return new long[]{start,target,continuation,continuation+(target==0x201eccdfL?16:8)};
  }
  return null;
 }
 private void append(ArrayList<Integer> p,int... values){for(int v:values)p.add(v);}
 private void leaPair(ArrayList<Integer> p,int r,boolean small){append(p,0x8d,5+8*r,-1,-1,-1,-1,0x8d,(small?0x40:0x80)+9*r);if(small)append(p,-1);else append(p,-1,-1,-1,-1);}
 private int displacement(long address,boolean small)throws Exception{return small?currentProgram.getMemory().getByte(toAddr(address)):currentProgram.getMemory().getInt(toAddr(address));}
 private void pushPattern(ArrayList<Integer> p,int r,boolean expanded){if(expanded)append(p,0x8d,0x64,0x24,0xfc,0x89,4+8*r,0x24);else append(p,0x50+r);}
 public void run() throws Exception {
  if(!currentProgram.getName().equals("api.dll"))throw new Exception("Wrong program");
  String[] args=getScriptArgs();if(args.length!=0&&args.length!=2)throw new Exception("Supply entry and exclusive upper bound, hex");
  final long lo=args.length==0?0x20120869L:Long.parseLong(args[0],16),hi=args.length==0?0x20120ed5L:Long.parseLong(args[1],16);
  if(hi<=lo||hi-lo>0x10000)throw new Exception("Invalid bounded interval");
  PseudoDisassembler decoder=new PseudoDisassembler(currentProgram);
  TreeMap<Long,JsonObject> instructions=new TreeMap<>();
  ArrayDeque<Long> pending=new ArrayDeque<>();pending.add(lo);
  TreeMap<Long,Long> byteOwners=new TreeMap<>();JsonArray edges=new JsonArray(),stops=new JsonArray(),gateways=new JsonArray();
  while(!pending.isEmpty()){
   long va=pending.removeFirst();
   if(instructions.containsKey(va))continue;
   if(va<lo||va>=hi){JsonObject s=new JsonObject();s.addProperty("address",Long.toHexString(va));s.addProperty("reason","outside provisional next-function bound");stops.add(s);continue;}
   if(instructions.size()>=700)throw new Exception("Instruction bound exceeded");
   if(byteOwners.containsKey(va))throw new Exception("Target enters an already decoded instruction");
   PseudoInstruction i=decoder.disassemble(toAddr(va));if(i==null)throw new Exception("Decode failed at "+Long.toHexString(va));
   int n=i.getLength();if(va+n>hi)throw new Exception("Instruction crosses bound");
   for(int j=0;j<n;j++){if(byteOwners.containsKey(va+j))throw new Exception("Overlapping reachable decode");byteOwners.put(va+j,va);}
   JsonObject row=new JsonObject();row.addProperty("address",String.format("%08X",va));row.addProperty("bytes",n);row.addProperty("instruction",i.toString());row.addProperty("flowType",i.getFlowType().toString());
   JsonArray targets=new JsonArray();for(Address a:i.getFlows())targets.add(a.toString());row.add("targets",targets);
   StringBuilder raw=new StringBuilder();for(byte b:i.getBytes())raw.append(String.format("%02X",b&255));row.addProperty("hex",raw.toString());instructions.put(va,row);
   if(i.getFlowType().isTerminal()&&i.toString().equals("RET 0x4")){
    long[] g=gateway(va);
    if(g==null){byte[] epilogue=new byte[3];currentProgram.getMemory().getBytes(toAddr(va-3),epilogue);if(java.util.Arrays.equals(epilogue,new byte[]{(byte)0x8b,(byte)0xe5,0x5d})){row.addProperty("terminationKind","frame-restoring return, MOV ESP,EBP; POP EBP; RET4");}else{JsonObject s=new JsonObject();s.addProperty("address",String.format("%08X",va));s.addProperty("reason","unresolved RET4; possible stack transfer");stops.add(s);}}
    else{JsonObject e=new JsonObject();e.addProperty("from",String.format("%08X",g[0]));e.addProperty("ret",String.format("%08X",va));e.addProperty("gateway",String.format("%08X",g[1]));e.addProperty("descriptor",String.format("%08X",g[2]));e.addProperty("resume",String.format("%08X",g[3]));e.addProperty("descriptorBytes",g[3]-g[2]);if(g[1]==0x201eccdfL){e.addProperty("condition","integrity check success; failure redirects to201ecc0d, not traversed");}else{e.addProperty("descriptorRva",String.format("%08X",currentProgram.getMemory().getInt(toAddr(g[2]))));e.addProperty("descriptorIndex",currentProgram.getMemory().getInt(toAddr(g[2]+4)));}gateways.add(e);pending.add(g[3]);}
   }else if(i.getFlowType().isCall()){
    for(Address a:i.getFlows()){JsonObject e=new JsonObject();e.addProperty("from",String.format("%08X",va));e.addProperty("to",a.toString());e.addProperty("kind","call, callee not followed");edges.add(e);}
    if(i.getFlows().length==0){JsonObject e=new JsonObject();e.addProperty("from",String.format("%08X",va));e.addProperty("kind","indirect call, callee not followed");e.addProperty("operand",i.toString());edges.add(e);}
   }else if(i.getFlowType().isJump()){
    Address[] flow=i.getFlows();for(Address a:flow){JsonObject e=new JsonObject();e.addProperty("from",String.format("%08X",va));e.addProperty("to",a.toString());e.addProperty("kind","jump");edges.add(e);pending.add(a.getOffset());}
    if(flow.length==0){JsonObject s=new JsonObject();s.addProperty("address",String.format("%08X",va));s.addProperty("reason","unresolved computed jump");stops.add(s);}
   }
   Address next=i.getFallThrough();if(next!=null)pending.add(next.getOffset());
  }
  JsonObject out=new JsonObject();out.addProperty("program","api.dll");out.addProperty("entry",String.format("%08X",lo));out.addProperty("provisionalUpperBound",String.format("%08X",hi));out.addProperty("scope","Read-only bounded native decode; calls opaque, verified gateways follow normal/success continuations only. Detached exception/integrity-failure paths excluded. Upper bound is provisional, not complete-function proof.");
  JsonArray rows=new JsonArray();for(JsonObject r:instructions.values())rows.add(r);out.add("instructions",rows);out.add("edges",edges);out.add("gateways",gateways);out.add("stops",stops);out.addProperty("instructionCount",instructions.size());out.addProperty("uniqueInstructionBytes",byteOwners.size());
  println(new GsonBuilder().setPrettyPrinting().create().toJson(out));
 }
}
