param([Parameter(Mandatory)][string]$BuildDirectory,[switch]$RequireImportContents,[switch]$RequireExactOrder)
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
$original="$PSScriptRoot\..\original\fsx.exe"
if((Get-FileHash $original).Hash -ne '61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735'){throw 'Original identity differs'}
$a=Open-PE $original;$b=Open-PE "$BuildDirectory\fsx.exe"
function Require($c,[string]$message){if(!$c){throw $message}}
function Header($p){
 $coff=[BitConverter]::ToInt32($p.Bytes,60)+4;$opt=$coff+20;$table=$opt+[BitConverter]::ToUInt16($p.Bytes,$coff+16)
 $names=@('Export','Import','Resource','Exception','Security','BaseRelocation','Debug','Architecture','GlobalPointer','TLS','LoadConfig','BoundImport','IAT','DelayImport','CLR','Reserved')
 Require ([BitConverter]::ToUInt16($p.Bytes,$opt) -eq 0x10b -and [BitConverter]::ToUInt32($p.Bytes,$opt+92) -eq 16) 'Expected PE32 with sixteen directories'
 $dirs=for($i=0;$i -lt 16;$i++){[pscustomobject]@{Index=$i;Name=$names[$i];Address=[long][BitConverter]::ToUInt32($p.Bytes,$opt+96+8*$i);Size=[long][BitConverter]::ToUInt32($p.Bytes,$opt+100+8*$i);AddressKind=$(if($i -eq 4){'FileOffset'}else{'RVA'})}}
 $sections=for($i=0;$i -lt $p.Sections.Count;$i++){$s=$table+40*$i;[pscustomobject]@{Name=[Text.Encoding]::ASCII.GetString($p.Bytes,$s,8).Trim([char]0);RVA=[long][BitConverter]::ToUInt32($p.Bytes,$s+12);VirtualSize=[long][BitConverter]::ToUInt32($p.Bytes,$s+8);RawSize=[long][BitConverter]::ToUInt32($p.Bytes,$s+16);RawOffset=[long][BitConverter]::ToUInt32($p.Bytes,$s+20);Flags=('{0:X8}' -f [BitConverter]::ToUInt32($p.Bytes,$s+36))}}
 [pscustomobject]@{Machine=[BitConverter]::ToUInt16($p.Bytes,$coff);CoffFlags=('{0:X4}' -f [BitConverter]::ToUInt16($p.Bytes,$coff+18));DllFlags=('{0:X4}' -f [BitConverter]::ToUInt16($p.Bytes,$opt+70));ImageBase=('{0:X8}' -f $p.Base);LinkerVersion=('{0}.{1}' -f $p.Bytes[$opt+2],$p.Bytes[$opt+3]);CoffTimestamp=[long][BitConverter]::ToUInt32($p.Bytes,$coff+4);OperatingSystemVersion=('{0}.{1}' -f [BitConverter]::ToUInt16($p.Bytes,$opt+40),[BitConverter]::ToUInt16($p.Bytes,$opt+42));ImageVersion=('{0}.{1}' -f [BitConverter]::ToUInt16($p.Bytes,$opt+44),[BitConverter]::ToUInt16($p.Bytes,$opt+46));SubsystemVersion=('{0}.{1}' -f [BitConverter]::ToUInt16($p.Bytes,$opt+48),[BitConverter]::ToUInt16($p.Bytes,$opt+50));StackReserve=[long][BitConverter]::ToUInt32($p.Bytes,$opt+72);StackCommit=[long][BitConverter]::ToUInt32($p.Bytes,$opt+76);HeapReserve=[long][BitConverter]::ToUInt32($p.Bytes,$opt+80);HeapCommit=[long][BitConverter]::ToUInt32($p.Bytes,$opt+84);EntryRVA=[long][BitConverter]::ToUInt32($p.Bytes,$opt+16);SectionAlignment=[long][BitConverter]::ToUInt32($p.Bytes,$opt+32);FileAlignment=[long][BitConverter]::ToUInt32($p.Bytes,$opt+36);SizeOfImage=[long][BitConverter]::ToUInt32($p.Bytes,$opt+56);SizeOfHeaders=[long][BitConverter]::ToUInt32($p.Bytes,$opt+60);Checksum=[long][BitConverter]::ToUInt32($p.Bytes,$opt+64);Subsystem=[BitConverter]::ToUInt16($p.Bytes,$opt+68);Directories=@($dirs);Sections=@($sections)}
}
function Imports($p,$header){
 $dir=$header.Directories[1];$iat=$header.Directories[12];$modules=@();$allIat=@();$entries=@();$terminators=@()
 Require ($dir.Size -ge 20 -and $dir.Size % 20 -eq 0) 'Import descriptor directory extent invalid'
 for($index=0;$index -lt $dir.Size/20;$index++){
  $descriptor=$p.Base+$dir.Address+20*$index;$raw=Bytes $p $descriptor 20
  if(@($raw|Where-Object {$_ -ne 0}).Count -eq 0){Require ($index -eq $dir.Size/20-1) 'Import null descriptor is not directory terminator';break}
  Require ($index -lt $dir.Size/20-1) 'Import descriptor null terminator missing'
  $oft=U32 $p $descriptor;$ft=U32 $p ($descriptor+16);$dll=CString $p ($p.Base+(U32 $p ($descriptor+12)));$time=U32 $p ($descriptor+4);$forward=U32 $p ($descriptor+8)
  Require ($oft -ne 0 -and $ft -ne 0) 'Expected separate nonzero ILT/IAT'
  $symbols=@();$hints=@();$count=0
  for($i=0;$i -lt 4096;$i++){
   $lookup=U32 $p ($p.Base+$oft+4*$i);$initial=U32 $p ($p.Base+$ft+4*$i)
   Require ($lookup -eq $initial) 'Unbound initial ILT/IAT contents differ'
   if(!$lookup){$terminators+=($p.Base+$ft+4*$i);break}
   if($lookup -band 0x80000000L){Require (($lookup -band 0x7fff0000L) -eq 0) 'Ordinal import has unexpected reserved bits';$symbol='#'+($lookup -band 65535);$hint=$null;$kind='Ordinal'}else{$symbol=CString $p ($p.Base+$lookup+2);$hint=[BitConverter]::ToUInt16((Bytes $p ($p.Base+$lookup) 2),0);$kind='Name'}
   $symbols+=($dll.ToLowerInvariant()+'!'+$symbol);$hints+=$hint;$count++;$allIat+=($p.Base+$ft+4*$i)
   $entries+=[pscustomobject]@{Module=$dll;Index=$i;Identity=$dll.ToLowerInvariant()+'!'+$symbol;Kind=$kind;Hint=$hint;IatVA=('{0:X8}' -f ($p.Base+$ft+4*$i));LookupRVA=$oft+4*$i;InitialWord=$lookup}
  }
  Require ($count -lt 4096) 'Import lookup terminator missing'
  $modules+=[pscustomobject]@{DescriptorIndex=$index;Dll=$dll;Count=$count;Timestamp=$time;ForwarderChain=$forward;ILT=$oft;IAT=$ft;Identities=$symbols;Hints=$hints}
 }
 Require ($allIat.Count+$terminators.Count -eq $iat.Size/4) 'IAT directory does not cover all entries and module terminators'
 $covered=@($allIat+$terminators|Sort-Object);for($i=0;$i -lt $covered.Count;$i++){Require ($covered[$i] -eq $p.Base+$iat.Address+4*$i) 'IAT directory is not the complete contiguous import range'}
 [pscustomobject]@{Modules=$modules;Entries=$entries;Count=$entries.Count;ModuleCount=$modules.Count;IatTerminatorCount=$terminators.Count;IatBytes=$iat.Size;DescriptorBytes=$dir.Size;InitialIltEqualsIat=$true;TerminatorAndDirectoryCoverageVerified=$true}
}
$ah=Header $a;$bh=Header $b;$ai=Imports $a $ah;$bi=Imports $b $bh
function Auxiliary-Metadata($p,$header){
 $debug=$header.Directories[6];$records=@()
 if($debug.Size){Require ($debug.Size % 28 -eq 0) 'Debug directory extent invalid';for($i=0;$i -lt $debug.Size/28;$i++){$d=$p.Base+$debug.Address+28*$i;$type=U32 $p ($d+12);$size=U32 $p ($d+16);$rva=U32 $p ($d+20);$file=U32 $p ($d+24);$path=$null;$age=$null;$guid=$null;$signature=$null;Require ($file+$size -le $p.Bytes.Length) 'Debug raw range exceeds file';if($size){$mapped=Bytes $p ($p.Base+$rva) $size;$raw=[byte[]]$p.Bytes[$file..($file+$size-1)];Require ([Convert]::ToBase64String($mapped) -ceq [Convert]::ToBase64String($raw)) 'Debug raw file pointer differs from mapped data';if($type -eq 2 -and $size -ge 25){$signature=[Text.Encoding]::ASCII.GetString($raw,0,4);if($signature -ceq 'RSDS'){$guid=[Convert]::ToHexString($raw[4..19]);$age=[BitConverter]::ToUInt32($raw,20);$path=CString $p ($p.Base+$rva+24)}}};$records+=[pscustomobject]@{Type=$type;Bytes=$size;Timestamp=(U32 $p ($d+4));DataRVA=('{0:X8}' -f $rva);FileOffset=$file;Signature=$signature;GuidRawBytes=$guid;Age=$age;PdbPath=$path}}}
 $security=$header.Directories[4];$certificates=@()
 if($security.Size){Require ($security.Address % 8 -eq 0 -and $security.Address+$security.Size -le $p.Bytes.Length) 'Certificate file range invalid';for($offset=0;$offset -lt $security.Size;){$f=$security.Address+$offset;Require ($offset+8 -le $security.Size) 'Certificate header truncated';$length=[BitConverter]::ToUInt32($p.Bytes,$f);Require ($length -ge 8 -and $offset+$length -le $security.Size) 'Certificate record extent invalid';$certificates+=[pscustomobject]@{FileOffset=$f;Length=$length;Revision=('{0:X4}' -f [BitConverter]::ToUInt16($p.Bytes,$f+4));Type=[BitConverter]::ToUInt16($p.Bytes,$f+6)};$offset+=[long]([Math]::Ceiling($length/8.0)*8)};Require ($offset -eq $security.Size) 'Certificate directory extent differs from aligned records'}
 [pscustomobject]@{DebugRecords=$records;CertificateRecords=$certificates;CertificateBytes=$security.Size;CertificateEndsAtEndOfFile=($security.Size -gt 0 -and $security.Address+$security.Size -eq $p.Bytes.Length);CertificateSignatureCryptographicallyVerified=$false}
}
$oldAux=Auxiliary-Metadata $a $ah;$newAux=Auxiliary-Metadata $b $bh
$missing=@(Compare-Object @($ai.Entries.Identity|Sort-Object) @($bi.Entries.Identity|Sort-Object)|Where-Object SideIndicator -EQ '<='|ForEach-Object InputObject)
$extra=@(Compare-Object @($ai.Entries.Identity|Sort-Object) @($bi.Entries.Identity|Sort-Object)|Where-Object SideIndicator -EQ '=>'|ForEach-Object InputObject)
$differences=@();$contentDifferences=@();$moduleRows=for($i=0;$i -lt [Math]::Min($ai.ModuleCount,$bi.ModuleCount);$i++){
 $x=$ai.Modules[$i];$y=$bi.Modules[$i];$equal=($x.Dll -ceq $y.Dll -and $x.Count -eq $y.Count -and $x.Timestamp -eq $y.Timestamp -and $x.ForwarderChain -eq $y.ForwarderChain -and ($x.Identities|ConvertTo-Json -Compress) -ceq ($y.Identities|ConvertTo-Json -Compress) -and ($x.Hints|ConvertTo-Json -Compress) -ceq ($y.Hints|ConvertTo-Json -Compress))
 $oldContent=@($ai.Entries|Where-Object Module -CEQ $x.Dll|Sort-Object Identity|Select-Object Identity,Kind,Hint);$newContent=@($bi.Entries|Where-Object Module -CEQ $y.Dll|Sort-Object Identity|Select-Object Identity,Kind,Hint)
 $contentEqual=($x.Dll -ceq $y.Dll -and $x.Count -eq $y.Count -and $x.Timestamp -eq $y.Timestamp -and $x.ForwarderChain -eq $y.ForwarderChain -and ($oldContent|ConvertTo-Json -Compress) -ceq ($newContent|ConvertTo-Json -Compress))
 if(!$equal){$differences+=$x.Dll};if(!$contentEqual){$contentDifferences+=$x.Dll};[pscustomobject]@{DescriptorIndex=$i;Dll=$x.Dll;Entries=$x.Count;ContentsWithHintsAndDescriptorFieldsMatch=$contentEqual;OrderedIdentityHintAndDescriptorFieldsMatch=$equal;OriginalIatRVA=$x.IAT;RebuiltIatRVA=$y.IAT;RelativeIatOffsetMatches=($x.IAT-$ah.Directories[12].Address -eq $y.IAT-$bh.Directories[12].Address)}
}
$match=(!$missing.Count -and !$extra.Count -and !$differences.Count -and $ai.ModuleCount -eq $bi.ModuleCount -and $ai.IatBytes -eq $bi.IatBytes -and $ai.DescriptorBytes -eq $bi.DescriptorBytes -and @($moduleRows|Where-Object {!$_.RelativeIatOffsetMatches}).Count -eq 0)
$contentMatch=(!$missing.Count -and !$extra.Count -and !$contentDifferences.Count -and $ai.ModuleCount -eq $bi.ModuleCount -and $ai.IatBytes -eq $bi.IatBytes -and $ai.DescriptorBytes -eq $bi.DescriptorBytes -and @($moduleRows|Where-Object {!$_.RelativeIatOffsetMatches}).Count -eq 0)
if($RequireImportContents){Require $contentMatch 'Import identity/hint/descriptor/module-IAT shape differs'}
if($RequireExactOrder){Require $match 'Import entry order differs'}
$directoryRows=for($i=0;$i -lt 16;$i++){$x=$ah.Directories[$i];$y=$bh.Directories[$i];[pscustomobject]@{Name=$x.Name;Index=$i;AddressKind=$x.AddressKind;OriginalAddress=('{0:X8}' -f $x.Address);RebuiltAddress=('{0:X8}' -f $y.Address);OriginalSize=$x.Size;RebuiltSize=$y.Size;RawFieldsMatch=($x.Address -eq $y.Address -and $x.Size -eq $y.Size);Classification=$(switch($i){1{'Import contents checked; placement separate'}2{'Original resource present; rebuild absent'}4{'Original certificate file range present; rebuild absent'}6{'Debug metadata/placement not matched'}10{'Load config contents checked by separate gate'}12{'Complete IAT contents/order shape checked; placement separate'}default{if(!$x.Address -and !$y.Address -and !$x.Size -and !$y.Size){'Absent in both'}else{'Open'}}})}}
[pscustomobject]@{OriginalSHA256=(Get-FileHash $original).Hash;RebuiltSHA256=(Get-FileHash "$BuildDirectory\fsx.exe").Hash;ImportCount=$ai.Count;RebuiltImportCount=$bi.Count;ModuleCount=$ai.ModuleCount;MissingImports=$missing;ExtraImports=$extra;ModuleComparisons=@($moduleRows);ImportContentsMatch=$contentMatch;ImportContentsAndOrderMatch=$match;IatBytes=$ai.IatBytes;DescriptorBytes=$ai.DescriptorBytes;InitialIltEqualsIat=$true;TerminatorsAndCoverageVerified=$true;OriginalImportEntries=$ai.Entries;ImportEntries=$bi.Entries;Directories=@($directoryRows);OriginalHeader=$ah;RebuiltHeader=$bh;OriginalAuxiliaryMetadata=$oldAux;RebuiltAuxiliaryMetadata=$newAux;Scope='Complete ordinary imports: DLL spelling/order, ordered identities/name-hints, ordinal values, descriptor timestamp/forwarder fields, initial ILT/IAT equality, module/null terminators and IAT coverage. Directory/header/section differences are inventoried, not normalized away or claimed matched. Resource/certificate/debug data are not reproduced by this gate; no whole EXE/runtime/source identity claim.'}|ConvertTo-Json -Depth 10
