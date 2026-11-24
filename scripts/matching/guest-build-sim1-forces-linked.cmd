@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /Gy /GL /fp:fast /DSIM1_FORCE_COMPARISON /Fdforces-vc.pdb /c flight_forces.cpp state_integration.cpp sim1-force-call-context.cpp
if errorlevel 1 exit 3
>api-forces.def echo LIBRARY api.dll
>>api-forces.def echo EXPORTS
>>api-forces.def echo     SimFillBytes @109 NONAME
lib /NOLOGO /MACHINE:X86 /DEF:api-forces.def /OUT:api-forces.lib
if errorlevel 1 exit 4
dumpbin /LINKERMEMBER:1 api-forces.lib
if errorlevel 1 exit 4
link /NOLOGO /DLL /NOENTRY /LTCG /OPT:REF /OPT:NOICF /DEBUG /ALTERNATENAME:__imp__SimFillBytes@12=__imp__SimFillBytes /MAP:fsx.map /PDB:fsx.pdb /OUT:fsx.exe flight_forces.obj state_integration.obj sim1-force-call-context.obj api-forces.lib kernel32.lib
if errorlevel 1 exit 4
dumpbin /DISASM fsx.exe > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS /EXPORTS /IMPORTS fsx.exe > headers.txt
if errorlevel 1 exit 5
exit 0
