param([Parameter(Mandatory)][string]$BuildDirectory)
# Compare non-branch instructions in paired-block order. Known reference fields
# are normalized only after target validation. Scheduling differences stay visible.
$ErrorActionPreference='Stop'
$project=[IO.Path]::GetFullPath("$PSScriptRoot\..")
$cfg=Get-Content "$BuildDirectory\initinstance-blocks.json" -Raw|ConvertFrom-Json
$diag=Get-Content "$BuildDirectory\initinstance-diagnostic.json" -Raw|ConvertFrom-Json
$refs=(Get-Content "$BuildDirectory\initinstance-references.json" -Raw|ConvertFrom-Json).References
$map=Get-Content "$BuildDirectory\fsx.map" -Raw
function Open-PE([string]$path){
 $b=[IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $path));$pe=[BitConverter]::ToInt32($b,60);$opt=$pe+24;$table=$opt+[BitConverter]::ToUInt16($b,$pe+20);$sections=@()
 for($i=0;$i -lt [BitConverter]::ToUInt16($b,$pe+6);$i++){$s=$table+$i*40;$sections+=,@([BitConverter]::ToUInt32($b,$s+12),[BitConverter]::ToUInt32($b,$s+16),[BitConverter]::ToUInt32($b,$s+20))}
 return @{Bytes=$b;Base=[BitConverter]::ToUInt32($b,$opt+28);Sections=$sections}
}
function Read-Bytes($pe,[long]$address,[int]$length){
 $rva=$address-$pe.Base
 foreach($section in $pe.Sections){if($rva -ge $section[0] -and $rva+$length -le $section[0]+$section[1]){$o=$section[2]+$rva-$section[0];return ,([byte[]]$pe.Bytes[$o..($o+$length-1)])}}
 throw ('Unmapped PE range {0:X}' -f $address)
}
function Symbol([string]$name){$m=[regex]::Match($map,'(?m)^\s*\S+\s+'+[regex]::Escape($name)+'\s+([0-9a-fA-F]{8})\b');if(!$m.Success){throw "Missing symbol $name"};return [Convert]::ToUInt32($m.Groups[1].Value,16)}
$old=Open-PE "$project\original\fsx.exe";$new=Open-PE "$BuildDirectory\fsx.exe"
$targets=@{}
foreach($ref in $refs){$targets[[long][Convert]::ToUInt32($ref.OriginalTarget,16)]=[long][Convert]::ToUInt32($ref.RebuiltTarget,16)}
$targets[0x01004004L]=[long](Symbol '___security_cookie')
$targets[0x01004388L]=[long](Symbol '?g_mainWindowPilot@@3VCWnd@@A')
$targets[0x01004400L]=[long](Symbol '?g_fsxAppPilot@@3VFSXAppPilot@@A')+0x20
$strings=@{0x01001304L='language.dll';0x01001314L='D3DX9_34.DLL';0x01001324L='D3DXCheckVersion';0x01001338L='msxml4.manifest';0x01001348L='API.DLL';0x01001350L='GetApplication';0x01001360L='Unable to install message filter';0x010014c8L=''}
function Sizes($blocks,[long]$end){
 $all=@($blocks.Instructions|Sort-Object {[Convert]::ToUInt32($_.address,16)});$sizes=@{}
 for($i=0;$i -lt $all.Count;$i++){$va=[Convert]::ToUInt32($all[$i].address,16);$next=if($i+1 -lt $all.Count){[Convert]::ToUInt32($all[$i+1].address,16)}else{$end};$sizes[$all[$i].address.ToLowerInvariant()]=$next-$va}
 return $sizes
}
$os=Sizes $cfg.Original (0x010019e0L+769);$ns=Sizes $cfg.Rebuilt ([Convert]::ToUInt32($diag.RebuiltVA,16)+$diag.RebuiltLength)
$rows=@();$verified=0
foreach($pair in $cfg.EdgeChecks){
 $left=@(($cfg.Original|Where-Object Start -EQ $pair.Original).Instructions|Where-Object instruction -NotMatch '(?i)^j')
 $right=@(($cfg.Rebuilt|Where-Object Start -EQ $pair.Rebuilt).Instructions|Where-Object instruction -NotMatch '(?i)^j')
 if($left.Count -ne $right.Count){throw 'Non-branch instruction counts differ; positional comparison is not applicable'}
 for($i=0;$i -lt $left.Count;$i++){
  $a=$left[$i];$b=$right[$i];$va=[Convert]::ToUInt32($a.address,16);$vb=[Convert]::ToUInt32($b.address,16)
  $ab=Read-Bytes $old $va $os[$a.address.ToLowerInvariant()];$bb=Read-Bytes $new $vb $ns[$b.address.ToLowerInvariant()];$normalized=[byte[]]$bb.Clone();$checked=@();$unresolved=@()
  if($ab.Length -eq $bb.Length){
   if($ab.Length -eq 5 -and $ab[0] -eq 0xe8 -and $bb[0] -eq 0xe8){
    $at=$va+5+[BitConverter]::ToInt32($ab,1);$bt=$vb+5+[BitConverter]::ToInt32($bb,1)
    if($targets.ContainsKey([long]$at) -and $bt -eq $targets[[long]$at]){[Array]::Copy($ab,1,$normalized,1,4);$checked+='relative-call'}else{$unresolved+='relative-call target'}
   }else{
    for($o=0;$o+4 -le $ab.Length;$o++){
     $at=[long][BitConverter]::ToUInt32($ab,$o);$bt=[long][BitConverter]::ToUInt32($bb,$o);$valid=$false
     if($targets.ContainsKey($at)){$valid=$bt -eq $targets[$at];if(!$valid){$unresolved+=('target@'+$o)}}
     elseif($strings.ContainsKey($at)){
      $expected=[Text.Encoding]::ASCII.GetBytes($strings[$at]+[char]0)
      try{$sa=Read-Bytes $old $at $expected.Length;$sb=Read-Bytes $new $bt $expected.Length;$valid=([Convert]::ToBase64String($sa) -eq [Convert]::ToBase64String($expected)) -and ([Convert]::ToBase64String($sb) -eq [Convert]::ToBase64String($expected))}catch{$valid=$false}
      if(!$valid){$unresolved+=('string@'+$o)}
     }
     if($valid){[Array]::Copy($ab,$o,$normalized,$o,4);$checked+=('absolute@'+$o);$o+=3}
    }
   }
  }
  $verified+=$checked.Count
  $exact=([Convert]::ToBase64String($ab) -eq [Convert]::ToBase64String($normalized)) -and !$unresolved.Count
  $rows+=,[pscustomobject]@{OriginalBlock=$pair.Original;RebuiltBlock=$pair.Rebuilt;OriginalAddress=$a.address;Original=$a.instruction;RebuiltAddress=$b.address;Rebuilt=$b.instruction;OriginalBytes=[BitConverter]::ToString($ab);RebuiltBytes=[BitConverter]::ToString($bb);ValidatedReferenceFields=$checked.Count;UnresolvedReferences=$unresolved;NormalizedBytesEqual=$exact}
 }
}
$summary=[ordered]@{Scope='Non-branch instructions paired by ordinal within CFG-paired blocks. Only target-validated known reference fields normalized; strings checked by exact bytes. Differences may reflect scheduling. Branches excluded; no whole-function match or runtime proof. Callee-body/import identity evidence is external to this report.';ComparedInstructions=$rows.Count;NormalizedEqualInstructions=@($rows|Where-Object NormalizedBytesEqual).Count;DifferentInstructions=@($rows|Where-Object {!$_.NormalizedBytesEqual}).Count;ValidatedReferenceFields=$verified;Rows=$rows}
$summary|ConvertTo-Json -Depth 6|Set-Content "$BuildDirectory\initinstance-operands.json"
$rows|Where-Object {!$_.NormalizedBytesEqual}|Export-Csv "$BuildDirectory\initinstance-operand-differences.csv" -NoTypeInformation
$summary.Remove('Rows');$summary|ConvertTo-Json
