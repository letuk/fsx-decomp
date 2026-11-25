@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /Gy /GL /fp:fast /DSIM1_THRUST_COMPARISON /Fdthrust-vc.pdb /c engine_forces.cpp electrical.cpp sim1-curve-call-context.cpp
if errorlevel 1 exit 3
>util-propeller.def echo LIBRARY util.dll
>>util-propeller.def echo EXPORTS
>>util-propeller.def echo     AdvanceWrappedAngle @199 NONAME
lib /NOLOGO /MACHINE:X86 /DEF:util-propeller.def /OUT:util-propeller.lib
if errorlevel 1 exit 4
dumpbin /HEADERS util-propeller.lib
if errorlevel 1 exit 4
link /NOLOGO /DLL /NOENTRY /LTCG /OPT:REF /OPT:NOICF /DEBUG /ALTERNATENAME:__imp__AdvanceWrappedAngle@16=__imp__AdvanceWrappedAngle /MAP:fsx.map /PDB:fsx.pdb /OUT:fsx.exe engine_forces.obj electrical.obj sim1-curve-call-context.obj util-propeller.lib kernel32.lib
if errorlevel 1 exit 4
dumpbin /DISASM fsx.exe > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS /EXPORTS /IMPORTS fsx.exe > headers.txt
if errorlevel 1 exit 5
exit 0
