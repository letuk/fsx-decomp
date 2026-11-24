import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
public class TypeApiMainWindowHelpers extends GhidraScript {
 public void run()throws Exception{
  if(!currentProgram.getName().equals("api.dll"))throw new Exception("Wrong program");
  Function hook=getFunctionAt(toAddr(0x20122834L)),child=getFunctionAt(toAddr(0x20121360L)),main=getFunctionAt(toAddr(0x2012205dL));
  if(hook==null||child==null||main==null||!hook.getName().equals("FUN_20122834")||!child.getName().equals("FUN_20121360")||!main.getName().equals("FUN_2012205d")||hook.getParameterCount()!=0||child.getParameterCount()!=0||hook.getBody().getNumAddresses()!=188||child.getBody().getNumAddresses()!=100||main.getBody().getNumAddresses()!=76)throw new Exception("Unexpected function state");
  int tx=currentProgram.startTransaction("Verified main-window helper interfaces and flow evidence");boolean ok=false;try{
   hook.setCallingConvention("__stdcall");hook.setReturnType(LongDataType.dataType,SourceType.USER_DEFINED);
   hook.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,new ParameterImpl("hookCode",IntegerDataType.dataType,currentProgram),new ParameterImpl("virtualKey",UnsignedIntegerDataType.dataType,currentProgram),new ParameterImpl("keyFlags",LongDataType.dataType,currentProgram));hook.setStackPurgeSize(12);
   hook.setComment("Reconstructed WH_KEYBOARD callback installed by2012205d with SetWindowsHookExA(2,...). Full188-byte native COFF body match after8 individually checked references; structured source, no goto. On code0 checks virtual mode slot+4 and focus, forwards selected Alt+Tab/Alt+Escape/Ctrl+Escape release as WM_SYSKEYUP, always returns CallNextHookEx result in EAX. Mode/class meaning beyond observed two-slot prefix unproven. See api-main-window.md.");
   child.setCallingConvention("__cdecl");child.setReturnType(BooleanDataType.dataType,SourceType.USER_DEFINED);child.setStackPurgeSize(0);
   child.setComment("Reconstructed FS98CHILD WNDCLASS registration, style4003h, callback20142031; bool result. Native100 bytes maps6 fields, differs only frame allocation28h versus original2ch; full comparator rejects. No fake padding added. See api-main-window.md.");
   main.setComment("WARNING: stored76-byte body stops at synthetic RET4 gateway and is incomplete. Read-only normal/success-path inventory spans2012205d..2012260e:398 instructions/1346 native bytes plus seven16-byte integrity descriptors. Gateway201eccdf calls201e44b4, advances saved return by10h on success, redirects failure separately. Seven actual frame-restoring RET4 exits return0 or1; one stack monitor argument. This signature/body is NOT repaired by assuming RET4 is an ordinary return or skipping arbitrary bytes. No body/flow/code patch. See notes/re/api.dll/api-main-window.md and artifacts/api.dll/api-main-window-flow-20261103.json.");
   if(!hook.getParameter(0).getVariableStorage().toString().equals("Stack[0x4]:4")||!hook.getParameter(2).getVariableStorage().toString().equals("Stack[0xc]:4"))throw new Exception("Wrong hook storage");
   println(hook.getPrototypeString(true,true));println(child.getPrototypeString(true,true));println("Main stored body preserved: "+main.getBody().getNumAddresses());ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
 }
}
