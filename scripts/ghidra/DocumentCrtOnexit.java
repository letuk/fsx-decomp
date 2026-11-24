// Explicit fsx.exe target; read-only preflight by default, pass apply to edit.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.data.VoidDataType;
import java.security.MessageDigest;
public class DocumentCrtOnexit extends GhidraScript {
 void hash(long a,int length,String expected)throws Exception{byte[] b=new byte[length];currentProgram.getMemory().getBytes(toAddr(a),b);StringBuilder s=new StringBuilder();for(byte x:MessageDigest.getInstance("SHA-256").digest(b))s.append(String.format("%02X",x&255));if(!s.toString().equals(expected))throw new Exception("Live bytes differ at "+toAddr(a));}
 void comment(long a,String text){String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,toAddr(a));currentProgram.getListing().setComment(toAddr(a),CodeUnit.PLATE_COMMENT,old==null?text:old+"\n\n"+text);}
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new Exception("Wrong program");
  hash(0x01002651L,150,"7FAA2E011B9F81E503B13C25213B2B35EFE7074AE05ED07E6F7678B6AF4D100F");hash(0x010026e7L,9,"6A9F50499FE536CB35993B65BEB88A614AEB88B266DC603543D25A476F414EA3");hash(0x010026f5L,18,"91C11191C65A96F71FA01E851626CBD69B951750943E2800FB175C99CE115678");hash(0x01002da8L,28,"79816A3D6FDA4B0D46A123F55F3AE76A771784042707CD9EF1980460153BBAF1");
  FunctionManager fm=currentProgram.getFunctionManager();Function onexit=fm.getFunctionAt(toAddr(0x01002651L)),cleanup=fm.getFunctionAt(toAddr(0x010026e7L)),adapter=fm.getFunctionAt(toAddr(0x010026f5L));
  if(onexit==null||!onexit.getName().equals("__onexit")||onexit.getBody().getNumAddresses()!=150||onexit.getParameterCount()!=0||!onexit.getReturnType().getName().equals("undefined"))throw new Exception("Existing onexit model changed");
  if(cleanup==null||!cleanup.getName().equals("FUN_010026e7")||cleanup.getBody().getNumAddresses()!=9||adapter==null||!adapter.getName().equals("FUN_010026f5")||adapter.getBody().getNumAddresses()!=18)throw new Exception("Existing helper model changed");
  println("CHECKED onexit150, cleanup9, adapter18, finally table28; existing meaningful onexit name preserved");
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only preflight; no edits");return;}
  int tx=currentProgram.startTransaction("Document verified CRT exit registration and finally cleanup");boolean ok=false;
  try{
   cleanup.setName("ReleaseCrtExitLock",SourceType.USER_DEFINED);cleanup.setCallingConvention("__cdecl");cleanup.setReturnType(VoidDataType.dataType,SourceType.USER_DEFINED);
   adapter.setName("atexit",SourceType.USER_DEFINED);
   comment(0x01002651L,"2026-11-02 complete150-byte CRT atonexit.obj registration routine normalized-exact with14 validated fields. Decodes the encoded begin pointer: sentinel -1 selects imported MSVCR80 _onexit; otherwise acquires lock8, decodes begin/end, calls imported __dllonexit, re-encodes the updated pointers and releases lock through the checked cleanup. EH4 finally record references the same cleanup. Existing function name retained; standard _onexit_t callback model checked separately. No imported runtime implementation or actual execution claim. See notes/re/fsx.exe/crt-onexit.md.");
   comment(0x010026e7L,"2026-11-02 verified9-byte cleanup: _unlock(8), caller stack adjustment, RET. Called on the normal registration path and referenced by its EH4 finally record. ReleaseCrtExitLock is a reconstructed descriptive name replacing the automatic FUN label, not a recovered original identifier. See notes/re/fsx.exe/crt-onexit.md.");
   comment(0x010026f5L,"2026-11-02 complete18-byte standard atexit adapter normalized-exact after checking its call to __onexit. Maps a nonnull registration result to0 and null to-1. New atexit name identifies the matched CRT counterpart; old name was automatic FUN_010026f5. Source-level atexit callbacks return void, while the existing _onexit_t archive model returns int; these pointer models are kept distinct. See notes/re/fsx.exe/crt-onexit.md.");
   comment(0x01002da8L,"2026-11-02 verified28-byte EH4 finally table for __onexit. Header/enclosing-level bytes match; filter field0 marks finally handling, handler pointer010026e7 maps to the complete verified lock8 cleanup. This is not a catch/filter callback prototype. See notes/re/fsx.exe/crt-onexit.md.");
   currentProgram.getReferenceManager().addMemoryReference(toAddr(0x01002dc0L),toAddr(0x010026e7L),RefType.DATA,SourceType.USER_DEFINED,0);
   ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
  println("Applied names/comments/finally reference; registration and adapter callback prototypes are set separately using standard archive types. No code bytes changed.");
 }
}
