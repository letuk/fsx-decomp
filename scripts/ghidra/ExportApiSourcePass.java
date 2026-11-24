// Read-only evidence export. Decompiler output is NOT compilable reconstruction.
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.nio.file.*;
import java.util.*;
import com.google.gson.*;

public class ExportApiSourcePass extends GhidraScript {
    private final Gson gson = new GsonBuilder().setPrettyPrinting().create();
    private void write(File f, String s) throws Exception {
        Files.write(f.toPath(), s.getBytes("UTF-8"));
    }
    public void run() throws Exception {
        if (!currentProgram.getName().equals("api.dll")) throw new Exception("api.dll required");
        File root = new File(getScriptArgs()[0]);
        if (!root.isDirectory()) throw new Exception("Existing output directory required");
        File funcs = new File(root,"functions"); funcs.mkdirs();
        DecompInterface dec = new DecompInterface();
        dec.openProgram(currentProgram);
        List<Object> rows = new ArrayList<>();
        int success=0, failures=0;
        try {
            FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
            while(it.hasNext()) {
                monitor.checkCancelled();
                Function f=it.next(); String entry=f.getEntryPoint().toString();
                Map<String,Object> row=new LinkedHashMap<>();
                row.put("entry",entry); row.put("name",f.getName(true));
                row.put("signature",f.getSignature().toString());
                row.put("convention",f.getCallingConventionName());
                row.put("customStorage",f.hasCustomVariableStorage());
                row.put("thunk",f.isThunk()); row.put("bodyBytes",f.getBody().getNumAddresses());
                List<Object> ranges=new ArrayList<>();
                for(AddressRange r:f.getBody().getAddressRanges()) ranges.add(Arrays.asList(r.getMinAddress().toString(),r.getMaxAddress().toString()));
                row.put("ranges",ranges);
                List<Object> params=new ArrayList<>();
                for(Parameter p:f.getParameters()) {
                    Map<String,Object> pr=new LinkedHashMap<>(); pr.put("name",p.getName());
                    pr.put("type",p.getDataType().getPathName());pr.put("storage",p.getVariableStorage().toString());params.add(pr);
                }
                row.put("parameters",params);
                StringBuilder asm=new StringBuilder(); Set<String> targets=new TreeSet<>();
                InstructionIterator ins=currentProgram.getListing().getInstructions(f.getBody(),true);
                while(ins.hasNext()) {
                    Instruction i=ins.next();asm.append(i.getAddress()).append("  ");
                    for(byte b:i.getBytes()) asm.append(String.format("%02x",b&255));
                    asm.append("  ").append(i.toString()).append("\n");
                    for(Reference ref:i.getReferencesFrom()) if(!ref.getReferenceType().isFlow() || ref.getReferenceType().isCall()) targets.add(ref.getToAddress().toString());
                }
                row.put("references",targets);
                write(new File(funcs,entry+".asm"),asm.toString());
                DecompileResults result=dec.decompileFunction(f,20,monitor);
                boolean ok=result.decompileCompleted() && result.getDecompiledFunction()!=null;
                row.put("decompiled",ok); row.put("diagnostic",result.getErrorMessage());
                if(ok) {write(new File(funcs,entry+".c"),result.getDecompiledFunction().getC());success++;}
                else failures++;
                write(new File(funcs,entry+".json"),gson.toJson(row));rows.add(row);
                if(rows.size()%50==0) write(new File(root,"progress.json"),"{\"processed\":"+rows.size()+",\"decompiled\":"+success+",\"failed\":"+failures+"}");
            }
            Map<String,Object> manifest=new LinkedHashMap<>();
            manifest.put("program",currentProgram.getName());manifest.put("executableSHA256",currentProgram.getExecutableSHA256());
            manifest.put("language",currentProgram.getLanguageID().toString());
            manifest.put("scope","Live model bodies, raw decompilation and listing; not full reachable-flow, compilability or matching proof");
            manifest.put("functions",rows);manifest.put("decompiled",success);manifest.put("failed",failures);
            write(new File(root,"manifest.json"),gson.toJson(manifest));
            println("Exported "+rows.size()+" functions; decompiled="+success+" failed="+failures+"; database unchanged");
        } finally {dec.dispose();}
    }
}
