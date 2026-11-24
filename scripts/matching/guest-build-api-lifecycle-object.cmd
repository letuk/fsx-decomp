@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
echo API lifecycle COFF object: unresolved dependencies retained, no linking/execution
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /GL- /Gy /Fdfsx.pdb /c lifecycle.cpp
if errorlevel 1 exit 3
rem Existing worker transports this COFF object using its fixed output filename.
copy /b lifecycle.obj fsx.exe >nul
if errorlevel 1 exit 4
dumpbin /DISASM lifecycle.obj > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS /SYMBOLS /RELOCATIONS lifecycle.obj > headers.txt
if errorlevel 1 exit 5
exit 0
