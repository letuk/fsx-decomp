@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /Gy /GL /fp:fast /Fdposition-vc.pdb /c position_attitude.cpp sim1-position-call-context.cpp
if errorlevel 1 exit 3
>api-position.def echo LIBRARY api.dll
>>api-position.def echo EXPORTS
>>api-position.def echo     SimPublishedPosition @304 NONAME DATA
lib /NOLOGO /MACHINE:X86 /DEF:api-position.def /OUT:api-position.lib
if errorlevel 1 exit 4
link /NOLOGO /DLL /NOENTRY /LTCG /OPT:REF /OPT:NOICF /DEBUG /MAP:fsx.map /PDB:fsx.pdb /OUT:fsx.exe position_attitude.obj sim1-position-call-context.obj api-position.lib kernel32.lib
if errorlevel 1 exit 4
dumpbin /DISASM fsx.exe > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS /EXPORTS /IMPORTS fsx.exe > headers.txt
if errorlevel 1 exit 5
exit 0
