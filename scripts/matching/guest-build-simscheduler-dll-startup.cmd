@echo off
setlocal
set "VCVARS=%ProgramFiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
if not exist "%VCVARS%" exit 2
call "%VCVARS%"
if errorlevel 1 exit 2
cl
cl /nologo /W4 /EHs /O2 /Oy- /hotpatch /GS /MD /Zi /GL- /Gy /GR- /Fdstartup-vc.pdb /c scheduler_dll_startup.cpp
if errorlevel 1 exit 3
>scheduler-api.def echo LIBRARY api.dll
>>scheduler-api.def echo EXPORTS
>>scheduler-api.def echo NotifySchedulerApiAttach@0 @103 NONAME
>>scheduler-api.def echo NotifySchedulerApiDetach@0 @104 NONAME
lib /NOLOGO /MACHINE:X86 /DEF:scheduler-api.def /OUT:scheduler-api.lib
if errorlevel 1 exit 4
link /NOLOGO /DLL /OPT:REF /OPT:NOICF /DEBUG /MAP:fsx.map /PDB:fsx.pdb /OUT:fsx.exe scheduler_dll_startup.obj scheduler-api.lib kernel32.lib
if errorlevel 1 exit 5
dumpbin /DISASM fsx.exe > disassembly.txt
if errorlevel 1 exit 6
dumpbin /HEADERS /IMPORTS fsx.exe > headers.txt
if errorlevel 1 exit 6
exit 0
