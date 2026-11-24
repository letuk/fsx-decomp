import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
// Dataflow/byte-verified types only; no original class-name/full-layout claim.
public class TypeApiFdiAndRegistry extends GhidraScript {
    public void run() throws Exception {
        if(!currentProgram.getName().equals("api.dll"))throw new Exception("Wrong program");
        long[] addresses={0x2014474cL,0x2014476dL,0x2014478eL,0x201447a5L,0x200f6451L};
        int[] sizes={33,33,23,51,318};
        Function[] functions=new Function[5];
        for(int i=0;i<5;i++){functions[i]=currentProgram.getFunctionManager().getFunctionAt(toAddr(addresses[i]));if(functions[i]==null||functions[i].getBody().getNumAddresses()!=sizes[i]||!functions[i].getName().startsWith("FUN_"))throw new Exception("Unexpected function state");}
        byte[] start=new byte[8];currentProgram.getMemory().getBytes(toAddr(addresses[4]),start);
        byte[] expected={(byte)0x8b,(byte)0xff,0x55,(byte)0x8b,(byte)0xec,(byte)0x85,(byte)0xf6,0x75};
        if(!java.util.Arrays.equals(start,expected))throw new Exception("ESI receiver entry differs");
        DataType integer=IntegerDataType.dataType, unsigned=UnsignedIntegerDataType.dataType;
        DataType pointer=new PointerDataType(VoidDataType.dataType);
        String[] names={"ReadFdiStream","WriteFdiStream","CloseFdiStream","SeekFdiStream"};
        int tx=currentProgram.startTransaction("Verified FDI adapters and ESI callback-list receiver");boolean ok=false;
        try {
            for(int i=0;i<4;i++){
                Function f=functions[i];f.setName(names[i],SourceType.USER_DEFINED);f.setCallingConvention("__cdecl");
                f.setReturnType(i<2?unsigned:i==2?integer:LongDataType.dataType,SourceType.USER_DEFINED);
                Parameter[] parameters=i<2?new Parameter[]{new ParameterImpl("streamHandle",integer,currentProgram),new ParameterImpl("buffer",pointer,currentProgram),new ParameterImpl("count",unsigned,currentProgram)}:i==2?new Parameter[]{new ParameterImpl("streamHandle",integer,currentProgram)}:new Parameter[]{new ParameterImpl("streamHandle",integer,currentProgram),new ParameterImpl("distance",LongDataType.dataType,currentProgram),new ParameterImpl("origin",integer,currentProgram)};
                f.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,parameters);
                f.setComment("Reconstructed FDI adapter with complete raw native COFF body equality. Handle is a32-bit IStream-ABI-compatible pointer value, not proof of concrete class/COM IID. "+(i<2?"Read/Write ignores HRESULT and returns the count output word.":i==2?"Invokes vtable Release slot+8 and returns1.":"Invokes Seek slot+0x14 using low distance/high0 and the same64-bit local as output. No sign-extension assumed.")+" See notes/re/api.dll/api-initialize-fdi.md.");
            }
            Function registry=functions[4];registry.setName("RegisterCallbackInList",SourceType.USER_DEFINED);
            VariableStorage eax=new VariableStorage(currentProgram,currentProgram.getRegister("EAX"));
            VariableStorage esi=new VariableStorage(currentProgram,currentProgram.getRegister("ESI"));
            Parameter[] args={new ParameterImpl("registryOwner",new PointerDataType(pointer),esi,currentProgram),new ParameterImpl("callback",pointer,new VariableStorage(currentProgram,4,4),currentProgram),new ParameterImpl("withContext",integer,new VariableStorage(currentProgram,8,4),currentProgram),new ParameterImpl("context1",unsigned,new VariableStorage(currentProgram,12,4),currentProgram),new ParameterImpl("context2",unsigned,new VariableStorage(currentProgram,16,4),currentProgram)};
            registry.updateFunction("__stdcall",new ReturnParameterImpl(integer,eax,currentProgram),Function.FunctionUpdateType.CUSTOM_STORAGE,true,SourceType.USER_DEFINED,args);
            registry.setStackPurgeSize(16);
            registry.setComment("Reconstructed optimized ABI: registryOwner in ESI, four explicit stack words, int result in EAX, RET0x10. Custom storage is essential; __stdcall is only the Ghidra stack-cleanup model, not a conventional five-stack-argument ABI. Appends callback/flags/context to a dynamic list; observed header16 and entry16, initial allocation0xb0 for10 entries. Original class/free-function/source convention unknown. No reconstructed-helper match claimed. Native source's ECX thiscall receiver differs; preserve this residual. See notes/re/api.dll/api-initialize-fdi.md.");
            for(Function f:functions)println(f.getPrototypeString(true,true));ok=true;
        }finally{currentProgram.endTransaction(tx,ok);}
    }
}
