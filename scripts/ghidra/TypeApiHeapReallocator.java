import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
public class TypeApiHeapReallocator extends GhidraScript {
 public void run() throws Exception {
  if(!currentProgram.getName().equals("api.dll"))throw new Exception("Wrong program");
  Function f=currentProgram.getFunctionManager().getFunctionAt(toAddr(0x20135e98L));
  if(f==null||!f.getName().equals("FUN_20135e98")||f.getBody().getNumAddresses()!=185)throw new Exception("Unexpected target");
  byte[] ret=new byte[3];currentProgram.getMemory().getBytes(toAddr(0x20135f4eL),ret);
  if(!java.util.Arrays.equals(ret,new byte[]{(byte)0xc2,0x0c,0}))throw new Exception("Unexpected cleanup");
  int tx=currentProgram.startTransaction("Verified private heap reallocator ABI");boolean ok=false;
  try {
   DataType pointer=new PointerDataType(VoidDataType.dataType);
   f.setName("ReallocatePrivateHeapMemory",SourceType.USER_DEFINED);f.setCallingConvention("__stdcall");f.setReturnType(IntegerDataType.dataType,SourceType.USER_DEFINED);
   f.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,new ParameterImpl("failureMode",IntegerDataType.dataType,currentProgram),new ParameterImpl("sizeBytes",UnsignedIntegerDataType.dataType,currentProgram),new ParameterImpl("inOutMemory",new PointerDataType(pointer),currentProgram));
   f.setComment("Reconstructed private-heap reallocator, export ordinal107. Three stack words and RET0x0c; EAX status0/-1. NULL input delegates to AllocatePrivateHeapMemory with zero-fill0. On failed mode4 restores previous pointer. On other failure, first writes NULL to output and retries HeapReAlloc with NULL lpMem (observed PUSH0 at20135efe), not the previous allocation. Error reporting uses stored instance directly, without GetModuleHandle fallback. Preserve these observed differences; no claim of intended API behavior/runtime safety. Native source185 bytes versus original185 but register/flag-test differences remain: not a whole-body match. See notes/re/api.dll/api-callback-registry.md.");
   println(f.getPrototypeString(true,true));ok=true;
  } finally {currentProgram.endTransaction(tx,ok);}
 }
}
