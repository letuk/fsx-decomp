const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');const run=process.argv[2];assert(run);
const original=fs.readFileSync('original/api.dll'),ob=fs.readFileSync(run+'/api-factory.obj');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
assert.equal(hash(original),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const importIdentities=require('./lib/pe-imports.cjs').readPE32Imports(original);assert.equal(importIdentities.size,828);
const ph=original.readUInt32LE(60),opt=ph+24,base=original.readUInt32LE(opt+28),va=0x20122a0b;
let offset;for(let i=0;i<original.readUInt16LE(ph+6);i++){let p=opt+original.readUInt16LE(ph+20)+i*40,r=original.readUInt32LE(p+12),n=original.readUInt32LE(p+16);if(va-base>=r&&va-base+214<=r+n)offset=original.readUInt32LE(p+20)+va-base-r;}assert(offset!==undefined);
const orig=original.subarray(offset,offset+214),coff=parseNativeCoff(ob),sym=coff.unique('_ForegroundIdleHookPilot@12'),sec=coff.sections[sym.section-1];
assert.equal(sym.value,0);assert.equal(sec.size,214);assert.equal(sec.relocations.length,13);
const targets={'___security_cookie':0x201f6284,'?g_idleHookEnabledPilot@@3HA':0x2020e04c,'_g_hookMainWindowPilot':0x2020e010,'_RunApplicationIdlePilot':0x2012294b,'?g_legacyWindowsPlatformPilot@@3HA':0x201f9954,'?g_foregroundIdleHookPilot@@3PAUHHOOK__@@A':0x2020d010,'@__security_check_cookie@4':0x20148b71};
const imports=fs.readFileSync('artifacts/api.dll/api-inventory/imports.csv','utf8');
for(const r of sec.relocations){const m=r.symbol.name.match(/^__imp__(\w+)@\d+$/);if(m){const line=imports.split(/\r?\n/).filter(l=>l.includes('!'+m[1]+'"'));assert.equal(line.length,1);const target=parseInt(line[0].match(/^"([A-F\d]+)"/)[1],16);assert.equal(importIdentities.get(target)?.name,m[1]);targets[r.symbol.name]=target;}}
function normalize(body,wrongTarget=false){const b=Buffer.from(body);for(const r of sec.relocations){assert([6,20].includes(r.type));assert.equal(b.readInt32LE(r.offset),0);let target=targets[r.symbol.name];assert(target!==undefined);if(wrongTarget&&r.symbol.name==='_RunApplicationIdlePilot')target++;const encoded=r.type===6?target:(target-va-r.offset-4)>>>0;assert.equal(orig.readUInt32LE(r.offset),encoded,'Reference identity '+r.symbol.name);b.writeUInt32LE(encoded,r.offset);}return b;}
const raw=ob.subarray(sec.raw,sec.raw+sec.size),normalized=normalize(raw),differences=[];for(let i=0;i<214;i++)if(normalized[i]!==orig[i])differences.push({offset:i,native:normalized[i],original:orig[i]});
fs.writeFileSync(run+'/foreground-idle-hook-comparison.json',JSON.stringify({entry:va.toString(16),completeCodeBytes:214,objectSHA256:hash(ob),matched:differences.length===0,mappedFields:13,differences,scope:'Complete object body only; idle body and runtime remain unverified'},null,2));
assert.equal(differences.length,0,'Hook body differs; retained diagnostic');
let bad=Buffer.from(raw);bad[0]^=1;assert(!normalize(bad).equals(orig));
bad=Buffer.from(raw);bad.writeUInt32LE(1,sec.relocations[0].offset);assert.throws(()=>normalize(bad));assert.throws(()=>normalize(raw,true));
console.log('Foreground idle hook: 214 complete bytes, 13 target-validated fields; opcode/addend/callee negative controls reject.');
