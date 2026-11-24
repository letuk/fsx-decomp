// Use actual Ghidra datatype objects: textual prototype parser cannot express
// this callback-return typedef and nested void callback reliably in this bridge.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.data.*;
import ghidra.program.model.symbol.SourceType;
public class TypeCrtExitCallbacks extends GhidraScript {
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe"))throw new Exception("Wrong program");
  Function onexit=currentProgram.getFunctionManager().getFunctionAt(toAddr(0x01002651L)),atexit=currentProgram.getFunctionManager().getFunctionAt(toAddr(0x010026f5L));
  if(onexit==null||!onexit.getName().equals("__onexit")||onexit.getBody().getNumAddresses()!=150||atexit==null||!atexit.getName().equals("atexit")||atexit.getBody().getNumAddresses()!=18)throw new Exception("Unexpected functions");
  DataTypeManager dtm=currentProgram.getDataTypeManager();DataType callback=dtm.getDataType("/stdlib.h/_onexit_t");
  if(!(callback instanceof TypeDef)||callback.getLength()!=4)throw new Exception("Expected existing stdlib callback typedef");
  DataType base=((TypeDef)callback).getBaseDataType();if(!(base instanceof Pointer))throw new Exception("Expected callback pointer");
  DataType target=((Pointer)base).getDataType();if(!(target instanceof FunctionDefinition)||!((FunctionDefinition)target).getReturnType().getName().equals("int")||((FunctionDefinition)target).getArguments().length!=0)throw new Exception("Unexpected onexit callback shape");
  DataType existing=dtm.getDataType("/FSX/CRT/CrtAtexitCallback");if(existing!=null&&(!(existing instanceof FunctionDefinition)||existing.getLength()!=-1||((FunctionDefinition)existing).getArguments().length!=0||!(((FunctionDefinition)existing).getReturnType() instanceof VoidDataType)))throw new Exception("Preserve conflicting existing callback definition");
  println("CHECKED stdlib _onexit_t int(void) pointer and distinct atexit void(void) model");
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only preflight");return;}
  int tx=currentProgram.startTransaction("Apply checked CRT callback datatypes without textual parser");boolean ok=false;
  try{
   DataType voidCallback=existing;
   if(voidCallback==null){FunctionDefinitionDataType definition=new FunctionDefinitionDataType(new CategoryPath("/FSX/CRT"),"CrtAtexitCallback",dtm);definition.setReturnType(VoidDataType.dataType);definition.setArguments(new ParameterDefinition[0]);definition.setCallingConvention("__cdecl");voidCallback=dtm.resolve(definition,DataTypeConflictHandler.DEFAULT_HANDLER);}
   DataType voidPointer=dtm.resolve(new PointerDataType(voidCallback,4,dtm),DataTypeConflictHandler.DEFAULT_HANDLER);
   onexit.setCallingConvention("__cdecl");onexit.setReturnType(callback,SourceType.USER_DEFINED);onexit.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,new ParameterImpl("callback",callback,currentProgram));
   atexit.setCallingConvention("__cdecl");atexit.setReturnType(IntegerDataType.dataType,SourceType.USER_DEFINED);atexit.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,new ParameterImpl("callback",voidPointer,currentProgram));
   println(onexit.getSignature().toString());println(atexit.getSignature().toString());ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
  println("Applied datatype-backed callback signatures; existing standard typedef preserved. Save separately.");
 }
}
