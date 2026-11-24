@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
echo API factory/four-slot VTable native COFF; no linking or execution
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /GL- /Gy /GR- /Fdfsx.pdb /c factory_boundary.cpp
if errorlevel 1 exit 3
copy /b factory_boundary.obj fsx.exe >nul
if errorlevel 1 exit 4
dumpbin /DISASM factory_boundary.obj > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS /SYMBOLS /RELOCATIONS factory_boundary.obj > headers.txt
if errorlevel 1 exit 5
exit 0
