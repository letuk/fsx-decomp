const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];assert(run);const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
assert.equal(crypto.createHash('sha256').update(p).digest('hex'),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){let s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),s=c.unique('_DiskSpaceFallbackPilot@16'),sec=c.sections[s.section-1];assert.equal(s.value,0);assert.equal(sec.size,125);assert.equal(sec.relocations.length,3);
const raw=Buffer.from(ob.subarray(sec.raw,sec.raw+125)),original=bytes(0x201325f1,125);
const expected=new Map([[31,[6,'__imp__GetDiskFreeSpaceA@20',0x200e1854]],[64,[20,'__allmul',0x201ec9ea]],[102,[20,'__allmul',0x201ec9ea]]]);
for(const r of sec.relocations){const t=expected.get(r.offset);assert(t);assert.equal(r.type,t[0]);assert.equal(r.symbol.name,t[1]);assert.equal(raw.readUInt32LE(r.offset),0);const value=t[0]===6?t[2]:t[2]-(0x201325f1+r.offset+4);if(t[0]===6)raw.writeUInt32LE(value,r.offset);else raw.writeInt32LE(value,r.offset);}
assert(raw.equals(original),'Full125-byte body mismatch');let b=Buffer.from(raw);b[0]^=1;assert(!b.equals(original));b=Buffer.from(raw);b[31]^=1;assert(!b.equals(original));assert.equal(raw[49],0x99);b=Buffer.from(raw);b[49]=0x90;assert(!b.equals(original));
const result={objectSHA256:crypto.createHash('sha256').update(ob).digest('hex'),originalEntry:'201325f1',completeCodeBytes:125,mappedFields:3,negativeControls:['opcode','API reference','signed extension CDQ'],scope:'Full object fallback body; API/runtime/full DLL not verified'};fs.writeFileSync(run+'/disk-fallback-comparison.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));

