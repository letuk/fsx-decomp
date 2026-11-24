// Deliberately baseline-specific: reconcile existing evidence, never infer coverage from names.
const fs=require('fs'), path=require('path'), crypto=require('crypto'), assert=require('assert');
const root=path.resolve(__dirname,'..'); process.chdir(root);
const archive='vm-share/modules/fsx/archive/41-rtm-mfc-support', evidence=archive+'/comparison.json';
const read=p=>fs.readFileSync(p), json=p=>JSON.parse(read(p)), hash=p=>crypto.createHash('sha256').update(read(p)).digest('hex').toUpperCase();
const oh=hash('original/fsx.exe'), rh=hash(archive+'/fsx.exe');
assert.equal(oh,'61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735');
assert.equal(rh,'6953DEA3584BECDFBE03A0D55511D8B4093A593F421603B97EE3938214E8580B');
const c=json(evidence), hex=n=>n.toString(16).padStart(8,'0').toUpperCase(), num=v=>typeof v==='number'?v:parseInt(v,16);
for(const x of Object.values(c)){if(x.OriginalSHA256)assert.equal(x.OriginalSHA256,oh);if(x.RebuiltSHA256||x.PilotSHA256)assert.equal(x.RebuiltSHA256||x.PilotSHA256,rh);}
const map=read(archive+'/fsx.map').toString(), symbols=new Map();
for(const m of map.matchAll(/^\s*\w{4}:\w{8}\s+(\S+)\s+([\da-f]{8})\b/gim)) symbols.set(m[1],num(m[2]));
function sym(s){assert(symbols.has(s),'Missing MAP symbol '+s);return symbols.get(s);}
function pe(p){const b=read(p), h=b.readUInt32LE(60), opt=h+24, base=b.readUInt32LE(opt+28), sections=[];
 for(let i=0;i<b.readUInt16LE(h+6);i++){const s=opt+b.readUInt16LE(h+20)+40*i;sections.push({name:b.toString('ascii',s,s+8).replace(/\0/g,''),va:base+b.readUInt32LE(s+12),virtual:b.readUInt32LE(s+8),raw:b.readUInt32LE(s+16),offset:b.readUInt32LE(s+20)});}
 return {b,base,sections,at(va,n){const s=sections.find(s=>va>=s.va&&va+n<=s.va+Math.max(s.raw,s.virtual));assert(s,'Unmapped '+hex(va));const out=Buffer.alloc(n);for(let i=0;i<n;i++)if(va-s.va+i<s.raw)out[i]=b[s.offset+va-s.va+i];return out;},u32(va){return this.at(va,4).readUInt32LE();},rel(va,o){return va+o+4+this.at(va+o,4).readInt32LE();}};
}
const a=pe('original/fsx.exe'), b=pe(archive+'/fsx.exe'), claims=[];
function add(label,o,r,n,status,category,check,rule='',rn=n){
 o=o==null?null:num(o);r=r==null?null:num(r);assert(n>0&&Number.isInteger(n));
 if(status==='exact raw')assert(a.at(o,n).equals(b.at(r,rn)),label+' raw mismatch');
 const row={id:'R'+String(claims.length+1).padStart(4,'0'),label,category,status,original:o==null?null:{va:hex(o),bytes:n},rebuilt:r==null?null:{va:hex(r),bytes:rn},evidence:check,normalization:rule||'none',originalSHA256:oh,rebuiltSHA256:rh};claims.push(row);return row;
}
const normalized='Only individually checked address/displacement fields are replaced after target, import, string or state validation; see named comparator. All remaining bytes compared.';
const scripts={Lifecycle:'app-lifecycle',Copy:'copy-pilot',Dispatch:'app-dispatch',Formatter:'formatter-pilot',PreCInit:'pre-c-init',AtlCodepage:'atl-codepage',CrtFailureStack:'crt-failure-stack',StaticInitialization:'static-glue',CrtOnexit:'crt-onexit',CrtImageHelpers:'crt-image-helpers',RuntimeRemainders:'runtime-remainders',CrtStartup:'crt-startup',Version:'version-pilot',PreCppInit:'pre-cpp-init',DataConstants:'data-constants'};
function walk(x,p,cat,check){if(!x||typeof x!=='object')return;
 const r=x.RebuiltVA||x.PilotVA,n=x.Length||x.Bytes||x.RegionLength;
 if(x.OriginalVA&&r&&n){const raw=x.RawExactMatch===true, norm=x.AddressNormalizedExactMatch===true||x.RegionAddressNormalizedExactMatch===true||x.TargetNormalizedExactMatch===true||x.TargetsVerified===true||x.InitialZeroMatch===true;
 if(raw||norm)add(x.Name||x.Role||x.Function||p,x.OriginalVA,r,n,raw?'exact raw':'target-validated normalized',cat,check+'#'+p,raw?'':normalized);
 }
 for(const [k,v]of Object.entries(x))if(typeof v==='object')walk(v,p+'.'+k,cat,check);
}
for(const [k,s]of Object.entries(scripts)){const x=c[k];assert.equal(x.OriginalSHA256,oh);assert.equal(x.RebuiltSHA256||x.PilotSHA256,rh);walk(x,k,k==='DataConstants'?'executable-section data':'executable code',evidence+'; scripts/compare-'+s+'.ps1');}
// Explicit dependencies whose compact reports omit address pairs. Extents from comparator source.
function body(o,s,n,check,raw=false){add(s,o,sym(s),n,raw?'exact raw':'target-validated normalized','executable code',evidence+'#'+check+'; scripts/compare-'+({Ole:'ole-filter',CrtStartup:'crt-startup',PreCInit:'pre-c-init',PreCppInit:'pre-cpp-init',StaticInitialization:'static-glue'}[check])+'.ps1',raw?'':normalized);}
for(const [o,s,n,k,raw]of [
 [0x10018e0,'?InitializeOleAndMessageFilterPilot@@YAHXZ',237,'Ole'],[0x10018d0,'?OnMessagePending@FSXMessageFilterPilot@@UAEHPBUtagMSG@@@Z',8,'Ole',true],
 [0x1002ca8,'__ehhandler$?InitializeOleAndMessageFilterPilot@@YAHXZ',27,'Ole'],[0x100262e,'_WinMainCRTStartup',10,'CrtStartup'],[0x10029d4,'___security_init_cookie',148,'CrtStartup'],[0x1002940,'__SEH_prolog4',69,'CrtStartup'],[0x1002985,'__SEH_epilog4',20,'CrtStartup',true],[0x1002c03,'_WinMain@16',5,'CrtStartup'],[0x10029cc,'__matherr',3,'PreCInit',true],[0x100299e,'__setdefaultprecision',41,'PreCInit'],[0x10027d9,'__RTC_Initialize',36,'PreCInit'],[0x1002802,'__RTC_Terminate',36,'PreCppInit'],[0x1002c0d,'?AfxInitialize@@YGHHK@Z',38,'StaticInitialization']])body(o,s,n,k,raw);
