// Exact native COFF body comparison. No relocation masking, padding or truncation.
const fs=require('fs'),path=require('path'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const root=path.resolve(__dirname,'..');process.chdir(root);
const run=process.argv[2];assert(run,'Pass retained run directory');
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const original=fs.readFileSync('original/api.dll');
assert.equal(sha(original),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const importIdentities=require('./lib/pe-imports.cjs').readPE32Imports(original);assert.equal(importIdentities.size,828);
const pe=original.readUInt32LE(60),opt=pe+24,base=original.readUInt32LE(opt+28),sections=[];
for(let i=0;i<original.readUInt16LE(pe+6);i++){const s=opt+original.readUInt16LE(pe+20)+40*i;sections.push({va:base+original.readUInt32LE(s+12),raw:original.readUInt32LE(s+20),size:original.readUInt32LE(s+16)});}
function bytes(va,n){const s=sections.find(s=>va>=s.va&&va+n<=s.va+s.size);assert(s);return original.subarray(s.raw+va-s.va,s.raw+va-s.va+n);}
const evidence='artifacts/api.dll/api-source-pass-20261107',manifest=JSON.parse(fs.readFileSync(evidence+'/manifest.json'));
let instructions=0,listingBytes=0;
for(const f of manifest.functions){
 const asm=fs.readFileSync(evidence+'/functions/'+f.entry+'.asm','utf8');
 for(const line of asm.trim().split('\n').filter(Boolean)){
  const m=line.match(/^([a-f\d]+)  ([a-f\d]+)  /);assert(m,line);
  const b=Buffer.from(m[2],'hex');assert(bytes(parseInt(m[1],16),b.length).equals(b),'Live/original mismatch '+m[1]);instructions++;listingBytes+=b.length;
 }
}
const ob=fs.readFileSync(run+'/api-factory.obj'),coff=parseNativeCoff(ob);
const source=JSON.parse(fs.readFileSync(run+'/source-pass.json'));
const rows=[];let matchedBytes=0,matched=0,normalizedBytes=0,normalizedCount=0;
const ranges=[];
for(const f of source.rows.filter(r=>r.status==='source_candidate')){
 const symbols=[...coff.symbols.values()].filter(s=>s.section>0&&new RegExp('^[_@]'+f.symbol+'(?:@\\d+)?$').test(s.name));
 assert.equal(symbols.length,1,'Unique emitted function '+f.entry);
 const sym=symbols[0],sec=coff.sections[sym.section-1];assert.equal(sym.value,0,'Expected /Gy function section');
 const native=ob.subarray(sec.raw,sec.raw+sec.size),orig=bytes(parseInt(f.entry,16),f.bodyBytes);
 const equal=sec.relocations.length===0&&native.equals(orig);
 const normalized=Buffer.from(native);let valid=native.length===orig.length;const mappings=[];
 for(const r of sec.relocations){
  const g=(f.globals||[]).find(g=>'_'+g.symbol===r.symbol?.name);
  if(r.type===0x14){
   const dep=(f.dependencies||[]).find(d=>new RegExp('^[_@]'+d.symbol+'(?:@\\d+)?$').test(r.symbol?.name||''));
   if(!dep||r.offset+4>orig.length){valid=false;continue;}
   const target=(parseInt(dep.entry,16)+native.readInt32LE(r.offset))>>>0;
   const originalTarget=(parseInt(f.entry,16)+r.offset+4+orig.readInt32LE(r.offset))>>>0;
   if(target!==originalTarget){valid=false;continue;}
   normalized.writeInt32LE((target-parseInt(f.entry,16)-r.offset-4)|0,r.offset);
   mappings.push({offset:r.offset,symbol:r.symbol.name,address:dep.entry,addend:native.readInt32LE(r.offset),target:target.toString(16),kind:'REL32 callee'});continue;
  }
  const imported=(f.imports||[]).find(i=>new RegExp('^__imp__'+i.name+'(?:@\\d+)?$').test(r.symbol?.name||''));
  if(r.type===6&&imported&&r.offset+4<=orig.length){
   const identity=importIdentities.get(parseInt(imported.address,16));
   if(!identity||identity.dll!==imported.dll.toLowerCase()||identity.name!==imported.name){valid=false;continue;}
   const target=(parseInt(imported.address,16)+native.readInt32LE(r.offset))>>>0;
   if(target!==orig.readUInt32LE(r.offset)){valid=false;continue;}
   normalized.writeUInt32LE(target,r.offset);mappings.push({offset:r.offset,symbol:r.symbol.name,address:imported.address,addend:native.readInt32LE(r.offset),target:target.toString(16),kind:'IAT',identity:imported.dll+'!'+imported.name});continue;
  }
  if(r.type!==6||!g||!/^20[0-9a-f]{6}$/.test(g.address)||r.offset+4>native.length||r.offset+4>orig.length){valid=false;continue;}
  const target=(parseInt(g.address,16)+native.readInt32LE(r.offset))>>>0;
  if(orig.readUInt32LE(r.offset)!==target){valid=false;continue;}
  normalized.writeUInt32LE(target,r.offset);mappings.push({offset:r.offset,symbol:r.symbol.name,address:g.address,addend:native.readInt32LE(r.offset),target:target.toString(16)});
 }
 const normalizedMatch=valid&&normalized.equals(orig);
 const row={entry:f.entry,symbol:sym.name,originalBytes:f.bodyBytes,nativeBytes:native.length,relocations:sec.relocations.length,rawEqual:equal,normalizedMatch,mappings,status:normalizedMatch?'matched_model_body':'unmatched',originalSHA256:sha(orig),nativeSHA256:sha(native)};
 if(normalizedMatch){
  let mutated=Buffer.from(normalized);mutated[0]^=1;assert(!mutated.equals(orig));
  assert(!normalized.subarray(0,normalized.length-1).equals(orig));
  for(const m of mappings){mutated=Buffer.from(normalized);mutated[m.offset]^=1;assert(!mutated.equals(orig));}
  const lo=parseInt(f.entry,16),hi=lo+f.bodyBytes;assert(!ranges.some(r=>lo<r[1]&&hi>r[0]));ranges.push([lo,hi]);
  normalizedCount++;normalizedBytes+=native.length;
  if(equal){matched++;matchedBytes+=native.length;}
 }else{
  row.firstDifferentByte=Array.from({length:Math.max(native.length,orig.length)},(_,i)=>i).find(i=>native[i]!==orig[i]);
 }
 rows.push(row);
}
const result={scope:'Complete contiguous Ghidra-model bodies; raw equality and individually target-validated DIR32 references separate. No linked DLL, data implementation, runtime or unassigned-flow claim',originalSHA256:sha(original),objectSHA256:sha(ob),liveListingVerified:{functions:manifest.functions.length,instructions,bytes:listingBytes},candidates:rows.length,exactModelBodies:matched,exactModelBytes:matchedBytes,normalizedModelBodies:normalizedCount,normalizedModelBytes:normalizedBytes,negativeControls:['first-byte mutation','truncated body','each mapped target mutation'],rows};
const acceptedFile='scripts/matching/api-source-pass-accepted.json';
if(fs.existsSync(acceptedFile)){
 const accepted=JSON.parse(fs.readFileSync(acceptedFile));assert.equal(accepted.originalSHA256,result.originalSHA256);
 for(const required of accepted.functions){const actual=rows.find(r=>r.entry===required.entry);assert(actual&&actual.normalizedMatch,'Accepted-body regression '+required.entry);assert.equal(actual.originalBytes,required.bytes);if(required.rawEqual)assert(actual.rawEqual,'Raw body became normalized '+required.entry);}
 result.acceptedRegressionBodies=accepted.functions.length;
}
fs.writeFileSync(run+'/source-pass-comparison.json',JSON.stringify(result,null,2)+'\n');
console.log(JSON.stringify({...result,rows:undefined},null,2));
