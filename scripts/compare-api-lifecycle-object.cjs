// Baseline-specific COFF relocation verification, separate from a linked DLL gate.
const fs=require('fs'),path=require('path'),crypto=require('crypto'),assert=require('assert');
process.chdir(path.resolve(__dirname,'..'));
const run=process.argv[2]||'vm-share/modules/api/runs/api-lifecycle-object-20261103-001';
const read=p=>fs.readFileSync(p),sha=p=>crypto.createHash('sha256').update(read(p)).digest('hex').toUpperCase(),hex=n=>(n>>>0).toString(16).padStart(8,'0').toUpperCase();
const original='original/api.dll';assert.equal(sha(original),'9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003');
const pe=read(original),ph=pe.readUInt32LE(60),opt=ph+24,base=pe.readUInt32LE(opt+28),ps=[];
for(let i=0;i<pe.readUInt16LE(ph+6);i++){const p=opt+pe.readUInt16LE(ph+20)+40*i;ps.push({va:base+pe.readUInt32LE(p+12),size:pe.readUInt32LE(p+16),raw:pe.readUInt32LE(p+20)});}
function bytes(va,n){const s=ps.find(s=>va>=s.va&&va+n<=s.va+s.size);assert(s,'Original range unmapped');return pe.subarray(s.raw+va-s.va,s.raw+va-s.va+n);}
const objectPaths=['api-factory.obj','api-lifecycle.obj'].map(n=>run+'/'+n).filter(p=>fs.existsSync(p));assert.equal(objectPaths.length,1,'Choose an unambiguous object run');const objectPath=objectPaths[0];
const ob=read(objectPath);assert.equal(ob.readUInt16LE(0),0x14c);assert.equal(ob.readUInt16LE(16),0,'Native COFF required');
const {sections,symbols,byName,unique}=require('./lib/coff.cjs').parseNativeCoff(ob);
function member(name){return byName.has(name)?name:name.replace('ApplicationBoundaryPilot@@QAE','ApplicationBoundaryPilot@@UAE');}
const wrapper='?InvokeApplicationCallbackBoundaryPilot@@YGHP6AHXZ@Z',shutdown=member('?Shutdown@ApplicationBoundaryPilot@@QAEHXZ'),loop=member('?RunMessageLoop@ApplicationBoundaryPilot@@QAEHP6GPAUtagMSG@@XZ@Z'),table='__sehtable$'+wrapper;
const targets=new Map([[wrapper,0x2013e3c1],[table,0x201f0808],['__except_handler4',0x20148e1d],['___security_cookie',0x201f6284],['?FilterApplicationExceptionPilot@@YGHKPAU_EXCEPTION_POINTERS@@@Z',0x2013e310],['?ShutdownApplicationBodyPilot@@YAHXZ',0x20120ed5],['?RunApplicationMessageLoopBodyPilot@@YAHXZ',0x20122ae1],['?g_currentMessageCallbackPilot@@3P6GPAUtagMSG@@XZA',0x2020de7c]]);
for(const [name,va,n]of [['?FdiReadStreamBoundaryPilot@@YAIHPAXI@Z',0x2014474c,33],['?FdiWriteStreamBoundaryPilot@@YAIHPAXI@Z',0x2014476d,33],['?FdiCloseStreamBoundaryPilot@@YAHH@Z',0x2014478e,23],['?FdiSeekStreamBoundaryPilot@@YAJHJH@Z',0x201447a5,51]])targets.set(name,va);
const ws=unique(wrapper);assert.equal(ws.value,0);const checks=[];
function verify(symbolName,originalVA,expectedSize,expectedRelocations,code=true,mutation=null){
 const sym=unique(symbolName),s=sections[sym.section-1];assert(s);assert.equal(sym.value,0,'Expected whole COMDAT');assert.equal(s.size,expectedSize,'Unexpected whole-section extent');assert.equal(s.relocations.length,expectedRelocations);assert.equal(Boolean(s.flags&0x20),code);
 const logical=Buffer.from(ob.subarray(s.raw,s.raw+s.size)),originalBytes=bytes(originalVA,expectedSize),fields=[];if(mutation)mutation(logical,s);
 const occupied=new Set();
 for(const r of s.relocations){assert(r.symbol,'Missing relocation symbol');assert([6,20].includes(r.type),'Only DIR32/REL32 supported');assert(r.offset+4<=s.size);for(let k=0;k<4;k++){assert(!occupied.has(r.offset+k));occupied.add(r.offset+k);}
  let target=targets.get(r.symbol.name);if(target===undefined&&r.symbol.section===ws.section)target=0x2013e3c1+r.symbol.value;
  assert(target!==undefined,'Unmapped relocation '+r.symbol.name);const addend=logical.readInt32LE(r.offset),resolved=(target+addend)>>>0;
  const observed=r.type===6?originalBytes.readUInt32LE(r.offset):(originalVA+r.offset+4+originalBytes.readInt32LE(r.offset))>>>0;
  assert.equal(observed,resolved,'Reference target/addend differs at '+r.offset);
  // Resolve into a private buffer only. Original/object files are never patched.
  if(r.type===6)logical.writeUInt32LE(resolved,r.offset);else logical.writeInt32LE((resolved-(originalVA+r.offset+4))|0,r.offset);
  fields.push({offset:r.offset,coffType:r.type===6?'DIR32':'REL32',symbol:r.symbol.name,addend,originalTarget:hex(observed),targetScope:r.symbol.section===0?'external dependency; implementation not certified':'paired object section/label'});
 }
 assert(logical.equals(originalBytes),'Non-relocation bytes differ for '+symbolName);
 return {symbol:symbolName,originalVA:hex(originalVA),objectSection:s.index,objectSectionName:s.name,bytes:expectedSize,status:'COFF relocation-validated body equality',fields};
}
checks.push(verify(wrapper,0x2013e3c1,137,4));checks.push(verify(shutdown,0x2012052a,11,2));checks.push(verify(loop,0x20122817,29,3));checks.push(verify(table,0x201f0808,28,2,false));
const streams=[['?FdiReadStreamBoundaryPilot@@YAIHPAXI@Z',0x2014474c,33],['?FdiWriteStreamBoundaryPilot@@YAIHPAXI@Z',0x2014476d,33],['?FdiCloseStreamBoundaryPilot@@YAHH@Z',0x2014478e,23],['?FdiSeekStreamBoundaryPilot@@YAJHJH@Z',0x201447a5,51]];
const hasStreams=byName.has(streams[0][0]);
if(hasStreams)for(const [name,va,n]of streams){const result=verify(name,va,n,0);result.status='exact raw native COFF body equality';checks.push(result);}
const initName=member('?Initialize@ApplicationBoundaryPilot@@QAEHPAUHINSTANCE__@@IHPAX1@Z');
let initializeDiagnostic=null;
function compareInitialize(mutate=null){
 if(!byName.has(initName))return null;
 for(const [n,v]of Object.entries({'?g_applicationInstanceBoundaryPilot@@3PAUHINSTANCE__@@A':0x2020de68,'?g_startupArgumentBoundaryPilot@@3IA':0x2020de6c,'?g_startupModeBoundaryPilot@@3HA':0x201f9500,'?g_errorCallbackBoundaryPilot@@3PAXA':0x2020de78,'?g_exceptionReportingEnabledBoundaryPilot@@3HA':0x2020e0c0,'?g_singleInstanceEventBoundaryPilot@@3PAXA':0x2020de80,'?g_fdiErrorBoundaryPilot@@3UERF@@A':0x2020d8d0,'?g_fdiContextBoundaryPilot@@3PAXA':0x2020d8dc,'?FdiAllocateBoundaryPilot@@YAPAXK@Z':0x20144576,'?FdiFreeBoundaryPilot@@YAXPAX@Z':0x200f40a9,'?FdiOpenBoundaryPilot@@YAHPADHH@Z':0x201445fe,'_FDICreate':0x201493a1,'?InitializeApplicationBodyBoundaryPilot@@YAHXZ':0x20120869,'?OnApplicationEventBoundaryPilot@@YAXKK@Z':0x201355d3,'?g_initializeCallbackRegistryBoundaryPilot@@3VCallbackRegistryOwnerBoundaryPilot@@A':0x2020bb48,'?Register@CallbackRegistryOwnerBoundaryPilot@@QAEHP6AXKK@ZHKK@Z':0x200f6451}))targets.set(n,v);
 const sym=unique(initName),s=sections[sym.section-1];assert.equal(sym.value,0);assert.equal(s.size,158,'Native Initialize extent changed');assert.equal(s.relocations.length,21);
 const logical=Buffer.from(ob.subarray(s.raw,s.raw+s.size)),originalBytes=bytes(0x2012048a,160),fields=[];
 if(mutate)mutate(logical,s);
 for(const r of s.relocations){assert([6,20].includes(r.type));const target=targets.get(r.symbol.name);assert(target!==undefined,'Unmapped Initialize reference '+r.symbol.name);const originalOffset=r.offset<0x7c?r.offset:r.offset+1;const addend=logical.readInt32LE(r.offset);const actual=r.type===6?originalBytes.readUInt32LE(originalOffset):(0x2012048a+originalOffset+4+originalBytes.readInt32LE(originalOffset))>>>0;assert.equal(actual,(target+addend)>>>0);originalBytes.copy(logical,r.offset,originalOffset,originalOffset+4);fields.push({objectOffset:r.offset,originalOffset,symbol:r.symbol.name,coffType:r.type===6?'DIR32':'REL32',originalTarget:hex(actual),addend});}
 assert(logical.subarray(0,14).equals(originalBytes.subarray(0,14)));
 assert(logical.subarray(15,124).equals(originalBytes.subarray(15,124)));
 assert.equal(originalBytes[14],0x57);assert.equal(logical[14],0x56);
 assert.equal(originalBytes[0x82],0x56);assert.equal(originalBytes[0x8e],0xbe);assert.equal(logical[0x8d],0xb9);
 return {symbol:initName,originalVA:'2012048A',originalBytes:160,objectSection:s.index,objectBytes:158,wholeBodyMatch:false,fields,verifiedSpans:[{originalVA:'2012048A',objectOffset:0,bytes:14,status:'exact raw'},{originalVA:'20120499',objectOffset:15,bytes:109,status:'target-validated object equality'}],unverifiedOriginalBytes:37,unverifiedObjectBytes:35,residual:'Original saves EDI for result, passes registry owner via ESI and locally preserves ESI around registration; native uses ESI for result and ECX for thiscall owner, omitting that extra PUSH/POP pair. Branch length/content and remaining tail not certified. Full function/ABI not matched.'};
}
initializeDiagnostic=compareInitialize();
const factoryName='_GetApplication',vtableName='??_7ApplicationBoundaryPilot@@6B@',windowName=member('?GetWindowHandle@ApplicationBoundaryPilot@@QAEPAUHWND__@@H@Z');
const hasFactory=byName.has(factoryName);
if(hasFactory){
 for(const [name,va]of [[initName,0x2012048a],[shutdown,0x2012052a],[loop,0x20122817],[windowName,0x20120535],[vtableName,0x200f2b24],['?g_applicationBoundaryPilot@@3PAVApplicationBoundaryPilot@@A',0x2020de64],['?g_factoryZeroInitializeBoundaryPilot@@3HA',0x2020d8ec],['?InitializeHeapManagerPilot@@YAHXZ',0x2013623e],['?AllocatePrivateHeapMemoryPilot@@YGHHKPAPAXH@Z',0x20135dd9],['?g_mainWindowHandlePilot@@3PAUHWND__@@A',0x2020e010]])targets.set(name,va);
 checks.push(verify(factoryName,0x20120442,72,7));checks.push(verify(windowName,0x20120535,21,1));checks.push(verify(vtableName,0x200f2b24,16,4,false));
 const vs=sections[unique(vtableName).section-1];assert.equal(vs.name,'.rdata');
 for(const [i,name]of [initName,shutdown,loop,windowName].entries()){const r=vs.relocations[i];assert.equal(r.offset,i*4);assert.equal(r.symbol.name,name);assert(r.symbol.section>0,'VTable member must be defined');assert(sections[r.symbol.section-1].flags&0x20);}
 assert.equal(bytes(0x200f2b20,4).readUInt32LE(),0x0073255c,'Original string-prefix observation changed');assert.equal(bytes(0x2020de64,4).readUInt32LE(),0);assert.equal(bytes(0x2020d8ec,4).readUInt32LE(),0);
 const e=base+pe.readUInt32LE(opt+96),ordinalBase=bytes(e+16,4).readUInt32LE(),nameCount=bytes(e+24,4).readUInt32LE(),functionsVA=base+bytes(e+28,4).readUInt32LE(),namesVA=base+bytes(e+32,4).readUInt32LE(),ordinalsVA=base+bytes(e+36,4).readUInt32LE();
 let found=0;for(let i=0;i<nameCount;i++){if(string(base+bytes(namesVA+4*i,4).readUInt32LE())!=='GetApplication')continue;const index=bytes(ordinalsVA+2*i,2).readUInt16LE();assert.equal(ordinalBase+index,144);assert.equal(base+bytes(functionsVA+4*index,4).readUInt32LE(),0x20120442);found++;}assert.equal(found,1);
}
// Separate evidence for the original EH4 bridge identity, not its callee behavior.
const bridge=bytes(0x20148e1d,35);assert.equal(bridge.subarray(0,16).toString('hex'),'ff742410ff742410ff742410ff742410');assert.equal(bridge[16],0x68);assert.equal(bridge.readUInt32LE(17),0x20148b71);assert.equal(bridge[21],0x68);assert.equal(bridge.readUInt32LE(22),0x201f6284);assert.equal(bridge[26],0xe8);assert.equal((0x20148e1d+31+bridge.readInt32LE(27))>>>0,0x20149133);assert.equal(bridge.subarray(31).toString('hex'),'83c418c3');assert.equal(bytes(0x20149133,2).toString('hex'),'ff25');
function string(va){const chars=[];for(let i=0;i<512;i++){const c=bytes(va+i,1)[0];if(!c)return Buffer.from(chars).toString('ascii');chars.push(c);}throw Error('Unterminated original string');}
const commonSlot=bytes(0x20149133+2,4).readUInt32LE();let commonIdentity=null;
for(let d=base+pe.readUInt32LE(opt+104);bytes(d+12,4).readUInt32LE();d+=20){const ilt=base+bytes(d,4).readUInt32LE(),iat=base+bytes(d+16,4).readUInt32LE(),dll=string(base+bytes(d+12,4).readUInt32LE()).toLowerCase();for(let i=0;;i++){const v=bytes(ilt+4*i,4).readUInt32LE();if(!v)break;if(iat+4*i===commonSlot)commonIdentity=dll+'!'+((v&0x80000000)?'#'+(v&65535):string(base+v+2));}}
assert.equal(commonIdentity,'msvcr80.dll!_except_handler4_common');
for(const [i,target]of [0x2012048a,0x2012052a,0x20122817,0x20120535].entries())assert.equal(bytes(0x200f2b24+i*4,4).readUInt32LE(),target);
assert.equal(bytes(0x2020de7c,4).readUInt32LE(),0,'Original callback slot initial state differs');
function rejects(action){let failed=false;try{action();}catch{failed=true;}assert(failed,'Negative control was accepted');}
rejects(()=>verify(loop,0x20122817,29,3,true,b=>{b[0]^=1;}));
rejects(()=>verify(loop,0x20122817,29,3,true,(b,s)=>{b.writeUInt32LE(4,s.relocations[0].offset);}));
const previous=targets.get('?ShutdownApplicationBodyPilot@@YAHXZ');targets.set('?ShutdownApplicationBodyPilot@@YAHXZ',previous+1);rejects(()=>verify(shutdown,0x2012052a,11,2));targets.set('?ShutdownApplicationBodyPilot@@YAHXZ',previous);
if(hasStreams)rejects(()=>verify(streams[0][0],0x2014474c,33,0,true,b=>{b[0x19]=0x10;}));
const report={originalSHA256:sha(original),objectSHA256:sha(objectPath),compilerCandidate:'VS2005 SP1 .762, native O2/Oy-/hotpatch/GS/MD/Gy'+(hasFactory?'/GR-':''),checks,uniqueOriginalCodeBytes:177,metadataBytes:28,negativeControls:['changed opcode rejected','relocation addend changed rejected','wrong callback target rejected'],scope:'Complete native object COMDAT code/metadata compared with original linked bytes after symbol/label/addend validation. Includes wrapper normal path90 plus detached EH47. External filter, callbacks, cookie helper, globals and final linker resolution remain dependencies; this is not a newly linked DLL match or runtime proof. No original class/type/name/full-size claim.'};
if(hasStreams){report.uniqueOriginalCodeBytes+=140;report.rawStreamCallbackBytes=140;report.negativeControls.push('read-vtable slot changed to write slot rejected');}
if(initializeDiagnostic){report.initializeDiagnostic=initializeDiagnostic;rejects(()=>compareInitialize(b=>{b[0]^=1;}));rejects(()=>compareInitialize((b,s)=>{b.writeUInt32LE(4,s.relocations[0].offset);}));report.negativeControls.push('Initialize prefix opcode mutation rejected','Initialize reference addend mutation rejected');}
if(hasFactory){
 rejects(()=>verify(factoryName,0x20120442,72,7,true,b=>{b[0x1f]=8;}));
 targets.set(vtableName,0x200f2b28);rejects(()=>verify(factoryName,0x20120442,72,7));targets.set(vtableName,0x200f2b24);
 targets.set(shutdown,0x20122817);rejects(()=>verify(vtableName,0x200f2b24,16,4,false));targets.set(shutdown,0x2012052a);
 report.uniqueOriginalCodeBytes+=93;report.metadataBytes+=16;report.factory={originalVA:'20120442',bytes:72,status:'complete COFF relocation-validated body equality',vtableOriginalVA:'200F2B24',vtableBytes:16,memberBodiesDefined:true,initializeMemberWholeMatch:false,originalZeroGlobalValuesChecked:true,candidateGlobalsDefined:false,linkedExport144Matched:false,scope:'Original export144 association is independently checked; COFF symbol _GetApplication uses chosen zero-argument cdecl model. Object globals/dependencies remain unresolved, no linked export table/runtime match. Window21 overlaps previously linked evidence and must not be added twice in an original-address coverage union.'};
 report.negativeControls.push('factory allocation size4 changed to8 rejected','factory wrong VTable pointer rejected','VTable shutdown-to-loop substitution rejected');
}
fs.writeFileSync(run+'/lifecycle-comparison.json',JSON.stringify(report,null,2)+'\n');
console.log(JSON.stringify({objectSHA256:report.objectSHA256,uniqueOriginalCodeBytes:report.uniqueOriginalCodeBytes,metadataBytes:report.metadataBytes,negativeControls:report.negativeControls,scope:report.scope},null,2));
