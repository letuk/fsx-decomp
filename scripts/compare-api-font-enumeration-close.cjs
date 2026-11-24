const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];assert(run);const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
assert.equal(crypto.createHash('sha256').update(p).digest('hex'),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){let s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),s=c.unique('_CloseFileEnumerationPilot@4'),sec=c.sections[s.section-1];assert.equal(s.value,0);assert.equal(sec.size,92);assert.equal(sec.relocations.length,4);
const raw=Buffer.from(ob.subarray(sec.raw,sec.raw+92)),original=bytes(0x20134929,92);
for(const r of sec.relocations){assert.equal(r.type,6);let target;if(r.symbol.name==='__imp__HeapFree@12')target=0x200e19a0;else if(r.symbol.name==='__imp__FindClose@4')target=0x200e19c0;else{assert(['_g_fileEnumerationHeapPilot','?g_privateHeapPilot@@3PAXA'].includes(r.symbol.name));target=0x2020e050;}assert.equal(raw.readUInt32LE(r.offset),0);assert.equal(original.readUInt32LE(r.offset),target);raw.writeUInt32LE(target,r.offset);}
assert(raw.equals(original));let bad=Buffer.from(raw);bad[0]^=1;assert(!bad.equals(original));bad=Buffer.from(raw);bad[21]^=1;assert(!bad.equals(original));assert.equal(raw[17],2);bad=Buffer.from(raw);bad[17]=1;assert(!bad.equals(original));
const result={objectSHA256:crypto.createHash('sha256').update(ob).digest('hex'),originalEntry:'20134929',completeCodeBytes:92,mappedFields:4,negativeControls:['opcode','HeapFree target','fallback flag bit'],scope:'Full object close body; iterator creation/advance/fallback implementations and runtime unresolved'};fs.writeFileSync(run+'/font-enumeration-close-comparison.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));

