import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
public class TypeApiFontEnumeration extends GhidraScript {
 void type(Function f,int purge,DataType result,Parameter...args)throws Exception{f.setCallingConvention("__stdcall");f.setReturnType(result,SourceType.USER_DEFINED);f.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,args);f.setStackPurgeSize(purge);}
 public void run()throws Exception{
  if(!currentProgram.getName().equals("api.dll"))throw new Exception("Wrong program");
  long[] va={0x20121083L,0x20134495L,0x2013468cL,0x20134929L,0x201350beL};int[] sizes={733,503,669,92,86};Function[] fs=new Function[5];
  for(int j=0;j<5;j++){fs[j]=getFunctionAt(toAddr(va[j]));if(fs[j]==null||!fs[j].getName().equals("FUN_"+Long.toHexString(va[j]))||fs[j].getParameterCount()!=0||fs[j].getBody().getNumAddresses()!=sizes[j])throw new Exception("Unexpected function");}
  DataTypeManager manager=currentProgram.getDataTypeManager();CategoryPath path=new CategoryPath("/RE/Api");
  if(manager.getDataType(path,"FileEnumerationStateObserved332")!=null||manager.getDataType(path,"FileEnumerationRecordObserved300")!=null)throw new Exception("Types already exist; inspect");
  DataType find=manager.getDataType("/winbase.h/_WIN32_FIND_DATAA"),handle=manager.getDataType("/winnt.h/HANDLE");if(find==null||find.getLength()!=320||handle==null||handle.getLength()!=4)throw new Exception("Wrong platform layout");
  DataType integer=IntegerDataType.dataType,unsigned=UnsignedIntegerDataType.dataType,text=new PointerDataType(CharDataType.dataType);
  int tx=currentProgram.startTransaction("Observed file enumeration and font interfaces");boolean ok=false;try{
   StructureDataType state=new StructureDataType(path,"FileEnumerationStateObserved332",0);state.add(handle,"handleOrFallback",null);state.add(find,"current",null);state.add(integer,"suffixSelector",null);state.add(unsigned,"flags",null);state.setDescription("Reconstructed observed allocation332: native or fallback enumeration handle, WIN32 record, selector and flags. Original type name/full surrounding object unproven.");
   if(state.getLength()!=332||state.getComponent(3).getOffset()!=0x148)throw new Exception("State layout mismatch");DataType savedState=manager.addDataType(state,DataTypeConflictHandler.DEFAULT_HANDLER);
   StructureDataType record=new StructureDataType(path,"FileEnumerationRecordObserved300",0);record.add(new ArrayDataType(CharDataType.dataType,260,1),"name",null);record.add(integer,"suffixSelector",null);record.add(unsigned,"sizeLow",null);record.add(unsigned,"sizeHigh",null);record.add(unsigned,"writable",null);record.add(new ArrayDataType(UnsignedShortDataType.dataType,6,2),"localTimestamp6",null);record.add(new ArrayDataType(UnsignedShortDataType.dataType,6,2),"utcTimestamp6",null);record.setDescription("Observed output fields through byte12bh. Timestamp arrays are year/month/day/hour/minute/second, not full SYSTEMTIME. Directory marker-2; directory writable field not proven initialized.");
   if(record.getLength()!=300||record.getComponent(5).getOffset()!=0x114||record.getComponent(6).getOffset()!=0x120)throw new Exception("Record layout mismatch");DataType savedRecord=manager.addDataType(record,DataTypeConflictHandler.DEFAULT_HANDLER);
   DataType pState=new PointerDataType(savedState),ppState=new PointerDataType(pState),pRecord=new PointerDataType(savedRecord);
   type(fs[0],4,VoidDataType.dataType,new ParameterImpl("addFonts",integer,currentProgram));
   type(fs[1],16,integer,new ParameterImpl("outputEnumeration",ppState,currentProgram),new ParameterImpl("directory",text,currentProgram),new ParameterImpl("mask",text,currentProgram),new ParameterImpl("suffixSelector",integer,currentProgram));
   type(fs[2],8,integer,new ParameterImpl("enumeration",pState,currentProgram),new ParameterImpl("outputRecord",pRecord,currentProgram));
   type(fs[3],4,integer,new ParameterImpl("inOutEnumeration",ppState,currentProgram));
   type(fs[4],12,integer,new ParameterImpl("suffixSelector",integer,currentProgram),new ParameterImpl("output",text,currentProgram),new ParameterImpl("capacity",unsigned,currentProgram));
   fs[0].setComment("Reconstructed add/remove private font lifecycle. Enumerates fonts with selector-1, skips directories marker-2, constructs paths and appends .ttf to stripped stems. Resolves AddFontResourceExA/RemoveFontResourceExA and uses flags10h, otherwise legacy GDI operation. Source891 versus original733, unmatched; no runtime execution. See api-font-resources.md.");
   fs[1].setComment("Reconstructed file enumeration opener. Allocates14ch from private heap, observed state fields current4/selector144h/flags148h; native FindFirstFileA then alternate backend20145110. Backend requires additional register inputs, not conventional signature inferred from decompilation. No native opener body match. See api-font-resources.md.");
   fs[2].setComment("Reconstructed iterator:0 record,-1 exhausted; output name stem, selector or directory marker-2, size/writable and local/UTC six-WORD timestamps. Advances native/alternate backend. Directory writable field not proven written. Source not reconstructed as simple FindNextFile wrapper. See api-font-resources.md.");
   fs[3].setComment("Reconstructed enumeration close, full92-byte COFF match with4 mapped fields. Native handle FindClose versus alternate memory HeapFree; then frees outer state and clears caller pointer. No leading NULL pointer check; preserve observed ordering. See api-font-resources.md.");
   fs[4].setComment("Reconstructed selected-suffix copy wrapper, stdcall3/RET12. Built-in/dynamic table and-2 empty suffix; capacity<=7fffffff calls custom copy200f79d5. Always returns0 in observed body even when skipping/copy fails; caller's-1 test does not prove a callee-1 path. See api-font-resources.md.");
   for(Function f:fs){println(f.getPrototypeString(true,true));for(Parameter p:f.getParameters())println(p.getName()+"="+p.getVariableStorage());}println(savedState.getPathName()+" size="+savedState.getLength());println(savedRecord.getPathName()+" size="+savedRecord.getLength());ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
 }
}
