// Explicit fsx.exe; default read-only. 'complete-listing' decodes only validated bodies.
import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.util.PseudoDisassembler;
import ghidra.app.util.PseudoInstruction;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
public class VerifyRuntimeRemainders extends GhidraScript {
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe"))throw new Exception("Wrong program");
  long[][] ranges={{0x0100270cL,72},{0x01002754L,15},{0x01002c50L,52},{0x0100282cL,6},{0x01002be0L,6},{0x01002becL,6},{0x0100299eL,41}};
  String[] hashes={"E9E92013FA0F9A715DA81A1A3BE4ACE3692AE2BD2C7EC22A8CDEFFD1760F2978","DC5DCBEC6270E33A1F02B561E5B75CBB63F9315323442D4266B93B0031748FDD","3D0898D998ACC8AF3EA17032FF56FD1B963C685B072F669C52D4B8267F9D0A78","3B7990FB76FCFFC0B1837F314808A64C0F439FB8AB6DBD812AF72F2C4131938E","EBFD80500D20A408696FD54773E4D97C483AB301FA1E89C4589D90A58F7D5C9F","479097194C71A66EF4BBB86CD59ED60C42160916D8E2979028732B0F4D76D044"};
  for(int n=0;n<hashes.length;n++){byte[] b=new byte[(int)ranges[n][1]];currentProgram.getMemory().getBytes(toAddr(ranges[n][0]),b);StringBuilder h=new StringBuilder();for(byte x:java.security.MessageDigest.getInstance("SHA-256").digest(b))h.append(String.format("%02X",x&255));if(!h.toString().equals(hashes[n]))throw new Exception("Body hash changed");}
  byte[] fp=new byte[41];currentProgram.getMemory().getBytes(toAddr(0x0100299eL),fp);StringBuilder fh=new StringBuilder();for(byte x:java.security.MessageDigest.getInstance("SHA-256").digest(fp))fh.append(String.format("%02X",x&255));if(!fh.toString().equals("E9B132A2727AD5D7B892E817350CACFBABD450F2D45AE879DADC60AD84A399A0"))throw new Exception("Precision body changed");
  boolean apply=getScriptArgs().length>0&&getScriptArgs()[0].equals("complete-listing");PseudoDisassembler decoder=new PseudoDisassembler(currentProgram);
  int tx=currentProgram.startTransaction("Verify bounded runtime listing");boolean ok=false;
  try{for(long[] r:ranges){Address a=toAddr(r[0]),end=a.add(r[1]);Function f=currentProgram.getFunctionManager().getFunctionAt(a);if(f==null||(f.getBody().getNumAddresses()!=r[1] && !(r[0]==0x0100299eL && f.getBody().getNumAddresses()==38)))throw new Exception("Body model differs");int missing=0;while(a.compareTo(end)<0){PseudoInstruction p=decoder.disassemble(a);if(p==null||a.add(p.getLength()).compareTo(end)>0)throw new Exception("Instruction boundary mismatch");Instruction i=currentProgram.getListing().getInstructionAt(a);if(i==null){missing+=p.getLength();if(apply){if(currentProgram.getListing().getDefinedDataContaining(a)!=null)throw new Exception("Data conflict");DisassembleCommand c=new DisassembleCommand(a,new AddressSet(a,a.add(p.getLength()-1)),false);if(!c.applyTo(currentProgram,monitor))throw new Exception(c.getStatusMsg());i=currentProgram.getListing().getInstructionAt(a);}}if(apply&&(i==null||i.getLength()!=p.getLength()))throw new Exception("Incomplete decode");a=a.add(p.getLength());}println(f.getName(true)+" comparedRegion="+r[1]+" storedBody="+f.getBody().getNumAddresses()+" missingListingBytes="+missing+(apply?" completed":""));}
   for(long[] r:new long[][]{{0x01001254L,4},{0x010012f4L,4},{0x01002d80L,40},{0x01002da8L,28},{0x01002dc8L,28}}){Data d=currentProgram.getListing().getDataAt(toAddr(r[0]));if(d==null||d.getLength()!=r[1])throw new Exception("Data type readback differs");println(d.getAddress()+" "+d.getDataType().getPathName()+" size="+d.getLength());}
   for(long a:new long[]{0x0100282cL,0x01002be0L,0x01002becL}){Function f=currentProgram.getFunctionManager().getFunctionAt(toAddr(a));if(!f.isThunk())throw new Exception("Thunk link missing");println(f.getName()+" -> "+f.getThunkedFunction(true).getSignature());}ok=true;
   if(apply){Address a=toAddr(0x0100299eL);String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,a);currentProgram.getListing().setComment(a,CodeUnit.PLATE_COMMENT,old+"\n\n2026-11-02 after binding the _invoke_watson import thunk to its existing noreturn external function, analysis stores38 normal-flow bytes. The complete compared41-byte library region includes three unreachable stack-cleanup bytes010029c2..010029c4; those bytes are now explicitly listed. Do not confuse contiguous comparison extent with reachable function body. See runtime-remainders.md.");}
  }finally{currentProgram.endTransaction(tx,ok);}
  println("Program unsaved_changes="+currentProgram.isChanged());
 }
}
