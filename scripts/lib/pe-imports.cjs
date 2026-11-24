// Read named/ordinal ordinary PE32 imports directly from pinned evidence bytes.
const assert=require('assert');
function readPE32Imports(b){
 const pe=b.readUInt32LE(60),opt=pe+24;assert.equal(b.readUInt16LE(opt),0x10b);
 const base=b.readUInt32LE(opt+28),sections=[];
 for(let i=0;i<b.readUInt16LE(pe+6);i++){const p=opt+b.readUInt16LE(pe+20)+40*i;sections.push({rva:b.readUInt32LE(p+12),raw:b.readUInt32LE(p+20),size:b.readUInt32LE(p+16)});}
 function raw(rva,n){const s=sections.find(s=>rva>=s.rva&&rva+n<=s.rva+s.size);assert(s,'Import RVA unmapped');const o=s.raw+rva-s.rva;assert(o+n<=b.length);return o;}
 function string(rva){const o=raw(rva,1),end=b.indexOf(0,o);assert(end>=o);raw(rva,end-o+1);return b.toString('ascii',o,end);}
 const start=b.readUInt32LE(opt+104),size=b.readUInt32LE(opt+108),map=new Map();
 if(!start)return map;
 for(let i=0;i+20<=size;i+=20){const d=raw(start+i,20),first=b.readUInt32LE(d+16);if(!first)break;const dll=string(b.readUInt32LE(d+12)).toLowerCase(),lookup=b.readUInt32LE(d)||first;
  for(let n=0;;n++){const value=b.readUInt32LE(raw(lookup+n*4,4));if(!value)break;const name=value&0x80000000?'#'+(value&0xffff):string(value+2);const va=base+first+n*4;assert(!map.has(va));map.set(va,{dll,name});}
 }
 return map;
}
module.exports={readPE32Imports};
