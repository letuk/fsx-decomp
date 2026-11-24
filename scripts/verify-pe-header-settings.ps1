param([Parameter(Mandatory)][string]$BuildDirectory,
 [Parameter(Mandatory)][ValidateSet('Default','LargeAddress','LargeAddressNx','Original')][string]$HeaderFlagsMode,
 [Parameter(Mandatory)][ValidateSet('Default','Original')][string]$ImageVersionMode,
 [Parameter(Mandatory)][ValidateSet('Default','Original')][string]$OsVersionMode)
$ErrorActionPreference='Stop'
. "$PSScriptRoot\lib\pe-image.ps1"
function Settings($p){$h=[BitConverter]::ToInt32($p.Bytes,60);$o=$h+24;[pscustomobject]@{CoffFlags=('{0:X4}' -f [BitConverter]::ToUInt16($p.Bytes,$h+22));DllFlags=('{0:X4}' -f [BitConverter]::ToUInt16($p.Bytes,$o+70));OsVersion=('{0}.{1}' -f [BitConverter]::ToUInt16($p.Bytes,$o+40),[BitConverter]::ToUInt16($p.Bytes,$o+42));ImageVersion=('{0}.{1}' -f [BitConverter]::ToUInt16($p.Bytes,$o+44),[BitConverter]::ToUInt16($p.Bytes,$o+46));SubsystemVersion=('{0}.{1}' -f [BitConverter]::ToUInt16($p.Bytes,$o+48),[BitConverter]::ToUInt16($p.Bytes,$o+50))}}
$a=Settings (Open-PE "$PSScriptRoot\..\original\fsx.exe");$b=Settings (Open-PE "$BuildDirectory\fsx.exe")
if($a.CoffFlags -cne '0123' -or $a.DllFlags -cne '8100' -or $a.OsVersion -cne '5.2' -or $a.ImageVersion -cne '5.2' -or $a.SubsystemVersion -cne '4.0'){throw 'Original header anchors changed'}
$expectedCoff=if($HeaderFlagsMode -eq 'Default'){'0103'}else{'0123'}
$expectedDll=switch($HeaderFlagsMode){'Default'{'0000'}'LargeAddress'{'0000'}'LargeAddressNx'{'0100'}'Original'{'8100'}}
$expectedImage=if($ImageVersionMode -eq 'Original'){'5.2'}else{'0.0'};$expectedOs=if($OsVersionMode -eq 'Original'){'5.2'}else{'4.0'}
if($b.CoffFlags -cne $expectedCoff -or $b.DllFlags -cne $expectedDll){throw "Requested PE flags not emitted: COFF=$($b.CoffFlags) DLL=$($b.DllFlags)"}
if($b.ImageVersion -cne $expectedImage){throw "Requested image version not emitted: $($b.ImageVersion)"}
if($b.OsVersion -cne $expectedOs){throw "Requested OS version not emitted: $($b.OsVersion)"}
if($b.SubsystemVersion -cne '4.0'){throw 'Unrequested subsystem-version change'}
[pscustomobject]@{HeaderFlagsMode=$HeaderFlagsMode;ImageVersionMode=$ImageVersionMode;OsVersionMode=$OsVersionMode;RequestedSettingsVerified=$true;Original=$a;Rebuilt=$b;AllSelectedFieldsMatchOriginal=(($a|ConvertTo-Json -Compress) -ceq ($b|ConvertTo-Json -Compress));Scope='Selected COFF/DLL flags and OS/image/subsystem versions checked against requested modes and original anchors. No interpretation as application runtime support requirements, original unique command or whole PE match.'}|ConvertTo-Json -Depth 5
