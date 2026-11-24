$ErrorActionPreference='Stop'
function Open-PE([string]$Path){
 $b=[IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $Path));$pe=[BitConverter]::ToInt32($b,60);$opt=$pe+24
 $sections=@();$table=$opt+[BitConverter]::ToUInt16($b,$pe+20)
 for($i=0;$i -lt [BitConverter]::ToUInt16($b,$pe+6);$i++){$s=$table+40*$i;$sections+=,@([BitConverter]::ToUInt32($b,$s+12),[BitConverter]::ToUInt32($b,$s+16),[BitConverter]::ToUInt32($b,$s+20))}
 return @{Bytes=$b;Base=[BitConverter]::ToUInt32($b,$opt+28);Sections=$sections;ImportRVA=[BitConverter]::ToUInt32($b,$opt+104);Imports=@{}}
}
function Bytes($P,[long]$VA,[int]$Length){
 $rva=$VA-$P.Base
 foreach($s in $P.Sections){if($rva -ge $s[0] -and ($rva+$Length) -le ($s[0]+$s[1])){$o=$s[2]+$rva-$s[0];return ,([byte[]]$P.Bytes[$o..($o+$Length-1)])}}
 throw ('Unmapped VA {0:X}' -f $VA)
}
function U32($P,[long]$VA){return [BitConverter]::ToUInt32((Bytes $P $VA 4),0)}
function CString($P,[long]$VA){$chars=@();for($i=0;$i -lt 512;$i++){$c=(Bytes $P ($VA+$i) 1)[0];if(!$c){return [Text.Encoding]::ASCII.GetString([byte[]]$chars)};$chars+=$c};throw 'Unterminated name'}
function Load-Imports($P){
 for($d=$P.Base+$P.ImportRVA;;$d+=20){$oft=U32 $P $d;$name=U32 $P ($d+12);$ft=U32 $P ($d+16);if(!$name){break};$module=(CString $P ($P.Base+$name)).ToLowerInvariant();if(!$oft){$oft=$ft}
  for($i=0;;$i++){$t=U32 $P ($P.Base+$oft+4*$i);if(!$t){break};if($t -band 0x80000000L){$symbol='#'+($t -band 65535)}else{$symbol=CString $P ($P.Base+$t+2)};$P.Imports[[long]($P.Base+$ft+4*$i)]=$module+'!'+$symbol}
 }
}

