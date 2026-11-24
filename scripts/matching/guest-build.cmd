@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
echo Matching search: SP1, pinned baseline flags
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /GL /EHsc /D_AFXDLL /c startup_context_pilot.cpp version_checks_pilot.cpp string_copy_pilot.cpp string_format_pilot.cpp
if errorlevel 1 exit 3
cl /nologo /W4 /O2 /Oy- /hotpatch /GS /MD /Zi /GL /c application_interface_pilot.cpp
if errorlevel 1 exit 3
link /nologo /OUT:fsx.exe /PDB:fsx.pdb /MAP:fsx.map /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF /MACHINE:X86 /SUBSYSTEM:CONSOLE /LTCG startup_context_pilot.obj version_checks_pilot.obj string_copy_pilot.obj string_format_pilot.obj application_interface_pilot.obj kernel32.lib user32.lib ole32.lib
if errorlevel 1 exit 4
dumpbin /DISASM fsx.exe > disassembly.txt
if errorlevel 1 exit 5
dumpbin /HEADERS fsx.exe > headers.txt
if errorlevel 1 exit 5
exit 0



