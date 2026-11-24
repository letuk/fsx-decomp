@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
cl /nologo /W4 /EHs /O2 /Oy /hotpatch /GS /MD /Zi /GL- /Gy /GR- /Fdfsx.pdb /c scheduler_object.cpp
if errorlevel 1 exit 3
copy /b scheduler_object.obj fsx.exe >nul
if errorlevel 1 exit 4
dumpbin /DISASM scheduler_object.obj > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS /SYMBOLS /RELOCATIONS scheduler_object.obj > headers.txt
if errorlevel 1 exit 5
exit 0
