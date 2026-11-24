// Complete raw body gate; no address normalization or dependency acceptance.
const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];assert(run);
const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
assert.equal(hash(p),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){const s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),s=c.unique('_ReleaseCabStreamPilot@4'),sec=c.sections[s.section-1];
assert.equal(s.value,0);assert.equal(sec.size,33);assert.equal(sec.relocations.length,0);
const raw=ob.subarray(sec.raw,sec.raw+33),original=bytes(0x201333ac,33);
assert(raw.equals(original));
assert.equal(raw[10],0x75);assert.equal(raw[23],8);assert.equal(raw[31],4);
for(const [offset,value] of [[10,0x74],[23,0xc],[31,8]]){const bad=Buffer.from(raw);bad[offset]=value;assert(!bad.equals(original));}
const fixed=JSON.parse(fs.readFileSync('scripts/matching/api-source-pass-accepted.json'));
for(const f of fixed.functions){const lo=parseInt(f.entry,16);assert(lo+f.bytes<=0x201333ac||lo>=0x201333cd,'Generated coverage overlaps');}
const rs=c.unique('_ReadCabStreamPilot@16'),rsec=c.sections[rs.section-1];
assert.equal(rs.value,0);assert.equal(rsec.size,39);assert.equal(rsec.relocations.length,0);
const read=ob.subarray(rsec.raw,rsec.raw+39),readOriginal=bytes(0x201333cd,39);
assert(read.equals(readOriginal));assert.equal(read[13],0x74);assert.equal(read[22],0xc);assert.equal(read[37],16);
for(const [offset,value] of [[13,0x75],[22,8],[37,12]]){const bad=Buffer.from(read);bad[offset]=value;assert(!bad.equals(readOriginal));}
for(const f of fixed.functions){const lo=parseInt(f.entry,16);assert(lo+f.bytes<=0x201333cd||lo>=0x201333f4,'Generated read coverage overlaps');}
const result={functions:[{entry:'201333ac',bytes:33},{entry:'201333cd',bytes:39}],completeCodeBytes:72,rawEqual:true,relocations:0,objectSHA256:hash(ob),negativeControls:['NULL branch direction','vtable slot','stack cleanup (both bodies)'],scope:'Complete release/read wrappers only; indirect callee implementations/runtime unresolved'};
fs.writeFileSync(run+'/cab-stream-wrappers-comparison.json',JSON.stringify(result,null,2)+'\n');console.log(JSON.stringify(result,null,2));
