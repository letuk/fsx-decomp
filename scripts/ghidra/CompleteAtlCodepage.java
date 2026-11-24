// Explicit fsx.exe; default preflight is read-only, pass apply for bounded edits.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.data.*;
import java.security.MessageDigest;
public class CompleteAtlCodepage extends GhidraScript {
 void hash(long a,int length,String expected)throws Exception{byte[] b=new byte[length];currentProgram.getMemory().getBytes(toAddr(a),b);StringBuilder s=new StringBuilder();for(byte x:MessageDigest.getInstance("SHA-256").digest(b))s.append(String.format("%02X",x&255));if(!s.toString().equals(expected))throw new Exception("Live body differs at "+toAddr(a));}
 void append(long a,String text){String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,toAddr(a));currentProgram.getListing().setComment(toAddr(a),CodeUnit.PLATE_COMMENT,old==null?text:old+"\n\n"+text);}
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new Exception("Wrong program");
  hash(0x01001f1fL,101,"9C626FF20B3A418A9BAE868D1EC5B2379CECC0ED96FA9597FE9BDAC8B9E92EBF");hash(0x01001f89L,4,"A944F10555E6598867A19D0B9FE50A5CBEDEC38FE535C71784C6C368083BDD9D");hash(0x01001f92L,100,"2A3B9FE0E2819EDD4506372EEB09361D7448C99572A7CA78BDCE1AF1E6598FB2");
  FunctionManager fm=currentProgram.getFunctionManager();Function fake=fm.getFunctionAt(toAddr(0x01001f1fL)),thunk=fm.getFunctionAt(toAddr(0x01001f92L));
  if(fake==null||fake.getBody().getNumAddresses()!=101||thunk==null||thunk.getBody().getNumAddresses()!=100||!fake.getParentNamespace().getName().equals("ATL")||fm.getFunctionContaining(toAddr(0x01001f89L))!=null)throw new Exception("Existing ATL model changed");
  if(currentProgram.getMemory().getInt(toAddr(0x01004000L))!=0x01001f92)throw new Exception("Initial cached pointer differs");
  Data old=currentProgram.getListing().getDataAt(toAddr(0x01004000L));if(old==null||old.getLength()!=4)throw new Exception("Cached pointer storage differs");
  for(long a:new long[]{0x01004000L,0x01001f89L})for(Symbol s:currentProgram.getSymbolTable().getSymbols(toAddr(a)))if(s.getSource()==SourceType.USER_DEFINED)throw new Exception("Preserve user-defined symbol");
  for(long a=0x01001f89L;a<0x01001f8dL;){Instruction i=currentProgram.getListing().getInstructionAt(toAddr(a));if(i==null)throw new Exception("Expected existing real-getter instructions");a+=i.getLength();if(a>0x01001f8dL)throw new Exception("Boundary mismatch");}
  DataTypeManager dtm=currentProgram.getDataTypeManager();if(dtm.getDataType("/FSX/ATL/ThreadCodePageGetter")!=null)throw new Exception("Preserve existing callback type");
  println("CHECKED ATL101/4/100, original namespace, generic pointer4 and bootstrap initializer");
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only preflight");return;}
  int tx=currentProgram.startTransaction("Complete verified ATL real getter and cache type");boolean ok=false;
  try{
   Function real=fm.createFunction("_AtlGetThreadACPReal",fake.getParentNamespace(),toAddr(0x01001f89L),new AddressSet(toAddr(0x01001f89L),toAddr(0x01001f8cL)),SourceType.USER_DEFINED);real.setCallingConvention("__stdcall");real.setReturnType(UnsignedIntegerDataType.dataType,SourceType.USER_DEFINED);
   FunctionDefinitionDataType def=new FunctionDefinitionDataType(new CategoryPath("/FSX/ATL"),"ThreadCodePageGetter",dtm);def.setReturnType(UnsignedIntegerDataType.dataType);def.setArguments(new ParameterDefinition[0]);def.setCallingConvention("__stdcall");DataType callback=dtm.resolve(def,DataTypeConflictHandler.DEFAULT_HANDLER);DataType pointer=dtm.resolve(new PointerDataType(callback,4,dtm),DataTypeConflictHandler.DEFAULT_HANDLER);
   DataUtilities.createData(currentProgram,toAddr(0x01004000L),pointer,-1,false,DataUtilities.ClearDataMode.CLEAR_ALL_CONFLICT_DATA);
   Symbol cache=currentProgram.getSymbolTable().createLabel(toAddr(0x01004000L),"g_pfnGetThreadACP",fake.getParentNamespace(),SourceType.USER_DEFINED);cache.setPrimary();
   append(0x01001f1fL,"2026-11-02 complete101-byte ATL atltrace.obj fallback getter normalized-exact with five checked references. Obtains thread locale, queries GetLocaleInfoA type0x1004 into buffer with capacity7, parses decimal codepage, falls back to GetACP if query fails or result is0. Capacity7 does not prove original declared buffer size. Existing name/signature preserved. See notes/re/fsx.exe/atl-codepage.md.");
   append(0x01001f89L,"2026-11-02 complete four-byte ATL real getter raw-exact: PUSH3; POP EAX; RET. Returns codepage constant3 (CP_THREAD_ACP). Separate entry is supported by the selector's actual pointer target and matched library symbol; supersedes the earlier provisional neighboring-fragment interpretation. Modeled uint stdcall(void) follows the matched ATL symbol/type metadata; zero-argument machine code alone would not uniquely distinguish stdcall/cdecl. See notes/re/fsx.exe/atl-codepage.md.");
   append(0x01001f92L,"2026-11-02 complete100-byte ATL selecting thunk normalized-exact with eight checked references. Uses OSVERSIONINFOA size0x94; NT platform2 and major>=5 select the real getter, otherwise fallback. InterlockedExchange updates the same global slot then the thunk invokes that slot. Existing name/signature/local OSVERSIONINFOA preserved. See notes/re/fsx.exe/atl-codepage.md.");
   append(0x01004000L,"2026-11-02 verified ATL cached codepage getter, initial target01001f92. Generic pointer type refined to uint stdcall(void) callback pointer; default PTR__AtlGetThreadACPThunk_01004000 replaced by ATL::g_pfnGetThreadACP using the matched library counterpart name. The role/name is analysis identification, not proof of original source identifier. Atomic update/invocation alias verified. See notes/re/fsx.exe/atl-codepage.md.");
   ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
  println("Applied bounded real getter, typed/named cache and evidence comments. Existing fake/thunk names/signatures preserved; no code bytes changed. Save separately.");
 }
}
