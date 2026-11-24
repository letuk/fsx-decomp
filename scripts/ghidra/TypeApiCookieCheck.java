import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
public class TypeApiCookieCheck extends GhidraScript {
 public void run()throws Exception{
  if(!currentProgram.getName().equals("api.dll"))throw new Exception("Wrong program");
  Function f=getFunctionAt(toAddr(0x20148b71L));
  if(f==null||!f.getName().equals("__security_check_cookie")||f.getCallFixup()!=null||f.getParameterCount()!=0)throw new Exception("Unexpected target state");
  byte[] b=new byte[15];currentProgram.getMemory().getBytes(f.getEntryPoint(),b);
  byte[] expected={(byte)0x3b,0x0d,(byte)0x84,0x62,0x1f,0x20,0x75,0x02,(byte)0xf3,(byte)0xc3,(byte)0xe9,(byte)0x97,0x04,0,0};
  if(!java.util.Arrays.equals(b,expected))throw new Exception("Unexpected cookie code");
  if(currentProgram.getMemory().getInt(toAddr(0x20148b73L))!=0x201f6284||0x20148b80L+currentProgram.getMemory().getInt(toAddr(0x20148b7cL))!=0x20149017L)throw new Exception("Unexpected cookie targets");
  if(!java.util.Arrays.asList(currentProgram.getCompilerSpec().getPcodeInjectLibrary().getCallFixupNames()).contains("security_check_cookie"))throw new Exception("Missing built-in fixup");
  int tx=currentProgram.startTransaction("Verified API cookie helper model");boolean ok=false;try{
   f.setCallingConvention("__fastcall");f.setReturnType(VoidDataType.dataType,SourceType.USER_DEFINED);
   f.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,new ParameterImpl("cookie",UnsignedIntegerDataType.dataType,currentProgram));
   f.setStackPurgeSize(0);f.setCallFixup("security_check_cookie");
   if(!f.getParameter(0).getVariableStorage().toString().equals("ECX:4"))throw new Exception("Wrong input storage");
   f.setComment("Verified api.dll15-byte cookie check: CMP ECX,[201f6284]; mismatch tail-jumps to existing noreturn ___report_gsfailure20149017, equal path RET preserves EAX/EDX. Void ECX input plus built-in security_check_cookie call fixup models successful return register preservation in decompiler; it does not prove cookie validation unnecessary or failure unreachable. No code patch. See api-initialization-flow.md.");
   println(f.getPrototypeString(true,true)+" fixup="+f.getCallFixup());ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
 }
}
