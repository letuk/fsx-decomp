@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /Gy /GL- /Fdgrid-vc.pdb /c magnetic_grid.cpp
if errorlevel 1 exit 3
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /Gy /GL /fp:fast /QIfist /Fdvariation-vc.pdb /c magnetic_variation.cpp
if errorlevel 1 exit 3
link /NOLOGO /DLL /NOENTRY /LTCG /OPT:REF /OPT:NOICF /DEBUG /MAP:fsx.map /PDB:fsx.pdb /OUT:fsx.exe magnetic_variation.obj magnetic_grid.obj kernel32.lib
if errorlevel 1 exit 4
dumpbin /DISASM fsx.exe > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS /EXPORTS fsx.exe > headers.txt
if errorlevel 1 exit 5
exit 0
