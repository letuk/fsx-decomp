@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /Gy /GL- /fp:fast /Fdfsx.pdb /Foturbine-state.obj /c SIM1_SOURCE_NAME
if errorlevel 1 exit 3
copy /b turbine-state.obj fsx.exe >nul
if errorlevel 1 exit 4
dumpbin /DISASM turbine-state.obj > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS /SYMBOLS /RELOCATIONS turbine-state.obj > headers.txt
if errorlevel 1 exit 5
exit 0
