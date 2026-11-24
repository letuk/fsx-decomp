@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
echo API comparison-only DLL: no entry point, no runtime execution
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /GL- /c window_handle.cpp window_state.cpp
if errorlevel 1 exit 3
rem fsx.exe is the existing worker transport filename; /DLL determines PE kind.
link /nologo /DLL /NOENTRY /NODEFAULTLIB /OUT:fsx.exe /PDB:fsx.pdb /MAP:fsx.map /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF /MACHINE:X86 /BASE:0x200e0000 window_handle.obj window_state.obj
if errorlevel 1 exit 4
dumpbin /DISASM fsx.exe > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS fsx.exe > headers.txt
if errorlevel 1 exit 5
exit 0
