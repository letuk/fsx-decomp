import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
public class TypeApiSplashImage extends GhidraScript {
 public void run()throws Exception{
  if(!currentProgram.getName().equals("api.dll"))throw new Exception("Wrong program");
  long[] va={0x2013afddL,0x20137ec0L,0x20137fb0L,0x20135114L};int[] sizes={445,62,34,53};Function[] fs=new Function[4];
  for(int j=0;j<4;j++){fs[j]=getFunctionAt(toAddr(va[j]));if(fs[j]==null||!fs[j].getName().equals("FUN_"+Long.toHexString(va[j]))||fs[j].getParameterCount()!=0||fs[j].getBody().getNumAddresses()!=sizes[j])throw new Exception("Unexpected function");}
  DataType text=new PointerDataType(CharDataType.dataType),unsigned=UnsignedIntegerDataType.dataType;
  int tx=currentProgram.startTransaction("Verified splash image and string interfaces");boolean ok=false;try{
   fs[0].setCallingConvention("__cdecl");fs[0].setReturnType(new PointerDataType(VoidDataType.dataType),SourceType.USER_DEFINED);fs[0].setStackPurgeSize(0);
   fs[0].setComment("Reconstructed splash bitmap loader. Returns LoadImageA HANDLE in EAX, preserved by cookie check. Fixed filename uires\\dlgsplash.bmp at200f34ac, IMAGE_BITMAP0, flags2010h (file/DIB). Generic integer-resource branch is statically bypassed for this literal; retained original code is not patched. Existing FUN name preserved. Source488 versus original445, not body matched. See api-initialization-flow.md.");
   fs[1].setCallingConvention("__stdcall");fs[1].setReturnType(text,SourceType.USER_DEFINED);
   fs[1].replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,new ParameterImpl("destination",text,currentProgram),new ParameterImpl("source",text,currentProgram),new ParameterImpl("capacity",unsigned,currentProgram));fs[1].setStackPurgeSize(12);
   fs[2].setCallingConvention("__stdcall");fs[2].setReturnType(unsigned,SourceType.USER_DEFINED);
   fs[2].replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,new ParameterImpl("text",text,currentProgram));fs[2].setStackPurgeSize(4);
   fs[3].updateFunction("__cdecl",new ReturnParameterImpl(VoidDataType.dataType,currentProgram),Function.FunctionUpdateType.CUSTOM_STORAGE,true,SourceType.USER_DEFINED,new ParameterImpl("path",text,new VariableStorage(currentProgram,currentProgram.getRegister("EAX")),currentProgram));fs[3].setStackPurgeSize(0);
   fs[3].setComment("Reconstructed trailing-backslash trim: path in EAX, no stack args, void; CharNextA scans multibyte characters, _mbsicmp tests final character against backslash before writing NUL. Custom storage essential; __cdecl only cleanup model. No original name/source ABI claim.");
   for(Function f:fs){println(f.getEntryPoint()+" "+f.getPrototypeString(true,true));for(Parameter p:f.getParameters())println(p.getName()+"="+p.getVariableStorage());}ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
 }
}