const vt=sym('??_7FSXMessageFilterPilot@@6B@');
add('Filter deleting destructor',0x10018a0,b.u32(vt+4),35,'target-validated normalized','executable code',evidence+'#Ole',normalized);
add('Filter vtable',0x1001458,vt,112,'target-validated normalized','executable-section data',evidence+'#Ole','28 individually verified dispatch targets; original class name/size not inferred.');
for(const i of [2,3,4])add('Filter RET leaf slot '+i,a.u32(0x1001458+i*4),b.u32(vt+i*4),i===3?1:3,'exact raw','executable code',evidence+'#Ole');
const ha=0x1002ca8,hb=sym('__ehhandler$?InitializeOleAndMessageFilterPilot@@YAHXZ');
const ia=a.u32(ha+18),ib=b.u32(hb+18),ua=a.u32(ia+8),ub=b.u32(ib+8),ca=a.u32(ua+4),cb=b.u32(ub+4);
for(const [label,o,r,n,cat]of [['OLE FuncInfo',ia,ib,36,'executable-section data'],['OLE unwind map',ua,ub,8,'executable-section data'],['OLE unwind cleanup',ca,cb,8,'executable code'],['OLE delete adapter',a.rel(ca,4),b.rel(cb,4),10,'executable code']])add(label,o,r,n,'target-validated normalized',cat,evidence+'#Ole; scripts/compare-ole-filter.ps1',normalized);
const t=c.Target;assert.equal(t.PilotSHA256,rh);
add('ShowErrorMessage whole body',t.OriginalVA,t.PilotVA,t.Length,'known difference','executable code',evidence+'#Target','16 validated reference fields; remaining offsets 0xDB/0xDC. No complete body certification.');
add('InitInstance whole body',0x10019e0,sym('?InitInstance@FSXAppPilot@@UAEHXZ'),769,'known difference','executable code',archive+'/initinstance-operands.json; '+archive+'/initinstance-blocks.json','CFG pairing and instruction diagnostics only; no full body certification.',777);
const thunkFile='artifacts/fsx.exe/all-import-thunk-placement41-20261103.json', th=json(thunkFile);assert.equal(th.RebuiltSHA256,rh);assert.equal(th.OriginalSHA256,oh);
for(const x of th.Thunks){add(x.Identity,x.OriginalVA,x.RebuiltVA,6,'target-validated normalized','executable code',thunkFile,'FF25 opcode and unique IAT import identity validated.');if(x.OriginalTrailingCCBytes)add(x.Identity+' trailing CC',num(x.OriginalVA)+6,num(x.RebuiltVA)+6,x.OriginalTrailingCCBytes,'exact raw','padding/unknown zeros',thunkFile);}
const dataFile='artifacts/fsx.exe/writable-data-layout41-20261103.json',d=json(dataFile);assert.equal(d.RebuiltSHA256,rh);assert.equal(d.OriginalSHA256,oh);
for(const x of d.ValidatedRanges){const equal=a.at(num(x.OriginalVA),x.Bytes).equals(b.at(num(x.RebuiltVA),x.Bytes));add(x.Role,x.OriginalVA,x.RebuiltVA,x.Bytes,equal?'exact raw':'target-validated normalized','writable state',dataFile,x.InitialValueCheck+'; initial image including virtual zero-fill only; no runtime equivalence.');}
// Correct category of zero-storage children encountered in DataConstants.
for(const x of claims)if(num(x.original.va)>=0x1004000)x.category='writable state';
const dc=c.DataConstants;add('SafeSEH table',0x1001560,sym('___safe_se_handler_table'),8,'target-validated normalized','executable-section data',evidence+'#DataConstants','Both handler RVAs validated.');
for(const [label,o,r,n]of [['CRT EH4 scope table',0x1002d80,b.u32(num(c.CrtStartup.RebuiltVA)+3),40],['onexit EH4 scope table',0x1002da8,b.u32(num(c.CrtOnexit.Onexit.RebuiltVA)+3),28],['Exception pointer pair',0x10012d0,b.u32(num(c.CrtFailureStack.FailureReporter.RebuiltVA)+0xd5),8],['C++ initializer table',0x1001238,sym('___xc_a'),24],['C initializer table',0x1001250,sym('___xi_a'),12],['Message map',0x10012f0,b.u32(sym('?GetMessageMap@FSXAppPilot@@MBEPBUAFX_MSGMAP@@XZ')+1),8]])add(label,o,r,n,'target-validated normalized','executable-section data',evidence+'; corresponding CRT/static/dispatch comparator',normalized);
const msg=b.u32(sym('?GetMessageMap@FSXAppPilot@@MBEPBUAFX_MSGMAP@@XZ')+1);
add('Empty message map terminator',0x10012d8,b.u32(msg+4),24,'exact raw','executable-section data',evidence+'#Dispatch');
add('Empty RTC initializer sentinels',0x1001568,sym('___rtc_iaa'),16,'exact raw','executable-section data',evidence+'#PreCInit/#PreCppInit');
// Independent target readback through previously paired code entries; pointer equality
// does not promote a near-matched callee to a verified function body.
const av=sym('??_7FSXAppPilot@@6B@');
for(let i=0;i<48;i++){const x=a.u32(0x1001398+4*i),y=b.u32(av+4*i);assert(claims.some(q=>q.category==='executable code'&&q.original&&num(q.original.va)===x&&q.rebuilt&&num(q.rebuilt.va)===y),'Unmapped application slot '+i);add('Application vtable slot '+i,0x1001398+4*i,av+4*i,4,'target-validated normalized','executable-section data','scripts/coverage-baseline41.cjs; '+evidence,'Pointer maps to a paired code entry; callee body status remains separate.');}
add('Application vtable sharing evidence',0x1001398,av,192,'diagnostic-only','executable-section data',evidence+'#VTableAliases','Alias relations alone do not certify bytes; separate slot claims do.');
// Import identities are mapped by identity, not by physical slot index.
for(const x of c.PeImports.OriginalImportEntries){const y=c.PeImports.ImportEntries.find(y=>y.Identity===x.Identity);assert(y);for(const [kind,o,r]of [['IAT',num(x.IatVA),num(y.IatVA)],['ILT',a.base+x.LookupRVA,b.base+y.LookupRVA]])add(kind+' '+x.Identity,o,r,4,'target-validated normalized','imports',evidence+'#PeImports','Identity/hint and initial ILT=IAT validated; entry position may differ (original '+x.Index+', rebuilt '+y.Index+').');}
for(const x of c.PeImports.OriginalImportEntries.filter(x=>x.Kind!=='Ordinal')){const y=c.PeImports.ImportEntries.find(y=>y.Identity===x.Identity);const name=x.Identity.slice(x.Identity.indexOf('!')+1);add('Hint/name '+name,a.base+x.InitialWord,b.base+y.InitialWord,Buffer.byteLength(name)+3,'exact raw','imports',evidence+'#PeImports');}
for(let i=0;i<c.PeImports.ModuleCount;i++){
 const oa=a.base+c.PeImports.OriginalHeader.Directories[1].Address+20*i,rb=b.base+c.PeImports.RebuiltHeader.Directories[1].Address+20*i;
 add('Import descriptor '+i,oa,rb,20,'diagnostic-only','imports',evidence+'#PeImports','Descriptor fields and associated import identities parsed; no raw descriptor equality claimed.');
 const namea=a.base+a.u32(oa+12),nameb=b.base+b.u32(rb+12);let len=0;while(a.at(namea+len,1)[0])len++;add('Import module name '+i,namea,nameb,len+1,'exact raw','imports',evidence+'#PeImports');
 const count=c.PeImports.OriginalImportEntries.filter(x=>x.Module.toLowerCase()===a.at(namea,len).toString().toLowerCase()).length;
 for(const field of [0,16])add('Import terminator '+i+'/'+field,a.base+a.u32(oa+field)+4*count,b.base+b.u32(rb+field)+4*count,4,'exact raw','imports',evidence+'#PeImports');
}
add('Null import descriptor',a.base+c.PeImports.OriginalHeader.Directories[1].Address+20*c.PeImports.ModuleCount,b.base+c.PeImports.RebuiltHeader.Directories[1].Address+20*c.PeImports.ModuleCount,20,'exact raw','imports',evidence+'#PeImports');
add('CodeView RSDS',0x1001518,0x1001518,4,'exact raw','PE/debug metadata',evidence+'#PdbDebug');
add('CodeView GUID',0x100151c,0x100151c,16,'known difference','PE/debug metadata',evidence+'#PdbDebug');
add('CodeView age/path',0x100152c,0x100152c,12,'exact raw','PE/debug metadata',evidence+'#PdbDebug');
add('Debug directory',0x1001260,b.base+c.PeImports.RebuiltHeader.Directories[6].Address,28,'known difference','PE/debug metadata',evidence+'#PeImports; '+evidence+'#PdbDebug','Timestamp differs; directory not certified as raw equal.');
// Model classifications help triage uncovered instructions, never certify their bytes.
const snapshot=json('artifacts/fsx.exe/exe-inventory/ghidra-export.json');
const modeled=[];function collect(x){if(!x||typeof x!=='object')return;if(x.address&&x.length&&x.text)modeled.push([num(x.address),num(x.address)+x.length]);for(const v of Object.values(x))if(typeof v==='object')collect(v);}collect(snapshot);
function partition(side){const image=side==='original'?a:b,out=[];
 for(const sec of image.sections.filter(s=>s.name!=='.rsrc')){const extent=sec.name==='.data'?sec.virtual:Math.max(sec.raw,sec.virtual),end=sec.va+extent;
 const points=new Set([sec.va,end]);for(const x of claims){const q=x[side];if(q&&num(q.va)>=sec.va&&num(q.va)+q.bytes<=end){points.add(num(q.va));points.add(num(q.va)+q.bytes);}}
 const p=[...points].sort((x,y)=>x-y);for(let i=0;i<p.length-1;i++){const lo=p[i],hi=p[i+1],cover=claims.filter(x=>x[side]&&num(x[side].va)<=lo&&num(x[side].va)+x[side].bytes>=hi),statuses=[...new Set(cover.map(x=>x.status))];
 const status=statuses.includes('known difference')?'known difference':statuses.includes('exact raw')?'exact raw':statuses.includes('target-validated normalized')?'target-validated normalized':statuses.includes('diagnostic-only')?'diagnostic-only':'unchecked';
 const bytes=image.at(lo,hi-lo),zeroOrCC=[...bytes].every(x=>x===0||x===0xcc);
 out.push({va:hex(lo),bytes:hi-lo,section:sec.name,status,categories:[...new Set(cover.map(x=>x.category))],evidenceIds:cover.map(x=>x.id),uncoveredByteShape:cover.length?null:zeroOrCC?'only 00/CC; ownership unknown':'contains other bytes',snapshotInstructionOverlap:side==='original'?modeled.some(([s,e])=>s<hi&&e>lo):null});
 }assert.equal(out.filter(x=>x.section===sec.name).reduce((n,x)=>n+x.bytes,0),extent);}
 return out;
}
const original=partition('original'),rebuilt=partition('rebuilt');
const totals=rows=>{const t={};for(const x of rows){const key=x.section+' / '+x.status;t[key]=(t[key]||0)+x.bytes;}return t;};
const categoryTotals=rows=>{const t={};for(const x of rows){assert(x.categories.length<=1,'Mixed categories '+x.va);const key=(x.categories[0]||(x.section==='.data'?'writable state':x.uncoveredByteShape==='only 00/CC; ownership unknown'?'padding/unknown zeros':'unclassified executable-section data'))+' / '+x.status;t[key]=(t[key]||0)+x.bytes;}return t;};
function fileScope(image,header){const rows=[{fileOffset:0,bytes:header.SizeOfHeaders,category:'PE headers',status:'known difference',evidence:evidence+'#PeHeaderSettings/#PeImports',rule:'Selected settings match; full header including timestamps/checksum/resource effects remains different.'}];
 for(const s of image.sections)rows.push({fileOffset:s.offset,bytes:s.raw,va:hex(s.va),category:s.name,status:s.name==='.rsrc'?'excluded':'see VA partition',rule:s.name==='.data'?'Raw file padding beyond virtual size is unchecked. Virtual zero-fill is represented in VA partition.':'Resources excluded from reconstruction; no blanket exclusion of header effects.'});
 const cert=header.Directories[4];if(cert.Size)rows.push({fileOffset:cert.Address,bytes:cert.Size,category:'certificate',status:'unchecked',rule:'Policy unresolved; no cryptographic signature validation.'});
 return {sha256:image===a?oh:rh,fileBytes:image.b.length,ranges:rows};}
