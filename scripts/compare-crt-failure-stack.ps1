param([Parameter(Mandatory)][string]$BuildDirectory)
$ErrorActionPreference='Stop'
$imageJson=. "$PSScriptRoot\compare-crt-image-helpers.ps1" -BuildDirectory $BuildDirectory
$imageReport=$imageJson|ConvertFrom-Json
$report=Symbol '___report_gsfailure';$probe=Symbol '__alloca_probe'
Equal-Bytes (Bytes $a 0x01002bb0 43) (Bytes $b $probe 43) 'Base stack probe'
$probes=@()
foreach($f in @(@(0x01002790,'__alloca_probe_16'),@(0x010027a6,'__alloca_probe_8'))){$new=Symbol $f[1];$x=Bytes $a $f[0] 22;$y=Bytes $b $new 22;Require ((Rel $a $f[0] 18) -eq 0x01002bb0 -and (Rel $b $new 18) -eq $probe) 'Aligned stack probe target differs';[Array]::Copy($x,18,$y,18,4);Equal-Bytes $x $y $f[1];$probes+=@{Name=$f[1];OriginalVA=('{0:X8}' -f $f[0]);RebuiltVA=('{0:X8}' -f $new);Bytes=22;VerifiedAddressFields=1;AddressNormalizedExactMatch=$true}}
$x=Bytes $a 0x01002a6d 260;$y=Bytes $b $report 260
$context=[long][BitConverter]::ToUInt32($y,0x7d);$exception=[long][BitConverter]::ToUInt32($y,0x91);$debug=[long][BitConverter]::ToUInt32($y,0xc0);$pointers=[long][BitConverter]::ToUInt32($y,0xd5)
# The standard x86 Windows layouts were independently checked in Ghidra:
# _CONTEXT=716 bytes, _EXCEPTION_RECORD=80 bytes. Compare initial backing data,
# including virtual zero-filled BSS, and the two actual EXCEPTION_POINTERS links.
foreach($r in @(@(0x010040b0,$context,716),@(0x01004058,$exception,80))){for($offset=0;$offset -lt $r[2];$offset+=4){Require ((Initial-Dword $a ($r[0]+$offset)) -eq 0 -and (Initial-Dword $b ($r[1]+$offset)) -eq 0) 'Failure record initial data differs'}}
Require ((Initial-Dword $a 0x010040a8) -eq 0 -and (Initial-Dword $b $debug) -eq 0) 'Debugger state initial value differs'
Require ($debug -eq $exception+80 -and $context -ge $debug+4) 'Failure record/debug/context relationship differs'
Require ((U32 $a 0x010012d0) -eq 0x01004058 -and (U32 $a 0x010012d4) -eq 0x010040b0 -and (U32 $b $pointers) -eq $exception -and (U32 $b ($pointers+4)) -eq $context) 'Exception pointer record links differ'
$references=[Collections.Generic.List[object]]::new()
function Normalize-Failure([int]$offset,[long]$old,[long]$new,[string]$kind,[string]$role){
 if($kind -eq 'call'){$ox=Rel $a 0x01002a6d $offset;$ny=Rel $b $report $offset}else{$ox=[long][BitConverter]::ToUInt32($x,$offset);$ny=[long][BitConverter]::ToUInt32($y,$offset)}
 Require ($ox -eq $old -and $ny -eq $new) "Failure report target differs: $role"
 $references.Add([pscustomobject]@{Offset=$offset;Kind=$kind;Role=$role;OriginalTarget=('{0:X8}' -f $old);RebuiltTarget=('{0:X8}' -f $new)})
 [Array]::Copy($x,$offset,$y,$offset,4)
}
$contextFields=@(@(0x0a,0xb0,'Eax'),@(0x10,0xac,'Ecx'),@(0x16,0xa8,'Edx'),@(0x1c,0xa4,'Ebx'),@(0x22,0xa0,'Esi'),@(0x28,0x9c,'Edi'),@(0x2f,0xc8,'SegSs'),@(0x36,0xbc,'SegCs'),@(0x3d,0x98,'SegDs'),@(0x44,0x94,'SegEs'),@(0x4b,0x90,'SegFs'),@(0x52,0x8c,'SegGs'),@(0x59,0xc0,'EFlags'),@(0x61,0xb4,'Ebp'),@(0x69,0xb8,'Eip'),@(0x71,0xc4,'Esp'),@(0x7d,0,'ContextFlags'),@(0x86,0xb8,'Eip readback'))
foreach($f in $contextFields){Normalize-Failure $f[0] (0x010040b0+$f[1]) ($context+$f[1]) 'context' $f[2]}
foreach($f in @(@(0x8b,12,'ExceptionAddress'),@(0x91,0,'ExceptionCode'),@(0x9b,4,'ExceptionFlags'))){Normalize-Failure $f[0] (0x01004058+$f[1]) ($exception+$f[1]) 'exception' $f[2]}
Normalize-Failure 0xa4 0x01004004 (Symbol '___security_cookie') 'state' 'security cookie'
Normalize-Failure 0xaf 0x01004008 (Symbol '___security_cookie_complement') 'state' 'cookie complement'
foreach($offset in @(0xc0,0xe1)){Normalize-Failure $offset 0x010040a8 $debug 'state' 'debugger-present flag'}
Normalize-Failure 0xd5 0x010012d0 $pointers 'record' 'verified exception/context pointer pair'
foreach($f in @(@(0xbb,0x01001080),@(0xd0,0x0100107c),@(0xdb,0x01001078),@(0xf7,0x01001074),@(0xfe,0x01001070))){$new=[long][BitConverter]::ToUInt32($y,$f[0]);$identity=Iat $a $f[1];Require ($identity -ceq (Iat $b $new)) 'Failure-report import identity differs';Normalize-Failure $f[0] $f[1] $new 'iat' $identity}
$hook=Symbol '__crt_debugger_hook';Require ((Thunk $a 0x01002bf8) -ceq (Thunk $b $hook)) 'CRT debugger hook import differs'
foreach($offset in @(0xc7,0xeb)){Normalize-Failure $offset 0x01002bf8 $hook 'call' 'import-identified CRT debugger hook'}
Equal-Bytes $x $y '260-byte GS failure reporter'
[pscustomobject]@{OriginalSHA256=$imageReport.OriginalSHA256;RebuiltSHA256=$imageReport.RebuiltSHA256;FailureReporter=@{OriginalVA='01002A6D';RebuiltVA=('{0:X8}' -f $report);Bytes=260;VerifiedAddressFields=$references.Count;AddressNormalizedExactMatch=$true;ContextBytes=716;ExceptionRecordBytes=80;InitialRecordDataVerified=$true;ExceptionPointerPairVerified=$true;References=@($references)};AlignedProbes=$probes;BaseProbe=@{OriginalVA='01002BB0';RebuiltVA=('{0:X8}' -f $probe);Bytes=43;RawExactMatch=$true};Scope='Complete stated CRT reporter/probe bodies and initial context/exception storage plus pointer pair. Known standard x86 Windows data offsets and target/import identities validated before normalization. Stack probes have specialized EAX/ESP ABI; no ordinary C prototype inferred. No full EXE/layout, external DLL implementation or runtime proof; existing 18 application/helper/wrapper count unchanged.'}|ConvertTo-Json -Depth 8
