import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
public class TypeApiPathInterfaces extends GhidraScript {
 DataType integer=IntegerDataType.dataType,unsigned=UnsignedIntegerDataType.dataType;
 DataType text=new PointerDataType(CharDataType.dataType);
 Parameter p(String name,DataType type,String register,int stack)throws Exception{
  VariableStorage storage=register==null?new VariableStorage(currentProgram,stack,4):new VariableStorage(currentProgram,currentProgram.getRegister(register));
  return new ParameterImpl(name,type,storage,currentProgram);
 }
 void type(Function f,int purge,DataType result,Parameter...args)throws Exception{
  f.updateFunction("__stdcall",new ReturnParameterImpl(result,new VariableStorage(currentProgram,currentProgram.getRegister("EAX")),currentProgram),Function.FunctionUpdateType.CUSTOM_STORAGE,true,SourceType.USER_DEFINED,args);
  f.setStackPurgeSize(purge);
  if(f.getParameterCount()!=args.length||!f.hasCustomVariableStorage())throw new Exception("Storage update failed");
  for(int j=0;j<args.length;j++)if(!f.getParameter(j).getVariableStorage().equals(args[j].getVariableStorage()))throw new Exception("Storage mismatch");
  f.setComment("Reconstructed path interface, assembly verified in api.dll. Explicit custom storage: __stdcall models cleanup only; register parameters are not stack arguments. EAX return. Names preserved; no original-source ABI/name claim. See notes/re/api.dll/api-initialization-flow.md (resolver/selector contract). "+(purge==16?"Output/capacity/input/selector are four stack words; returns0/-1.":"Preserve observed register and stack locations; status/attribute semantics documented in topic."));
 }
 public void run()throws Exception{
  if(!currentProgram.getName().equals("api.dll"))throw new Exception("Wrong program");
  long[] va={0x20133d17L,0x20134fc4L,0x200f79d5L,0x201203f4L,0x2013341bL,0x20132c8eL,0x2014507fL};int[] size={258,182,86,78,202,49,145};Function[] f=new Function[7];
  for(int j=0;j<7;j++){f[j]=getFunctionAt(toAddr(va[j]));if(f[j]==null||!f[j].getName().equals("FUN_"+Long.toHexString(va[j]))||f[j].getBody().getNumAddresses()!=size[j]||f[j].getParameterCount()!=0)throw new Exception("Unexpected target "+Long.toHexString(va[j]));}
  int tx=currentProgram.startTransaction("Verified API path argument storage");boolean ok=false;try{
   type(f[0],16,integer,p("outputPath",text,null,4),p("capacity",unsigned,null,8),p("inputPath",text,null,12),p("suffixSelector",integer,null,16));
   type(f[1],4,integer,p("suffixSelector",integer,"EAX",0),p("inputPath",text,"ECX",0),p("outputPath",text,null,4));
   type(f[2],4,integer,p("source",text,"EAX",0),p("destination",text,"EDX",0),p("capacity",unsigned,null,4));
   type(f[3],4,integer,p("destination",text,"EDI",0),p("capacity",unsigned,"ECX",0),p("suffix",text,null,4));
   type(f[4],12,integer,p("outputAttributes",new PointerDataType(unsigned),null,4),p("inputPath",text,null,8),p("suffixSelector",integer,null,12));
   type(f[5],8,integer,p("inputPath",text,null,4),p("suffixSelector",integer,null,8));
   type(f[6],4,unsigned,p("inputPath",text,null,4));
   for(Function fun:f){println(fun.getEntryPoint()+" "+fun.getPrototypeString(true,true)+" purge="+fun.getStackPurgeSize());for(Parameter a:fun.getParameters())println(a.getName()+"="+a.getVariableStorage());}ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
 }
}
