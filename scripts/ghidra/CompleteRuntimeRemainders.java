// Explicit fsx.exe. Default read-only preflight; pass apply for bounded model edits.
import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.data.*;
import java.security.MessageDigest;
public class CompleteRuntimeRemainders extends GhidraScript {
 long[] starts={0x0100270cL,0x01002754L,0x01002c50L,0x0100282cL,0x01002be0L,0x01002becL};
 int[] lengths={72,15,52,6,6,6};
 String[] names={"_SEH_prolog4_GS","_SEH_epilog4_GS","_allmul","_XcptFilter","_invoke_watson","_controlfp_s"};
 String[] hashes={"E9E92013FA0F9A715DA81A1A3BE4ACE3692AE2BD2C7EC22A8CDEFFD1760F2978","DC5DCBEC6270E33A1F02B561E5B75CBB63F9315323442D4266B93B0031748FDD","3D0898D998ACC8AF3EA17032FF56FD1B963C685B072F669C52D4B8267F9D0A78","3B7990FB76FCFFC0B1837F314808A64C0F439FB8AB6DBD812AF72F2C4131938E","EBFD80500D20A408696FD54773E4D97C483AB301FA1E89C4589D90A58F7D5C9F","479097194C71A66EF4BBB86CD59ED60C42160916D8E2979028732B0F4D76D044"};
 void hash(long a,int length,String expected)throws Exception {byte[] b=new byte[length];currentProgram.getMemory().getBytes(toAddr(a),b);StringBuilder h=new StringBuilder();for(byte x:MessageDigest.getInstance("SHA-256").digest(b))h.append(String.format("%02X",x&255));if(!h.toString().equals(expected))throw new Exception("Live bytes differ at "+toAddr(a));}
 void note(long a,String s){String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,toAddr(a));currentProgram.getListing().setComment(toAddr(a),CodeUnit.PLATE_COMMENT,old==null?s:old+"\n\n"+s);}
 void data(long a,DataType t,int len)throws Exception{DataUtilities.createData(currentProgram,toAddr(a),t,len,false,DataUtilities.ClearDataMode.CHECK_FOR_SPACE);}
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new Exception("Wrong program");
  FunctionManager fm=currentProgram.getFunctionManager();Function[] targets=new Function[3];long[] iats={0x010011ecL,0x010011a0L,0x0100119cL};
  for(int n=0;n<starts.length;n++){
   hash(starts[n],lengths[n],hashes[n]);Function f=fm.getFunctionAt(toAddr(starts[n]));
   if(f!=null&&(!f.getName().equals(String.format("FUN_%08x",starts[n]))||f.getBody().getNumAddresses()!=1))throw new Exception("Preserve existing function: "+f);
   for(int k=0;k<lengths[n];k++){Address a=toAddr(starts[n]+k);Function owner=fm.getFunctionContaining(a);if(owner!=null&&owner!=f)throw new Exception("Function overlap");if(currentProgram.getListing().getDefinedDataContaining(a)!=null)throw new Exception("Defined data overlap");for(Symbol s:currentProgram.getSymbolTable().getSymbols(a))if(s.getSource()==SourceType.USER_DEFINED)throw new Exception("Preserve symbol "+s);}
   println("CHECKED "+toAddr(starts[n])+" "+lengths[n]+" "+names[n]);
  }
  for(int n=0;n<3;n++){for(Reference r:currentProgram.getReferenceManager().getReferencesFrom(toAddr(iats[n]))){Function t=fm.getFunctionAt(r.getToAddress());if(t!=null&&t.isExternal()&&t.getName().equals(names[n+3]))targets[n]=t;}if(targets[n]==null)throw new Exception("External target identity changed");println("EXTERNAL "+targets[n].getSignature());}
  hash(0x0100124cL,20,"C8FBA39EF941B65C1D56DA5DD10EC87A506006ECBE9453C9AD8394181869660E");hash(0x0100129cL,20,"924C06A3F397851593CB7167860AE87F5DEC3FAC234DB2117E99ABC04AC68B4F");hash(0x010012f4L,4,"8B6078B7AB046F609689915644C739A94A1D2F23672FF4798FC9600D22C3702E");hash(0x01002d80L,100,"C768D6CF49FE24CFF87B2E65DB03664675BB3DD316391D8092A20A3F398943A5");
  for(long[] r:new long[][]{{0x0100124cL,16},{0x0100129cL,4},{0x010012a0L,2},{0x010012a4L,2},{0x010012a8L,2},{0x010012acL,2},{0x010012f4L,4},{0x01002d80L,40},{0x01002da8L,28},{0x01002dc8L,28}})for(int k=0;k<r[1];k++){Address a=toAddr(r[0]+k);if(currentProgram.getListing().getDefinedDataContaining(a)!=null||currentProgram.getListing().getInstructionContaining(a)!=null)throw new Exception("Expected undefined data at "+a);for(Symbol s:currentProgram.getSymbolTable().getSymbols(a))if(s.getSource()==SourceType.USER_DEFINED)throw new Exception("Preserve data symbol");}
  DataTypeManager dtm=currentProgram.getDataTypeManager();for(String name:new String[]{"EH4ScopeRecordModel","EH4ScopeTable1Model","EH4ScopeTable2Model"})if(dtm.getDataType("/FSX/CRT/"+name)!=null)throw new Exception("Preserve existing type");
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only preflight complete");return;}
  int tx=currentProgram.startTransaction("Complete verified runtime remainders and bounded metadata");boolean ok=false;
  try{
   for(int n=0;n<starts.length;n++){Address a=toAddr(starts[n]);AddressSet body=new AddressSet(a,a.add(lengths[n]-1));DisassembleCommand c=new DisassembleCommand(a,body,false);if(!c.applyTo(currentProgram,monitor))throw new Exception(c.getStatusMsg());Function f=fm.getFunctionAt(a);if(f==null)f=fm.createFunction(names[n],a,body,SourceType.USER_DEFINED);else{f.setBody(body);f.setName(names[n],SourceType.USER_DEFINED);}if(n>=3)f.setThunkedFunction(targets[n-3]);note(starts[n],"2026-11-02 live verified: "+names[n]+", bounded "+lengths[n]+"-byte body. See notes/re/fsx.exe/runtime-remainders.md. "+(n<2?"Special SEH helper stack/register ABI; no ordinary C prototype inferred.":n==2?"64-bit multiply helper, two stack operands, EDX:EAX result, RET16. Library identification; default prototype remains provisional.":"FF25 import identity verified from original PE and independent external signature; linked as thunk to existing external function without replacing its signature."));println("APPLIED "+f.getName(true)+" bytes="+f.getBody().getNumAddresses()+" thunk="+f.isThunk());}
   data(0x0100124cL,DWordDataType.dataType,4);data(0x01001250L,DWordDataType.dataType,4);data(0x01001254L,PointerDataType.dataType,4);data(0x01001258L,DWordDataType.dataType,4);
   for(long[] r:new long[][]{{0x0100129cL,4},{0x010012a0L,2},{0x010012a4L,2},{0x010012a8L,2},{0x010012acL,2}})data(r[0],StringDataType.dataType,(int)r[1]);
   data(0x010012f4L,PointerDataType.dataType,4);
   CategoryPath cat=new CategoryPath("/FSX/CRT");StructureDataType record=new StructureDataType(cat,"EH4ScopeRecordModel",0,dtm);record.add(IntegerDataType.dataType,"enclosingLevel",null);record.add(PointerDataType.dataType,"filterOrFinally", "Null allowed; no callback ABI inferred");record.add(PointerDataType.dataType,"handler",null);DataType rec=dtm.resolve(record,DataTypeConflictHandler.DEFAULT_HANDLER);
   for(int count=1;count<=2;count++){StructureDataType table=new StructureDataType(cat,"EH4ScopeTable"+count+"Model",0,dtm);for(String field:new String[]{"gsCookieOffset","gsCookieXorOffset","ehCookieOffset","ehCookieXorOffset"})table.add(IntegerDataType.dataType,field,null);table.add(new ArrayDataType(rec,count,12),"records",null);DataType type=dtm.resolve(table,DataTypeConflictHandler.DEFAULT_HANDLER);if(type.getLength()!=16+12*count)throw new Exception("Unexpected EH4 model size");if(count==2)data(0x01002d80L,type,40);else{data(0x01002da8L,type,28);data(0x01002dc8L,type,28);}}
   note(0x0100124cL,"2026-11-02 initializer boundaries: C++ end sentinel124c, C start1250, pre_c_init pointer1254, C end1258. Zero125c remains untyped: precise padding/data role unresolved. See runtime-remainders.md.");
   note(0x0100129cL,"2026-11-02 observed tokens Val/B/D/M/S; existing eight-pointer group12b0..12cc points to S,M,D,B,Val,ForceRemove,NoRemove,Delete. Registry-related interpretation provisional; original table name/extent unproven. Alignment zeros remain undefined. See runtime-remainders.md.");
   note(0x010012f4L,"2026-11-02 second pointer of verified empty MFC message map12f0, points to24-byte terminator12d8. Existing first pointer12f0 preserved. See compare-app-dispatch.ps1 and runtime-remainders.md.");
   for(long a:new long[]{0x01002d80L,0x01002da8L,0x01002dc8L})note(a,"2026-11-02 applied reconstructed EH4 record/table model (header16, record12), preserving existing labels/comments. Field layout/pointers checked by existing CRT comparators; model type names are not recovered original typedef names. Zero gap2dc4 remains untyped. See runtime-remainders.md.");
   ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
  println("Applied bounded listing/data changes; no original bytes changed. Save separately.");
 }
}
