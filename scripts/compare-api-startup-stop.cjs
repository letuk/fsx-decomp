const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];assert(run);const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
assert.equal(crypto.createHash('sha256').update(p).digest('hex'),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){let s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),s=c.unique('_StopStartupWindowPilot'),sec=c.sections[s.section-1];assert.equal(sec.size,98);assert.equal(sec.relocations.length,12);
const raw=Buffer.from(ob.subarray(sec.raw,sec.raw+98)),original=bytes(0x2013b6a4,98);
const targets={'_g_startupWindowPilot':0x2020e0a4,'_g_startupThreadPilot':0x2020e0a8,'_g_startupInstancePilot':0x2020e024,'__imp__IsWindow@4':0x200e1b40,'__imp__PostMessageA@16':0x200e1ba0,'__imp__WaitForSingleObject@8':0x200e1874,'__imp__CloseHandle@4':0x200e176c,'__imp__UnregisterClassA@8':0x200e1b94};
for(const r of sec.relocations){assert.equal(r.type,6);let target=targets[r.symbol.name];if(target===undefined){assert(r.symbol.name.includes('FlightSimSplash'));const text='FlightSimSplash\0',ss=c.sections[r.symbol.section-1];assert.equal(ob.toString('ascii',ss.raw+r.symbol.value,ss.raw+r.symbol.value+text.length),text);target=0x200f34c0;assert(bytes(target,text.length).equals(Buffer.from(text)));}assert.equal(raw.readUInt32LE(r.offset),0);assert.equal(original.readUInt32LE(r.offset),target);raw.writeUInt32LE(target,r.offset);}
assert(raw.equals(original));let bad=Buffer.from(raw);bad[0]^=1;assert(!bad.equals(original));bad=Buffer.from(raw);bad[1]^=1;assert(!bad.equals(original));assert.equal(raw[30],0x10);bad=Buffer.from(raw);bad[30]=0x12;assert(!bad.equals(original));
const result={objectSHA256:crypto.createHash('sha256').update(ob).digest('hex'),originalEntry:'2013b6a4',completeCodeBytes:98,mappedFields:12,negativeControls:['opcode','window global','WM_CLOSE changed to WM_QUIT'],scope:'Complete object body only; thread/window procedure/data lifetime and runtime unresolved'};fs.writeFileSync(run+'/startup-stop-comparison.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));

