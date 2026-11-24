const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];assert(run);const ob=fs.readFileSync(run+'/api-factory.obj'),p=fs.readFileSync('original/api.dll');
assert.equal(crypto.createHash('sha256').update(p).digest('hex'),'9cd195aeada1505f8ca73c6419ab0f03a8e7e7fc08550b754a4cc4030c370003');
const pe=p.readUInt32LE(60),opt=pe+24,base=p.readUInt32LE(opt+28),table=opt+p.readUInt16LE(pe+20);
function bytes(va,n){for(let j=0;j<p.readUInt16LE(pe+6);j++){let s=table+j*40,r=p.readUInt32LE(s+12),size=p.readUInt32LE(s+16);if(va-base>=r&&va-base+n<=r+size)return p.subarray(p.readUInt32LE(s+20)+va-base-r,p.readUInt32LE(s+20)+va-base-r+n);}throw Error('Unmapped');}
const c=parseNativeCoff(ob),s=c.unique('_QueryPathAttributesPilot@12'),sec=c.sections[s.section-1];assert.equal(s.value,0);
const raw=ob.subarray(sec.raw,sec.raw+sec.size),original=bytes(0x2013341b,202);
const targets={'___security_cookie':[6,0x201f6284],'_ResolvePathPilot@16':[20,0x20133d17],'@__security_check_cookie@4':[20,0x20148b71],'__imp__GetFileAttributesA@4':[6,0x200e18ac],'_QueryFallbackAttributesPilot@4':[20,0x2014507f]};
const refs=sec.relocations.map(r=>{const t=targets[r.symbol.name];assert(t);assert.equal(r.type,t[0]);assert.equal(raw.readUInt32LE(r.offset),0);return {offset:r.offset,symbol:r.symbol.name,type:r.type,originalTarget:t[1].toString(16)};});
const result={objectSHA256:crypto.createHash('sha256').update(ob).digest('hex'),originalBytes:202,nativeBytes:sec.size,references:refs,status:'diagnostic_only_unmatched',scope:'Source compiles; reference identities declared, not offset-aligned equality. No new code coverage.'};
fs.writeFileSync(run+'/path-attributes-diagnostic.json',JSON.stringify(result,null,2)+'\n');console.log(JSON.stringify(result,null,2));
