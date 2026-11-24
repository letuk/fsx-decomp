@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
echo API callback-registry/reallocation COFF, no linking or execution
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /GL- /Gy /Fdfsx.pdb /c callback_registry.cpp
if errorlevel 1 exit 3
copy /b callback_registry.obj fsx.exe >nul
if errorlevel 1 exit 4
dumpbin /DISASM callback_registry.obj > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS /SYMBOLS /RELOCATIONS callback_registry.obj > headers.txt
if errorlevel 1 exit 5
exit 0
