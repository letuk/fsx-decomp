// One-question acceptance audit; never patches either binary.
const fs=require('fs'),path=require('path'),crypto=require('crypto'),assert=require('assert');
process.chdir(path.resolve(__dirname,'..'));
const old='vm-share/modules/fsx/archive/41-rtm-mfc-support',fresh='vm-share/modules/fsx/runs/initinstance-module-local-20261103/candidates/initinstance-module-local-20261103-001';
const read=p=>fs.readFileSync(p),json=p=>JSON.parse(read(p)),sha=p=>crypto.createHash('sha256').update(read(p)).digest('hex').toUpperCase();
function pe(p){const b=read(p),pe=b.readUInt32LE(60),opt=pe+24,sections=[];for(let i=0;i<b.readUInt16LE(pe+6);i++){const s=opt+b.readUInt16LE(pe+20)+40*i;sections.push({name:b.toString('ascii',s,s+8).replace(/\0/g,''),va:0x1000000+b.readUInt32LE(s+12),virtual:b.readUInt32LE(s+8),raw:b.readUInt32LE(s+16),offset:b.readUInt32LE(s+20)});}return {b,pe,opt,sections};}
const a=pe(old+'/fsx.exe'),b=pe(fresh+'/fsx.exe'),orig=pe('original/fsx.exe');
assert.equal(sha(old+'/fsx.exe'),'6953DEA3584BECDFBE03A0D55511D8B4093A593F421603B97EE3938214E8580B');
assert.deepEqual(a.sections,b.sections);
const diffs=[];for(const s of a.sections){for(let i=0;i<s.raw;i++)if(a.b[s.offset+i]!==b.b[s.offset+i])diffs.push({va:s.va+i,old:a.b[s.offset+i],new:b.b[s.offset+i]});}
const code=diffs.filter(d=>d.va>=0x1001c45&&d.va<0x1001c4a);
assert(code.length>0);assert(diffs.every(d=>(d.va>=0x1001c45&&d.va<0x1001c4a)||(d.va>=0x100151c&&d.va<0x100152c)||(d.va>=0x1001264&&d.va<0x1001268)),'Unexpected section changes');
const at=(p,va,n)=>{const s=p.sections.find(s=>va>=s.va&&va+n<=s.va+s.raw);assert(s);return p.b.subarray(s.offset+va-s.va,s.offset+va-s.va+n);};
assert.equal(at(orig,0x1001c5d,5).toString('hex'),'8b40088b16');
assert.equal(at(a,0x1001c45,5).toString('hex'),'8b168b4008');
assert(at(orig,0x1001c5d,5).equals(at(b,0x1001c45,5)));
const before=json(old+'/initinstance-operands.json'),after=json(fresh+'/initinstance-operands.json');assert.equal(before.DifferentInstructions,43);assert.equal(after.DifferentInstructions,41);
const changed=before.Rows.filter((x,i)=>JSON.stringify(x)!==JSON.stringify(after.Rows[i]));assert.equal(changed.length,2);assert(changed.every(x=>['01001c5d','01001c60'].includes(x.OriginalAddress)));
const oldHashes=json(old+'/source-hashes.json'),newHashes=json(fresh+'/source-hashes.json');
const changedSources=newHashes.filter(x=>oldHashes.find(y=>y.File===x.File)?.Hash!==x.Hash).map(x=>x.File);assert.deepEqual(changedSources,['startup_context_pilot.cpp']);
const initial=read(old+'/source/startup_context_pilot.cpp').toString(),actual=read(fresh+'/source/startup_context_pilot.cpp').toString();
assert.equal(actual,initial.replace('startupResult = application->Initialize(moduleState->m_hCurrentInstanceHandle,','HINSTANCE currentInstance = moduleState->m_hCurrentInstanceHandle;\r\n        startupResult = application->Initialize(currentInstance,'));
assert(read(old+'/build.cmd').equals(read(fresh+'/build.cmd')),'Build command changed');
const headerDiffs=[];assert.equal(a.b.length,b.b.length);for(let i=0;i<1024;i++)if(a.b[i]!==b.b[i])headerDiffs.push(i);assert(headerDiffs.every(i=>i>=a.pe+8&&i<a.pe+12),'Unexpected header difference');
const report={originalSHA256:sha('original/fsx.exe'),baselineSHA256:sha(old+'/fsx.exe'),candidateSHA256:sha(fresh+'/fsx.exe'),candidate:fresh,changedSources,buildCommandIdentical:true,sectionLayoutsIdentical:true,sectionDifferences:diffs,headerDifferences:headerDiffs,acceptedCodeRange:{originalVA:'01001C5D',rebuiltVA:'01001C45',bytes:5,originalAndCandidateHex:'8B40088B16',baselineHex:'8B168B4008',rawExactMatch:true},beforeDifferentNonbranchPairs:43,afterDifferentNonbranchPairs:41,allOtherOperandRowsIdentical:true,scope:'Only two reordered loads (five-byte range); all other section changes confined to CodeView GUID/debug timestamp, header changes to COFF timestamp. Not a whole InitInstance match. Full controller regression report retained with candidate.'};
fs.writeFileSync('artifacts/fsx.exe/initinstance-module-local-acceptance-20261103.json',JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report,null,2));
