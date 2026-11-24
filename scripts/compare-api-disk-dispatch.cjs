const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];assert(run);const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
assert.equal(crypto.createHash('sha256').update(p).digest('hex'),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){let s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),s=c.unique('_ResolveDiskSpaceQueryPilot@16'),sec=c.sections[s.section-1];assert.equal(sec.size,139);assert.equal(sec.relocations.length,12);
const raw=Buffer.from(ob.subarray(sec.raw,sec.raw+sec.size)),original=bytes(0x2013266e,139);
const targets={'__imp__GetModuleHandleA@4':0x200e18d8,'__imp__GetProcAddress@8':0x200e1704,'__imp__GetLastError@0':0x200e16f0,'_g_diskSpaceQueryPilot':0x201f7538,'_DiskSpaceFallbackPilot@16':0x201325f1};
for(const r of sec.relocations){let target=targets[r.symbol.name];if(target===undefined){let text,targetString;if(r.symbol.name.includes('GetDiskFreeSpaceExA')){text='GetDiskFreeSpaceExA';targetString=0x200f3120;}else if(r.symbol.name.includes('KERNEL32')){text='KERNEL32';targetString=0x200f3114;}else throw Error('Unknown relocation');const rs=c.sections[r.symbol.section-1];assert.equal(ob.toString('ascii',rs.raw+r.symbol.value,rs.raw+r.symbol.value+text.length+1),text+'\0');assert(bytes(targetString,text.length+1).equals(Buffer.from(text+'\0')));target=targetString;}
assert.equal(raw.readUInt32LE(r.offset),0);assert([6,20].includes(r.type));const mapped=r.type===6?target:target-(0x2013266e+r.offset+4);if(r.type===6)raw.writeUInt32LE(mapped,r.offset);else raw.writeInt32LE(mapped,r.offset);}
assert(raw.equals(original),'Full body mismatch');assert.equal(bytes(0x201f7538,4).readUInt32LE(0),0x2013266e);
let bad=Buffer.from(raw);bad[0]^=1;assert(!bad.equals(original));bad=Buffer.from(raw);bad[sec.relocations[0].offset]^=1;assert(!bad.equals(original));assert.equal(raw[0x45],0x78);bad=Buffer.from(raw);bad[0x45]^=1;assert(!bad.equals(original));
const result={objectSHA256:crypto.createHash('sha256').update(ob).digest('hex'),originalEntry:'2013266e',completeCodeBytes:139,mappedFields:12,initialPointerVerified:true,negativeControls:['opcode','reference byte','error constant'],scope:'Complete object body only; fallback/runtime/full DLL unresolved'};fs.writeFileSync(run+'/disk-dispatch-comparison.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));

