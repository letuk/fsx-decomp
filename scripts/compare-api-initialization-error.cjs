const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];assert(run);const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
assert.equal(crypto.createHash('sha256').update(p).digest('hex'),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){let s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),s=c.unique('_ReportInitializationErrorPilot@12'),sec=c.sections[s.section-1];assert.equal(s.value,0);assert.equal(sec.size,48);assert.equal(sec.relocations.length,3);
const expected=new Map([[7,['_g_initializationErrorCallbackPilot',0x2020de78]],[18,['_g_initializationErrorContextPilot',0x2020e028]],[36,['_g_initializationErrorTextPilot',0x2020d468]]]);
const raw=Buffer.from(ob.subarray(sec.raw,sec.raw+48)),original=bytes(0x201230da,48);
for(const r of sec.relocations){const t=expected.get(r.offset);assert(t);assert.equal(r.type,6);assert.equal(r.symbol.name,t[0]);assert.equal(r.symbol.section,0);assert.equal(raw.readUInt32LE(r.offset),0);assert.equal(original.readUInt32LE(r.offset),t[1]);raw.writeUInt32LE(t[1],r.offset);}
assert(raw.equals(original));const bad=Buffer.from(raw);bad[0]^=1;assert(!bad.equals(original));const wrong=Buffer.from(raw);wrong.writeUInt32LE(0x2020de7c,7);assert(!wrong.equals(original));const arg=Buffer.from(raw);arg[28]^=1;assert(!arg.equals(original));
const exp=base+p.readUInt32LE(opt+96),ordinalBase=bytes(exp+16,4).readUInt32LE(0),eat=base+bytes(exp+28,4).readUInt32LE(0);assert.equal(base+bytes(eat+4*(264-ordinalBase),4).readUInt32LE(0),0x201230da);
const result={objectSHA256:crypto.createHash('sha256').update(ob).digest('hex'),originalEntry:'201230da',originalOrdinal:264,completeCodeBytes:48,mappedFields:3,negativeControls:['opcode rejected','wrong callback global rejected','callback argument mutation rejected'],scope:'Object body only; unresolved globals/callback, no linked export/runtime claim'};
fs.writeFileSync(run+'/initialization-error-comparison.json',JSON.stringify(result,null,2)+'\n');console.log(JSON.stringify(result,null,2));
