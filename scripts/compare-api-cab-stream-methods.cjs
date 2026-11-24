// Entire native bodies; only individually verified import fields are normalized.
const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs'),{readPE32Imports}=require('./lib/pe-imports.cjs');
const run=process.argv[2];assert(run);const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
assert.equal(hash(p),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const imports=readPE32Imports(p),pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){const s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),specs=[
 {entry:0x20124356,symbol:'_ReadCabFileMethodPilot@16',size:57,offset:35,target:0x200e18d0,import:'ReadFile',external:'__imp__ReadFile@20'},
 {entry:0x201243c8,symbol:'_SeekCabFileMethodPilot@20',size:64,offset:24,target:0x200e16c8,import:'SetFilePointer',external:'__imp__SetFilePointer@16'},
 {entry:0x20126fc0,symbol:'_SeekCabWrappedMethodPilot@20',size:92}
];
const fixed=JSON.parse(fs.readFileSync('scripts/matching/api-source-pass-accepted.json'));
const rows=specs.map(spec=>{
 const s=c.unique(spec.symbol),sec=c.sections[s.section-1];assert.equal(s.value,0);assert.equal(sec.size,spec.size);
 const raw=Buffer.from(ob.subarray(sec.raw,sec.raw+sec.size)),original=bytes(spec.entry,spec.size);
 assert.equal(sec.relocations.length,spec.target?1:0);
 if(spec.target){const r=sec.relocations[0];assert.equal(r.offset,spec.offset);assert.equal(r.type,6);assert.equal(r.symbol.name,spec.external);assert.equal(raw.readUInt32LE(r.offset),0);assert.equal(original.readUInt32LE(r.offset),spec.target);assert.deepEqual(imports.get(spec.target),{dll:'kernel32.dll',name:spec.import});raw.writeUInt32LE(spec.target,r.offset);}
 assert(raw.equals(original));
 for(const offset of [0,spec.size-2,...(spec.target?[spec.offset]:[])]){const bad=Buffer.from(raw);bad[offset]^=1;assert(!bad.equals(original));}
 for(const f of fixed.functions){const lo=parseInt(f.entry,16);assert(lo+f.bytes<=spec.entry||lo>=spec.entry+spec.size,'Generated body overlaps');}
 return {entry:spec.entry.toString(16),bytes:spec.size,rawEqual:!spec.target,fields:sec.relocations.map(r=>({offset:r.offset,symbol:r.symbol.name,target:spec.target.toString(16)}))};
});
for(let i=0;i<specs.length;i++)for(let j=i+1;j<specs.length;j++)assert(specs[i].entry+specs[i].size<=specs[j].entry||specs[j].entry+specs[j].size<=specs[i].entry);
const result={objectSHA256:hash(ob),functions:rows,completeCodeBytes:rows.reduce((s,r)=>s+r.bytes,0),negativeControls:['opcode','stack cleanup','IAT target for both file methods'],scope:'Three full native methods; import identities verified separately, no full class/runtime proof'};
fs.writeFileSync(run+'/cab-stream-methods-comparison.json',JSON.stringify(result,null,2)+'\n');console.log(JSON.stringify(result,null,2));
