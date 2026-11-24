const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];assert(run);const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
assert.equal(crypto.createHash('sha256').update(p).digest('hex'),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){let s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),s=c.unique('_CheckPathExistsPilot@8'),sec=c.sections[s.section-1];assert.equal(s.value,0);assert.equal(sec.size,49);assert.equal(sec.relocations.length,1);
const r=sec.relocations[0];assert.equal(r.offset,18);assert.equal(r.type,20);assert.equal(r.symbol.name,'_QueryPathAttributesPilot@12');assert.equal(r.symbol.value,0);assert(r.symbol.section>=0);assert.equal(c.unique('_QueryPathAttributesPilot@12'),r.symbol);
const candidate=Buffer.from(ob.subarray(sec.raw,sec.raw+49)),original=bytes(0x20132c8e,49);assert.equal(candidate.readInt32LE(18),0);assert.equal(0x20132c8e+22+original.readInt32LE(18),0x2013341b);
candidate.writeInt32LE(0x2013341b-(0x20132c8e+22),18);assert(candidate.equals(original));
const bad=Buffer.from(candidate);bad[0]^=1;assert(!bad.equals(original));const wrong=Buffer.from(candidate);wrong.writeInt32LE(wrong.readInt32LE(18)+1,18);assert(!wrong.equals(original));
const result={objectSHA256:crypto.createHash('sha256').update(ob).digest('hex'),originalEntry:'20132c8e',completeCodeBytes:49,relocations:1,target:'2013341b',negativeControls:['opcode rejected','wrong call target rejected'],scope:'Object wrapper only; dependency unresolved, no linked DLL/runtime claim'};
fs.writeFileSync(run+'/path-wrapper-comparison.json',JSON.stringify(result,null,2)+'\n');console.log(JSON.stringify(result,null,2));
