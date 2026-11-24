// Explicit fsx.exe; default read-only preflight. Pass apply to append evidence.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.security.MessageDigest;
public class DocumentPeImports extends GhidraScript {
 void hash(long a,int len,String expected)throws Exception{byte[] b=new byte[len];currentProgram.getMemory().getBytes(toAddr(a),b);StringBuilder h=new StringBuilder();for(byte x:MessageDigest.getInstance("SHA-256").digest(b))h.append(String.format("%02X",x&255));if(!h.toString().equals(expected))throw new Exception("Live bytes differ at "+toAddr(a));}
 void append(long a,String note){String old=currentProgram.getListing().getComment(CodeUnit.PLATE_COMMENT,toAddr(a));currentProgram.getListing().setComment(toAddr(a),CodeUnit.PLATE_COMMENT,old==null?note:old+"\n\n"+note);}
 public void run()throws Exception{
  if(!currentProgram.getName().equals("fsx.exe")||currentProgram.getImageBase().getOffset()!=0x01000000L)throw new Exception("Wrong program");
  hash(0x01001000L,568,"89350D371243F294D551EBE00DB20FBA9C7CE1A868D360BC4FA656F5C3829AB3");hash(0x01002e10L,120,"25CA7D50D6DCB1912B8D24FE1B7FB250F429933D859A5BEB8E3207F7255F2FD4");hash(0x01001260L,28,"4024DF7B554DA52BFF8612918A837B8483A42EA7AB5F203DCF8A16721315E00C");
  int entries=0,externalNonfunctions=0;for(long[] r:new long[][]{{0x01001000L,33},{0x01001088L,67},{0x01001198L,32},{0x0100121cL,3},{0x0100122cL,2}}){for(int i=0;i<r[1];i++){entries++;for(Reference ref:currentProgram.getReferenceManager().getReferencesFrom(toAddr(r[0]+4*i))){if(ref.getToAddress().isExternalAddress()&&currentProgram.getFunctionManager().getFunctionAt(ref.getToAddress())==null){externalNonfunctions++;println("External nonfunction reference "+toAddr(r[0]+4*i)+" -> "+ref.getToAddress()+" "+currentProgram.getSymbolTable().getPrimarySymbol(ref.getToAddress()));}}}}
  println("Verified IAT bytes/import-descriptor/debug anchors; entries="+entries+" externalNonfunctionRefs="+externalNonfunctions);
  if(getScriptArgs().length==0||!getScriptArgs()[0].equals("apply")){println("Read-only preflight; unsaved_changes="+currentProgram.isChanged());return;}
  int tx=currentProgram.startTransaction("Document full EXE import contents and remaining PE layout");boolean ok=false;try{
   append(0x01001000L,"2026-11-02 full ordinary-import audit:137 entries plus five zero module terminators,568 bytes. DLL/ordinal/name identities and name hints agree with source-based rebuild32; ILT and initial IAT agree within each unbound image. Module block order and relative block offsets agree, but entry order differs in KERNEL32/MFC80/MSVCR80/USER32; only ole32 entries retain order. Absolute locations/entry order remain open. No imported names/types changed. See notes/re/fsx.exe/pe-imports-and-directories.md.");
   append(0x01002e10L,"2026-11-02 complete120-byte import descriptor directory: five descriptors plus null descriptor. Original/rebuilt DLL spelling and descriptor order MFC80.DLL, MSVCR80.dll, KERNEL32.dll, USER32.dll, ole32.dll agree, with zero timestamp/forwarder fields and matching import contents/hints/counts. Pointer RVAs differ with layout and symbol order; no whole-byte directory match claimed. See notes/re/fsx.exe/pe-imports-and-directories.md.");
   append(0x01001260L,"2026-11-02 debug directory audited separately from code/data matching: one CodeView Type2 record, raw size32 at RVA1518, RSDS signature, Age1, PDB path fsx.pdb. Existing Ghidra PDB data model31 bytes does not include the path null terminator at01001537, which is included in the32-byte raw record. Rebuild has a different GUID/timestamp and longer worker-path record; not an application logic difference or currently matched metadata. See notes/re/fsx.exe/pe-imports-and-directories.md.");
   ok=true;
  }finally{currentProgram.endTransaction(tx,ok);}
  println("Applied comments only; save separately.");
 }
}