const result={schema:1,baseline:41,originalSHA256:oh,rebuiltSHA256:rh,evidenceStatus:'live verified on-disk binary identities; archived same-hash comparator evidence',scope:'All .text raw bytes and .data virtual bytes partitioned independently in both binaries. Unmapped intervals stay unchecked. Claims are paired; partitions retain all overlapping evidence IDs without double counting.',policy:{resources:'excluded from current reconstruction, no match claimed',headers:'selected fields checked in comparison.json; whole header unchecked/different',certificate:'original Security file-offset range retained; equality/signature validity unresolved',metadata:'timestamps/GUID/checksum are not silently normalized or excluded',snapshot:'Historical model classification only, not independent byte evidence',runtime:'No replacement EXE execution; no runtime equivalence claim'},evidenceFiles:[evidence,thunkFile,dataFile,archive+'/fsx.map'].map(p=>({path:p,sha256:hash(p)})),claims,originalPartitions:original,rebuiltPartitions:rebuilt,totals:{original:totals(original),rebuilt:totals(rebuilt)},uncheckedOriginalInstructions:original.filter(x=>x.status==='unchecked'&&x.snapshotInstructionOverlap),headers:{original:c.PeImports.OriginalHeader,rebuilt:c.PeImports.RebuiltHeader},auxiliaryMetadata:{original:c.PeImports.OriginalAuxiliaryMetadata,rebuilt:c.PeImports.RebuiltAuxiliaryMetadata}};
result.categoryTotals={original:categoryTotals(original),rebuilt:categoryTotals(rebuilt)};
result.fileScope={original:fileScope(a,c.PeImports.OriginalHeader),rebuilt:fileScope(b,c.PeImports.RebuiltHeader)};
result.evidenceFiles.push(...['artifacts/fsx.exe/exe-inventory/ghidra-export.json',archive+'/initinstance-operands.json',archive+'/initinstance-blocks.json',archive+'/source-hashes.json','scripts/coverage-baseline41.cjs',...fs.readdirSync('scripts').filter(p=>p.startsWith('compare-')&&p.endsWith('.ps1')).map(p=>'scripts/'+p)].map(p=>({path:p,sha256:hash(p)})));
const instructionBytes=new Set();for(const [start,end]of modeled)for(let va=start;va<end;va++)instructionBytes.add(va);
result.snapshotInstructionUniqueBytes=instructionBytes.size;
assert.equal(instructionBytes.size,5168,'Snapshot changed; reassess triage and report');
fs.writeFileSync('artifacts/fsx.exe/coverage-baseline41-20261103.json',JSON.stringify(result,null,2)+'\n');
const report=['# Baseline 41 consolidated range ledger','', '2026-11-03. Status: live verified on-disk identities; same-hash archived comparison evidence.','',
 'Original SHA256: `'+oh+'`.','Rebuilt SHA256: `'+rh+'`.','',
 'Machine-readable evidence: [coverage-baseline41-20261103.json](../../../artifacts/fsx.exe/coverage-baseline41-20261103.json). Reproduce with `node scripts/coverage-baseline41.cjs`. Each paired claim records both addresses/extents, hashes, evidence and normalization. Independent interval partitions retain every overlapping evidence ID and count each byte once. Source/MAP names are reconstructed.','',
 '## Unique range accounting','', '| Region/status | Original bytes | Rebuilt bytes |','|---|---:|---:|'];
