import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
public class TypeApiMonitorGeometry extends GhidraScript {
 void standard(Function f,int purge,DataType result,Parameter...args)throws Exception{
  f.setCallingConvention("__stdcall");f.setReturnType(result,SourceType.USER_DEFINED);
  f.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,args);f.setStackPurgeSize(purge);
 }
 public void run()throws Exception{
  if(!currentProgram.getName().equals("api.dll"))throw new Exception("Wrong program");
  long[] va={0x201214adL,0x201215fdL,0x20121796L,0x201213c4L};int[] sizes={336,409,585,233};Function[] fs=new Function[4];
  for(int j=0;j<4;j++){fs[j]=getFunctionAt(toAddr(va[j]));if(fs[j]==null||!fs[j].getName().equals("FUN_"+Long.toHexString(va[j]))||fs[j].getBody().getNumAddresses()!=sizes[j]||fs[j].getParameterCount()!=0)throw new Exception("Unexpected function");}
  DataTypeManager manager=currentProgram.getDataTypeManager();CategoryPath path=new CategoryPath("/RE/Api");
  if(manager.getDataType(path,"MonitorInfoExAObserved72")!=null||manager.getDataType(path,"MonitorSelectionContextObserved184")!=null)throw new Exception("Type already exists; inspect first");
  DataType rect=manager.getDataType("/WinDef.h/tagRECT"),monitor=manager.getDataType("/WinDef.h/HMONITOR"),dc=manager.getDataType("/WinDef.h/HDC");
  if(rect==null||rect.getLength()!=16||monitor==null||dc==null)throw new Exception("Missing platform types");
  DataType unsigned=UnsignedIntegerDataType.dataType,integer=IntegerDataType.dataType;
  int tx=currentProgram.startTransaction("Observed monitor context and geometry interfaces");boolean ok=false;try{
   StructureDataType info=new StructureDataType(path,"MonitorInfoExAObserved72",0);info.add(unsigned,"cbSize",null);info.add(rect,"rcMonitor",null);info.add(rect,"rcWork",null);info.add(unsigned,"dwFlags",null);info.add(new ArrayDataType(CharDataType.dataType,32,1),"szDevice",null);
   info.setDescription("Reconstructed mirror of observed72-byte MONITORINFOEXA layout; native SDK sizeof and offsets verified. Original source type name not claimed.");
   if(info.getLength()!=72||info.getComponent(4).getOffset()!=40)throw new Exception("Info layout mismatch");DataType savedInfo=manager.addDataType(info,DataTypeConflictHandler.DEFAULT_HANDLER);
   StructureDataType context=new StructureDataType(path,"MonitorSelectionContextObserved184",0);context.add(new ArrayDataType(CharDataType.dataType,32,1),"requestedDevice",null);context.add(unsigned,"primarySeen",null);context.add(unsigned,"colorDepthState",null);context.add(savedInfo,"fallback",null);context.add(savedInfo,"selected",null);
   context.setDescription("Observed enumeration context only: device32, flags/state, two72-byte records; no original class/name/layout outside184 bytes claim.");
   if(context.getLength()!=184||context.getComponent(3).getOffset()!=0x28||context.getComponent(4).getOffset()!=0x70)throw new Exception("Context layout mismatch");DataType savedContext=manager.addDataType(context,DataTypeConflictHandler.DEFAULT_HANDLER);
   DataType pInfo=new PointerDataType(savedInfo),pRect=new PointerDataType(rect);
   standard(fs[0],16,integer,new ParameterImpl("monitor",monitor,currentProgram),new ParameterImpl("deviceContext",dc,currentProgram),new ParameterImpl("monitorRect",pRect,currentProgram),new ParameterImpl("selection",new PointerDataType(savedContext),currentProgram));
   standard(fs[1],8,integer,new ParameterImpl("requested",pInfo,currentProgram),new ParameterImpl("fallback",pInfo,currentProgram));
   fs[2].updateFunction("__stdcall",new ReturnParameterImpl(integer,new VariableStorage(currentProgram,currentProgram.getRegister("EAX")),currentProgram),Function.FunctionUpdateType.CUSTOM_STORAGE,true,SourceType.USER_DEFINED,new ParameterImpl("monitor",monitor,new VariableStorage(currentProgram,currentProgram.getRegister("ECX")),currentProgram),new ParameterImpl("outputBounds",pRect,new VariableStorage(currentProgram,4,4),currentProgram));fs[2].setStackPurgeSize(4);
   standard(fs[3],4,unsigned,new ParameterImpl("geometryStatus",unsigned,currentProgram));
   fs[0].setComment("Reconstructed monitor enumeration callback. BOOL stdcall4. Uses72-byte monitor info incl device name, context184 offsets20/24/28/70. Rejects24-bit color while checking mixed display formats; selects eligible1024x720 records. Source309 versus original336, unmatched. See api-window-geometry.md.");
   fs[1].setComment("Reconstructed monitor selector stdcall2: requested and fallback72-byte records. Status-1 DC failure, b0001 color-depth rejection, b0002 no eligible monitor,0 success. Native407 versus original409, unmatched. See api-window-geometry.md.");
   fs[2].setComment("Reconstructed geometry resolver. CUSTOM ABI: HMONITOR in ECX, RECT* at stack4, RET4, EAX status; __stdcall models cleanup only. Reads MAIN/Location and monitor device name using sscanf_s, selects monitor, restores or centers bounds then AdjustWindowRect. Source uses explicit ordinary-stack boundary and is not ABI-equivalent. See api-window-geometry.md.");
   fs[3].setComment("Reconstructed geometry error response stdcall1. b0001/b0002 resource strings; callback result6 opens display settings through20123315. Observed body returns1 or E_FAIL, with no zero-return path even though its caller has a zero-response retry branch. Formatter20120210 uses EDX capacity and cdecl stack varargs; source boundary remains unresolved. See api-window-geometry.md.");
   if(!fs[2].getParameter(0).getVariableStorage().toString().equals("ECX:4")||!fs[2].getParameter(1).getVariableStorage().toString().equals("Stack[0x4]:4"))throw new Exception("Wrong custom geometry storage");
   println(savedInfo.getPathName()+" size="+savedInfo.getLength());println(savedContext.getPathName()+" size="+savedContext.getLength());for(Function f:fs){println(f.getPrototypeString(true,true));for(Parameter p:f.getParameters())println(p.getName()+"="+p.getVariableStorage());}ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
 }
}
