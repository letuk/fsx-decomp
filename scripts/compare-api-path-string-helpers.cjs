const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];assert(run);const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
assert.equal(crypto.createHash('sha256').update(p).digest('hex'),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){let s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),checks=[['_AppendResolvedPathPilot@12',0x20137ec0,62],['_NullablePathLengthPilot@4',0x20137fb0,34]];
for(const [name,entry,size] of checks){const s=c.unique(name),sec=c.sections[s.section-1];assert.equal(sec.size,size);assert.equal(s.value,0);const raw=Buffer.from(ob.subarray(sec.raw,sec.raw+size)),original=bytes(entry,size);
if(size===62){assert.equal(sec.relocations.length,1);const r=sec.relocations[0];assert.equal(r.type,20);assert.equal(r.offset,51);assert.equal(r.symbol.name,'_CopyResolvedPathPilot@12');assert.equal(raw.readUInt32LE(51),0);raw.writeInt32LE(0x20137e17-(entry+55),51);}else assert.equal(sec.relocations.length,0);
assert(raw.equals(original),name+' full mismatch');const b=Buffer.from(raw);b[0]^=1;assert(!b.equals(original));if(size===62){const wrong=Buffer.from(raw);wrong[51]^=1;assert(!wrong.equals(original));}}
const result={objectSHA256:crypto.createHash('sha256').update(ob).digest('hex'),checks:checks.map(([name,entry,size])=>({name,originalEntry:entry.toString(16),completeBytes:size})),completeCodeBytes:96,negativeControls:['opcode in both helpers','wrong copy target'],scope:'Complete object bodies only; copy implementation/runtime/full DLL unresolved'};fs.writeFileSync(run+'/path-string-comparison.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));