for(const k of new Set([...Object.keys(result.categoryTotals.original),...Object.keys(result.categoryTotals.rebuilt)]))report.push('| '+k+' | '+(result.categoryTotals.original[k]||0)+' | '+(result.categoryTotals.rebuilt[k]||0)+' |');
report.push('', 'These are scoped byte unions, not a whole-EXE match percentage. A whole-body known-difference row marks its entire extent; it does not mean every byte differs. Exact raw initial writable bytes include virtual zero-fill, not runtime state. Target-normalized equality does not establish original addresses or source/compiler identity.', '',
 '## Remaining work and selection','',
 'No unchecked interval overlaps an instruction in the retained Ghidra snapshot. This is a snapshot-based triage result, not proof that the original executable contains no hidden/unmodeled code. All 80 import thunks and their immediate CC runs are included; aliases and prerequisite comparator chains are not added repeatedly. The 18/1114 application count is retained as historical scope, not used as the total.', '',
 'InitInstance (original 0x010019E0, 769 bytes; rebuilt 0x010019C0, 777 bytes) is selected for bounded code work. Its 21 paired CFG blocks and 246 instructions are diagnostic only; 43 nonbranch positions differ. ShowErrorMessage (0x01001D30, 437 bytes) retains two normalized byte differences at 0xDB/0xDC. No substantial unchecked application code range was exposed.', '',
 'Unassigned nonzero executable-section interval: 0x01002DC4..0x01002DE3 (32 bytes, metadata-area data, no snapshot instructions); mapping remains unchecked. Other unassigned .text bytes are 00/CC, with ownership unresolved. Writable unassigned bytes remain 31 original / 47 rebuilt. Do not reinterpret zero runs as absent variables or recovered padding.', '',
 'All 137 import identities/hints are checked. The 100 non-null descriptor bytes are diagnostic-only in this ledger; pointer parsing and import-set equality do not certify physical descriptor bytes. Import names/terminators have explicit raw checks. 103 within-DLL positions still differ. Layout/IAT refinement remains deferred.', '',
 '## Comparison policy and limitations','',
 'The VA partitions cover all 9728 raw .text bytes in each image and .data virtual extents 1184/1200. File-scope rows separately locate headers, raw sections, resources and certificate ranges. Resources are excluded from the current reconstruction task. Header effects are not excluded: selected PE settings pass, but whole headers, checksum, timestamps, original CodeView GUID, certificate/signature and final metadata policy remain unresolved. CodeView signature, age and path bytes are separately checked; the GUID and debug directory remain different. No whole-file identity is claimed.', '',
 'Archived evidence is accepted only for the exact selected EXE hashes. Raw claims and explicit vtable targets are rechecked by this script. Normalized body claims retain the exact scopes of their existing comparators rather than introduce a new masking scheme. No builds are needed for this reconciliation. Current canonical source differs from effective archived source by the documented callback linkage staging; archive/source and source-hashes.json describe compiled input.', '',
 'Read-only Ghidra check: fsx.exe current, api.dll also open; InitInstance revision 1793671788023:335a223f:3, unsaved_changes=false. No tabs, database names/types, source defaults, save or backup changed. No new variable rename is supported by this accounting. Static checks do not establish runtime equivalence; no candidate EXE was executed.', '');
report.push('## Subsequent bounded code result','', 'After this baseline41 ledger was complete, one InitInstance source trial recovered the original five-byte load sequence at 0x01001C5D. The candidate has a different EXE hash and is not silently substituted into this ledger. See [initinstance-matching.md](initinstance-matching.md#bounded-module-instance-local-trial-2026-11-03). Canonical source now contains the accepted local; archive41 remains the immutable coverage baseline.','');
fs.writeFileSync('notes/re/fsx.exe/coverage-baseline41.md',report.join('\n'));
console.log(JSON.stringify({claims:claims.length,totals:result.totals,uncheckedOriginalInstructions:result.uncheckedOriginalInstructions,uncheckedNonzero:original.filter(x=>x.status==='unchecked'&&x.uncoveredByteShape==='contains other bytes')},null,2));

