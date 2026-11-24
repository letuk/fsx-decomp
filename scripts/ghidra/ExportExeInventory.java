// Read-only export; pass an output JSON path as the single script argument.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import java.util.*;
import java.io.*;
import com.google.gson.GsonBuilder;

public class ExportExeInventory extends GhidraScript {
    public void run() throws Exception {
        if (getScriptArgs().length != 1) throw new IllegalArgumentException("Pass one output JSON path");
        Map<String,Object> root = new LinkedHashMap<>();
        root.put("program", currentProgram.getName());
        root.put("imageBase", currentProgram.getImageBase().toString());
        root.put("scope", "Read-only model/listing export; unassigned instructions do not certify function boundaries");
        List<Object> functions = new ArrayList<>();
        FunctionIterator iterator = currentProgram.getFunctionManager().getFunctions(true);
        while (iterator.hasNext()) {
            Function function = iterator.next();
            Map<String,Object> row = new LinkedHashMap<>();
            row.put("entry", function.getEntryPoint().toString());
            row.put("name", function.getName(true));
            row.put("bodyBytes", function.getBody().getNumAddresses());
            row.put("bodyEnd", function.getBody().getMaxAddress().toString());
            row.put("ranges", function.getBody().getNumAddressRanges());
            row.put("isThunk", function.isThunk());
            if (function.isThunk()) {
                Function target = function.getThunkedFunction(true);
                row.put("thunkTarget", target == null ? null : target.getName(true));
            }
            List<Object> instructions = new ArrayList<>();
            InstructionIterator code = currentProgram.getListing().getInstructions(function.getBody(), true);
            while (code.hasNext()) instructions.add(instruction(code.next()));
            row.put("instructions", instructions);
            functions.add(row);
        }
        root.put("functions", functions);
        int external = 0;
        iterator = currentProgram.getFunctionManager().getExternalFunctions();
        while (iterator.hasNext()) { iterator.next(); external++; }
        root.put("externalFunctionCount", external);
        root.put("modelFunctionCount", currentProgram.getFunctionManager().getFunctionCount());
        List<Object> blocks = new ArrayList<>(), loose = new ArrayList<>(), undefined = new ArrayList<>(), data = new ArrayList<>();
        for (MemoryBlock block : currentProgram.getMemory().getBlocks()) {
            if (!block.isExecute()) continue;
            Map<String,Object> row = new LinkedHashMap<>();
            row.put("name", block.getName()); row.put("start", block.getStart().toString());
            row.put("end", block.getEnd().toString()); row.put("size", block.getSize());
            long instructionBytes = 0, dataBytes = 0, undefinedBytes = 0;
            int unassigned = 0;
            Map<String,Object> undefinedRun = null;
            CodeUnitIterator units = currentProgram.getListing().getCodeUnits(block.getStart(), true);
            while (units.hasNext()) {
                CodeUnit unit = units.next();
                if (unit.getMinAddress().compareTo(block.getEnd()) > 0) break;
                if (unit instanceof Instruction) {
                    undefinedRun = null;
                    instructionBytes += unit.getLength();
                    if (currentProgram.getFunctionManager().getFunctionContaining(unit.getMinAddress()) == null) {
                        loose.add(instruction((Instruction)unit)); unassigned++;
                    }
                } else if (unit instanceof Data && ((Data)unit).isDefined()) {
                    undefinedRun = null;
                    dataBytes += unit.getLength();
                    Map<String,Object> item = new LinkedHashMap<>();
                    item.put("address", unit.getMinAddress().toString()); item.put("length", unit.getLength());
                    item.put("type", ((Data)unit).getDataType().getName()); data.add(item);
                } else {
                    undefinedBytes += unit.getLength();
                    if (undefinedRun == null) {
                        undefinedRun = new LinkedHashMap<>(); undefinedRun.put("address", unit.getMinAddress().toString());
                        undefinedRun.put("length", 0); undefined.add(undefinedRun);
                    }
                    undefinedRun.put("length", ((Integer)undefinedRun.get("length")) + unit.getLength());
                }
            }
            row.put("instructionBytes", instructionBytes); row.put("definedDataBytes", dataBytes);
            row.put("undefinedBytes", undefinedBytes); row.put("unassignedInstructions", unassigned);
            blocks.add(row);
        }
        root.put("executableBlocks", blocks); root.put("unassignedInstructions", loose);
        root.put("undefinedRanges", undefined); root.put("definedExecutableData", data);
        try (Writer output = new OutputStreamWriter(new FileOutputStream(getScriptArgs()[0]), "UTF-8")) {
            new GsonBuilder().setPrettyPrinting().create().toJson(root, output);
        }
        println("Exported " + functions.size() + " internal functions, " + external + " external declarations and " + loose.size() + " unassigned instructions; database unchanged.");
    }
    private Map<String,Object> instruction(Instruction value) {
        Map<String,Object> result = new LinkedHashMap<>();
        result.put("address", value.getAddress().toString()); result.put("text", value.toString());
        result.put("length", value.getLength()); return result;
    }
}
