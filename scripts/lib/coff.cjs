// Bounded native x86 COFF reader shared by API object checks. No bigobj/LTCG IR.
const assert=require('assert');
function parseNativeCoff(ob){
 assert(ob.length>=20);assert.equal(ob.readUInt16LE(0),0x14c);assert.equal(ob.readUInt16LE(16),0);
 const st=ob.readUInt32LE(8),nc=ob.readUInt32LE(12),strings=st+18*nc;
 assert(strings+4<=ob.length);const sl=ob.readUInt32LE(strings);assert(sl>=4&&strings+sl<=ob.length);
 function cstr(off,end){assert(off>=strings+4);const z=ob.indexOf(0,off);assert(z>=off&&z<end);return ob.toString('ascii',off,z);}
 function name(p){return ob.readUInt32LE(p)===0?cstr(strings+ob.readUInt32LE(p+4),strings+sl):ob.toString('ascii',p,p+8).replace(/\0.*$/,'');}
 const symbols=new Map(),byName=new Map();
 for(let i=0;i<nc;){const p=st+18*i;assert(p+18<=strings);const s={index:i,name:name(p),value:ob.readUInt32LE(p+8),section:ob.readInt16LE(p+12),type:ob.readUInt16LE(p+14),storage:ob[p+16]};symbols.set(i,s);if(!byName.has(s.name))byName.set(s.name,[]);byName.get(s.name).push(s);const next=i+1+ob[p+17];assert(next<=nc);i=next;}
 const sections=[];
 for(let i=0;i<ob.readUInt16LE(2);i++){const p=20+40*i;assert(p+40<=ob.length);const s={index:i+1,name:ob.toString('ascii',p,p+8).replace(/\0.*$/,''),size:ob.readUInt32LE(p+16),raw:ob.readUInt32LE(p+20),rel:ob.readUInt32LE(p+24),count:ob.readUInt16LE(p+32),flags:ob.readUInt32LE(p+36)};assert(s.raw+s.size<=ob.length);assert(s.rel+s.count*10<=ob.length);s.relocations=[];for(let j=0;j<s.count;j++){const r=s.rel+10*j;s.relocations.push({offset:ob.readUInt32LE(r),symbol:symbols.get(ob.readUInt32LE(r+4)),type:ob.readUInt16LE(r+8)});}sections.push(s);}
 function unique(n){const ss=byName.get(n)||[];assert.equal(ss.length,1,'Ambiguous/missing symbol '+n);return ss[0];}
 return {buffer:ob,sections,symbols,byName,unique};
}
module.exports={parseNativeCoff};
