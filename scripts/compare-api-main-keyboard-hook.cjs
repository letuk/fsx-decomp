const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];assert(run);const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
assert.equal(crypto.createHash('sha256').update(p).digest('hex'),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){let s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),s=c.unique('_MainKeyboardHookPilot@12'),sec=c.sections[s.section-1];assert.equal(sec.size,188);assert.equal(sec.relocations.length,8);
const raw=Buffer.from(ob.subarray(sec.raw,sec.raw+188)),original=bytes(0x20122834,188);
const targets={'_g_windowModeBoundaryPilot':0x2020de70,'_g_hookMainWindowPilot':0x2020e010,'_g_keyboardHookPilot':0x2020d018,'__imp__GetFocus@0':0x200e1a9c,'__imp__GetKeyState@4':0x200e1a98,'__imp__PostMessageA@16':0x200e1ba0,'__imp__CallNextHookEx@16':0x200e1a94};
for(const r of sec.relocations){assert.equal(r.type,6);const target=targets[r.symbol.name];assert(target!==undefined);assert.equal(raw.readUInt32LE(r.offset),0);assert.equal(original.readUInt32LE(r.offset),target);raw.writeUInt32LE(target,r.offset);}
const differences=[];for(let i=0;i<188;i++)if(raw[i]!==original[i])differences.push({offset:i,native:raw[i],original:original[i]});if(differences.length){fs.writeFileSync(run+'/main-hook-mismatch.json',JSON.stringify({differences,status:'unmatched'},null,2));console.log(JSON.stringify(differences));}assert(raw.equals(original),'Full188-byte mismatch');
let bad=Buffer.from(raw);bad[0]^=1;assert(!bad.equals(original));bad=Buffer.from(raw);bad[21]^=1;assert(!bad.equals(original));const message=raw.indexOf(Buffer.from('6805010000','hex'));assert(message>=0);bad=Buffer.from(raw);bad[message+1]^=1;assert(!bad.equals(original));
const result={objectSHA256:crypto.createHash('sha256').update(ob).digest('hex'),originalEntry:'20122834',completeCodeBytes:188,mappedFields:8,negativeControls:['opcode','mode-interface pointer','WM_SYSKEYUP constant'],scope:'Full object hook body, two observed vtable slots only; mode meaning/runtime/full DLL unresolved'};fs.writeFileSync(run+'/main-hook-comparison.json',JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));

