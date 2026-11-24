// Retain sizes and unresolved references of hand-reconstructed additions; not a match gate.
const fs=require('fs'),path=require('path');const {parseNativeCoff}=require('./lib/coff.cjs');
const run=process.argv[2];if(!run)throw Error('Pass run directory');
const ob=fs.readFileSync(run+'/api-factory.obj'),coff=parseNativeCoff(ob);
const manifest=JSON.parse(fs.readFileSync('artifacts/api.dll/api-source-pass-20261107/manifest.json'));
const specs=[['20134495','_OpenFileEnumerationPilot@16','file_enumeration_open_next.cpp'],['2013468c','_NextFileEnumerationPilot@8','file_enumeration_open_next.cpp'],['201350be','_CopySelectedSuffixPilot@12','suffix_registry.cpp'],['201336ab','_FindSuffixSelectorPilot@8','suffix_registry.cpp'],['20133778','_RegisterSuffixSelectorPilot@8','suffix_registry.cpp'],['20137efe','_DuplicateHeapStringPilot@4','suffix_registry.cpp'],['2012294b','_RunApplicationIdlePilot','message_loop.cpp'],['20122a0b','_ForegroundIdleHookPilot@12','message_loop.cpp'],['20122ae1','?RunApplicationMessageLoopBodyPilot@@YAHXZ','message_loop.cpp']];
for(const [entry,symbol,source] of [['20145110','_OpenAlternateEnumerationSourceBoundaryPilot@8','alternate_enumeration.cpp'],['20144d5d','_CreateAlternateEnumerationSourceBoundaryPilot@12','alternate_enumeration.cpp'],['20144df0','_NextAlternateEnumerationSourceBoundaryPilot@12','alternate_enumeration.cpp'],['20145169','_SplitCabPathSourceBoundaryPilot@12','cab_owner.cpp'],['2014483b','_InitializeCabOwnerSourceBoundaryPilot@8','cab_owner.cpp'],['201448a4','_DestroyCabOwnerSourceBoundaryPilot@4','cab_owner.cpp']]) {
 if((coff.byName.get(symbol)||[]).some(s=>s.section>0)) specs.push([entry,symbol,source]);
}
if((coff.byName.get('_ResolveAlternateEnumerationOwnerPilot@8')||[]).some(s=>s.section>0)) specs.push(['201452e1','_ResolveAlternateEnumerationOwnerPilot@8','cab_owner.cpp']);
for(const [entry,symbol,source] of [['201333ac','_ReleaseCabStreamPilot@4','cab_stream.cpp'],['201333cd','_ReadCabStreamPilot@16','cab_stream.cpp'],['20133592','_SeekCabStreamPilot@12','cab_stream.cpp'],['20135378','_ReadCabStreamTimesPilot@16','cab_stream.cpp'],['2014420b','_LoadCabImagePilot@4','cab_image.cpp']]) {
 if((coff.byName.get(symbol)||[]).some(s=>s.section>0)) specs.push([entry,symbol,source]);
}
const gates={'20122a0b':'foreground-idle-hook-comparison.json','201333ac':'cab-stream-wrappers-comparison.json','201333cd':'cab-stream-wrappers-comparison.json'};
for(const [entry,symbol] of [['201246c4','_OpenCabFileStreamSourcePilot@28'],['20127440','_WrapCabStreamSourcePilot@24'],['20126be0','_InitializeCabWrappedStreamSourcePilot@28'],['20126d82','_DestroyCabWrappedStreamSourcePilot@4']]) {
 if((coff.byName.get(symbol)||[]).some(s=>s.section>0)) specs.push([entry,symbol,'cab_stream_producers.cpp']);
}
for(const [entry,symbol] of [['20124356','_ReadCabFileMethodPilot@16'],['201243c8','_SeekCabFileMethodPilot@20'],['20124408','_StatCabFileMethodPilot@12'],['20126dcf','_ReadCabWrappedMethodPilot@16'],['20126fc0','_SeekCabWrappedMethodPilot@20'],['2012701c','_StatCabWrappedMethodPilot@12']]) {
 if((coff.byName.get(symbol)||[]).some(s=>s.section>0)) specs.push([entry,symbol,'cab_stream_methods.cpp']);
}
for(const entry of ['20124356','201243c8','20126fc0']) gates[entry]='cab-stream-methods-comparison.json';
for(const [entry,symbol] of [['20127dd3','_FindCabCachedBufferPilot@4'],['20127642','_AllocateCabBufferSourcePilot@4'],['2012772b','_ReturnCabBufferPilot@4']]) {
 if((coff.byName.get(symbol)||[]).some(s=>s.section>0)) specs.push([entry,symbol,'cab_buffer_cache.cpp']);
}
const rows=specs.map(([entry,symbol,source])=>{const s=coff.unique(symbol),section=coff.sections[s.section-1];return {entry,symbol,source:'src/api-pilot/'+source,originalModelBytes:manifest.functions.find(f=>f.entry===entry).bodyBytes,nativeBytes:section.size,status:gates[entry]?'compiled; see '+gates[entry]+' for independent acceptance':'compiled_unmatched',references:section.relocations.map(r=>({offset:r.offset,type:r.type,symbol:r.symbol?.name}))};});
fs.writeFileSync(run+'/manual-source-diagnostic.json',JSON.stringify({scope:'Sizes/references only; no match acceptance, dependency implementation or runtime proof',rows},null,2)+'\n');console.log(JSON.stringify(rows.map(({references,...r})=>r),null,2));
