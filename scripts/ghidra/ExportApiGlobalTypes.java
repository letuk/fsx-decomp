// Read-only inferred decompiler global types. Not authoritative original declarations.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.pcode.*;
import java.io.*;
import java.util.*;
import com.google.gson.*;
public class ExportApiGlobalTypes extends GhidraScript {
    public void run() throws Exception {
        if(!currentProgram.getName().equals("api.dll"))throw new Exception("api.dll required");
        Map<String,Object> rows=new LinkedHashMap<>();
        DecompInterface dec=new DecompInterface();dec.openProgram(currentProgram);
        try {
            FunctionIterator it=currentProgram.getFunctionManager().getFunctions(true);
            while(it.hasNext()) {
                Function f=it.next();monitor.checkCancelled();
                DecompileResults r=dec.decompileFunction(f,20,monitor);
                List<Object> globals=new ArrayList<>();
                if(r.decompileCompleted()&&r.getHighFunction()!=null){
                    Iterator<HighSymbol> symbols=r.getHighFunction().getGlobalSymbolMap().getSymbols();
                    while(symbols.hasNext()){
                        HighSymbol s=symbols.next();Map<String,Object> v=new LinkedHashMap<>();
                        v.put("name",s.getName());v.put("type",s.getDataType().getDisplayName());
                        v.put("size",s.getDataType().getLength());v.put("address",s.getStorage().getMinAddress().toString());
                        globals.add(v);
                    }
                }
                rows.put(f.getEntryPoint().toString(),globals);
            }
            try(Writer w=new OutputStreamWriter(new FileOutputStream(getScriptArgs()[0]),"UTF-8")){
                new GsonBuilder().setPrettyPrinting().create().toJson(rows,w);
            }
            println("Exported inferred global symbols for "+rows.size()+" functions; database unchanged");
        }finally{dec.dispose();}
    }
}
