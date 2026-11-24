@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /Gy /GL- /fp:fast /Fdfsx.pdb /c state_integration.cpp
if errorlevel 1 exit 3
copy /b state_integration.obj fsx.exe >nul
if errorlevel 1 exit 4
dumpbin /DISASM state_integration.obj > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS /SYMBOLS /RELOCATIONS state_integration.obj > headers.txt
if errorlevel 1 exit 5
exit 0
