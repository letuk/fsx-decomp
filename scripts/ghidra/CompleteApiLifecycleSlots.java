import ghidra.app.script.GhidraScript;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.program.model.address.*;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;

// Scoped model corrections, not binary patching or complete class recovery.
public class CompleteApiLifecycleSlots extends GhidraScript {
    private FunctionDefinition definition(String name, DataType result, ParameterDefinition[] args, String cc) throws Exception {
        CategoryPath category = new CategoryPath("/FSXReconstruction");
        if (currentProgram.getDataTypeManager().getDataType(category, name) != null)
            throw new Exception("Type already exists: " + name);
        FunctionDefinitionDataType d = new FunctionDefinitionDataType(category, name, currentProgram.getDataTypeManager());
        d.setReturnType(result); d.setArguments(args); d.setCallingConvention(cc);
        return (FunctionDefinition) currentProgram.getDataTypeManager().addDataType(d, DataTypeConflictHandler.KEEP_HANDLER);
    }
    public void run() throws Exception {
        if (!currentProgram.getName().equals("api.dll")) throw new Exception("Wrong program");
        DataTypeManager dtm = currentProgram.getDataTypeManager();
        Structure table = (Structure) dtm.getDataType("/ApplicationVTable");
        DataType app = dtm.getDataType("/Application"), msg = dtm.getDataType("/winuser.h/tagMSG");
        if (table == null || table.getLength() != 16 || app == null || msg == null || msg.getLength() != 28)
            throw new Exception("Unexpected existing layouts");
        long[] targets = {0x2012048aL,0x2012052aL,0x20122817L,0x20120535L};
        for(int i=0;i<4;i++) if (Integer.toUnsignedLong(currentProgram.getMemory().getInt(toAddr(0x200f2b24L+4*i))) != targets[i]) throw new Exception("VTable target differs");
        byte[] expected={(byte)0x68,(byte)0xd5,0x0e,0x12,0x20,(byte)0xe8,(byte)0x8d,(byte)0xde,0x01,0x00,(byte)0xc3};
        byte[] actual=new byte[11];currentProgram.getMemory().getBytes(toAddr(targets[1]),actual);
        if(!java.util.Arrays.equals(expected,actual))throw new Exception("Slot04 bytes differ");
        FunctionManager fm=currentProgram.getFunctionManager();
        Function init=fm.getFunctionAt(toAddr(targets[0])), loop=fm.getFunctionAt(toAddr(targets[2]));
        if(init==null||loop==null||loop.getBody().getNumAddresses()!=29||!loop.getName().equals("FUN_20122817")||fm.getFunctionAt(toAddr(targets[1]))!=null)
            throw new Exception("Unexpected stored functions; do not overwrite");
        FunctionDefinition initDef=(FunctionDefinition)((Pointer)table.getComponentAt(0).getDataType()).getDataType();
        FunctionDefinition windowDef=(FunctionDefinition)((Pointer)table.getComponentAt(12).getDataType()).getDataType();
        if(initDef.getArguments().length!=6||windowDef.getArguments().length!=2||!init.getParameter(2).getName().equals("startupArgument"))throw new Exception("Prototype changed");
        int tx=currentProgram.startTransaction("Verified API lifecycle boundary slots");boolean ok=false;
        try {
            DataType appPointer=new PointerDataType(app);
            FunctionDefinition callback=definition("CurrentMessageCallback",new PointerDataType(msg),new ParameterDefinition[0],"__stdcall");
            DataType callbackPointer=new PointerDataType(callback);
            FunctionDefinition shutdownDef=definition("ApplicationShutdown",IntegerDataType.dataType,new ParameterDefinition[]{new ParameterDefinitionImpl("this",appPointer,null)},"__thiscall");
            FunctionDefinition loopDef=definition("ApplicationRunMessageLoop",IntegerDataType.dataType,new ParameterDefinition[]{new ParameterDefinitionImpl("this",appPointer,null),new ParameterDefinitionImpl("getCurrentMessage",callbackPointer,null)},"__thiscall");
            AddressSet body=new AddressSet(toAddr(targets[1]),toAddr(targets[1]+10));
            AddressIterator addresses=body.getAddresses(true);while(addresses.hasNext()){Address a=addresses.next();if(currentProgram.getListing().getDefinedDataContaining(a)!=null||fm.getFunctionContaining(a)!=null)throw new Exception("Slot04 overlap");}
            DisassembleCommand cmd=new DisassembleCommand(toAddr(targets[1]),body,true);
            if(!cmd.applyTo(currentProgram,monitor))throw new Exception(cmd.getStatusMsg());
            Function shutdown=fm.createFunction("Shutdown",init.getParentNamespace(),toAddr(targets[1]),body,SourceType.USER_DEFINED);
            shutdown.setCallingConvention("__thiscall");shutdown.setReturnType(IntegerDataType.dataType,SourceType.USER_DEFINED);
            shutdown.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,new Parameter[0]);
            loop.setParentNamespace(init.getParentNamespace());loop.setName("RunMessageLoop",SourceType.USER_DEFINED);
            loop.setCallingConvention("__thiscall");loop.setReturnType(IntegerDataType.dataType,SourceType.USER_DEFINED);
            loop.replaceParameters(Function.FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS,true,SourceType.USER_DEFINED,new ParameterImpl("getCurrentMessage",callbackPointer,currentProgram));
            table.replaceAtOffset(4,new PointerDataType(shutdownDef),4,"Shutdown","Reconstructed slot; normal result forwards shutdown callback EAX.");
            table.replaceAtOffset(8,new PointerDataType(loopDef),4,"RunMessageLoop","Reconstructed callback contract verified across EXE and DLL.");
            ParameterDefinition[] ia=java.util.Arrays.copyOf(initDef.getArguments(),initDef.getArguments().length,ParameterDefinition[].class);ia[0]=new ParameterDefinitionImpl("this",appPointer,ia[0].getComment());ia[2]=new ParameterDefinitionImpl("startupArgument",UnsignedIntegerDataType.dataType,"Neutral 32-bit word; caller supplies SDK-backed m_nCmdShow.");initDef.setArguments(ia);
            ParameterDefinition[] wa=java.util.Arrays.copyOf(windowDef.getArguments(),windowDef.getArguments().length,ParameterDefinition[].class);wa[0]=new ParameterDefinitionImpl("this",appPointer,wa[0].getComment());windowDef.setArguments(wa);
            init.getParameter(2).setDataType(UnsignedIntegerDataType.dataType,SourceType.USER_DEFINED);
            shutdown.setComment("Reconstructed Application slot+4. Full 11-byte body verified against native COFF relocations. Invokes shutdown implementation20120ed5 through SEH wrapper2013e3c1. Returned EAX is ignored by the known EXE caller; source return type is a reconstruction model, not original declaration. Dependencies not rebuilt here.");
            loop.setComment("Reconstructed Application slot+8. Full29-byte body verified against native COFF relocations. Stores WINAPI MSG* no-argument callback at2020de7c, invokes loop20122ae1 via the SEH wrapper, then returns0/RET4. Loop/dependency implementation not certified by this outer-body comparison.");
            println(shutdown.getPrototypeString(true,true));println(loop.getPrototypeString(true,true));println(init.getPrototypeString(true,true));
            ok=true;
        } finally {currentProgram.endTransaction(tx,ok);}
    }
}
