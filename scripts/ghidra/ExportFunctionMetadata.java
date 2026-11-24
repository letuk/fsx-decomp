// Read-only Ghidra model export. Pass the program name and output JSON path.
import com.google.gson.GsonBuilder;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressRange;
import ghidra.program.model.address.AddressRangeIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.FunctionTag;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import java.io.Writer;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class ExportFunctionMetadata extends GhidraScript {
    public void run() throws Exception {
        String[] arguments = getScriptArgs();
        if (arguments.length != 2 || !currentProgram.getName().equals(arguments[0])) {
            throw new IllegalArgumentException("Expected the current program name and an output JSON path");
        }
        Path output = Paths.get(arguments[1]);
        FunctionManager manager = currentProgram.getFunctionManager();
        List<Map<String, Object>> functions = readFunctions(manager.getFunctions(true));
        List<Map<String, Object>> externalFunctions = readFunctions(manager.getExternalFunctions());
        int modelCount = manager.getFunctionCount();
        if (modelCount != functions.size() + externalFunctions.size()) {
            throw new IllegalStateException("Internal and external function counts do not reconcile");
        }

        Map<String, Object> export = new LinkedHashMap<>();
        export.put("program", currentProgram.getName());
        export.put("executablePath", currentProgram.getExecutablePath());
        export.put("imageBase", currentProgram.getImageBase().toString());
        export.put("modelFunctionCount", modelCount);
        export.put("functions", functions);
        export.put("externalFunctions", externalFunctions);
        export.put("executableBlocks", readInstructionCoverage(manager));
        try (Writer writer = Files.newBufferedWriter(output, StandardCharsets.UTF_8,
                StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
            new GsonBuilder().disableHtmlEscaping().setPrettyPrinting().create().toJson(export, writer);
        }
        println("Exported " + functions.size() + " internal functions and " +
            externalFunctions.size() + " external functions.");
    }

    private List<Map<String, Object>> readFunctions(FunctionIterator iterator) throws Exception {
        List<Map<String, Object>> functions = new ArrayList<>();
        while (iterator.hasNext()) {
            monitor.checkCancelled();
            Function function = iterator.next();
            Address entry = function.getEntryPoint();
            Map<String, Object> row = new LinkedHashMap<>();
            row.put("entry", entry.toString());
            row.put("name", function.getName());
            row.put("qualifiedName", function.getName(true));
            row.put("nameSource", function.getSymbol().getSource().toString());
            row.put("signature", function.getPrototypeString(false, true));
            row.put("callingConvention", function.getCallingConventionName());
            row.put("isThunk", function.isThunk());
            if (function.isThunk()) {
                Function target = function.getThunkedFunction(false);
                if (target != null) {
                    row.put("thunkTarget", target.getEntryPoint().toString());
                    row.put("thunkTargetName", target.getName(true));
                }
            }
            if (!function.isExternal()) {
                row.put("bodyBytes", function.getBody().getNumAddresses());
                row.put("bodyEnd", function.getBody().getMaxAddress().toString());
                List<List<String>> ranges = new ArrayList<>();
                AddressRangeIterator bodies = function.getBody().getAddressRanges();
                while (bodies.hasNext()) {
                    AddressRange range = bodies.next();
                    ranges.add(List.of(range.getMinAddress().toString(), range.getMaxAddress().toString()));
                }
                row.put("bodyRanges", ranges);
                row.put("externalEntryPoint", currentProgram.getSymbolTable().isExternalEntryPoint(entry));
            }

            int references = 0;
            int callReferences = 0;
            ReferenceIterator xrefs = currentProgram.getReferenceManager().getReferencesTo(entry);
            while (xrefs.hasNext()) {
                Reference reference = xrefs.next();
                references++;
                if (reference.getReferenceType().isCall()) {
                    callReferences++;
                }
            }
            row.put("referencesToEntry", references);
            row.put("callReferencesToEntry", callReferences);
            List<String> tags = new ArrayList<>();
            for (FunctionTag tag : function.getTags()) {
                tags.add(tag.getName());
            }
            Collections.sort(tags);
            row.put("tags", tags);
            functions.add(row);
        }
        return functions;
    }

    private List<Map<String, Object>> readInstructionCoverage(FunctionManager manager) throws Exception {
        List<Map<String, Object>> blocks = new ArrayList<>();
        for (MemoryBlock block : currentProgram.getMemory().getBlocks()) {
            if (!block.isExecute()) {
                continue;
            }
            long assignedInstructions = 0;
            long unassignedInstructions = 0;
            long unassignedBytes = 0;
            InstructionIterator instructions = currentProgram.getListing().getInstructions(block.getStart(), true);
            while (instructions.hasNext()) {
                monitor.checkCancelled();
                Instruction instruction = instructions.next();
                if (instruction.getMinAddress().compareTo(block.getEnd()) > 0) {
                    break;
                }
                if (manager.getFunctionContaining(instruction.getMinAddress()) == null) {
                    unassignedInstructions++;
                    unassignedBytes += instruction.getLength();
                } else {
                    assignedInstructions++;
                }
            }
            Map<String, Object> row = new LinkedHashMap<>();
            row.put("name", block.getName());
            row.put("start", block.getStart().toString());
            row.put("end", block.getEnd().toString());
            row.put("assignedInstructions", assignedInstructions);
            row.put("unassignedInstructions", unassignedInstructions);
            row.put("unassignedInstructionBytes", unassignedBytes);
            blocks.add(row);
        }
        return blocks;
    }
}
