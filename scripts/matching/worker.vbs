Option Explicit
Dim fso, shell, root, queue, job, localDir, exitCode, stream, filename, claimed
If WScript.Arguments.Named.Exists("validate") Then
 WScript.Echo "VBScript syntax OK"
 WScript.Quit 0
End If
Set fso = CreateObject("Scripting.FileSystemObject")
Set shell = CreateObject("WScript.Shell")
If LCase(fso.GetFileName(WScript.FullName)) = "wscript.exe" Then
 shell.Run "cscript.exe //nologo """ & WScript.ScriptFullName & """", 1, False
 WScript.Quit 0
End If
root = fso.GetParentFolderName(WScript.ScriptFullName)
If WScript.Arguments.Named.Exists("takeover") Then
 Dim service, process
 Set service = GetObject("winmgmts:\\.\root\cimv2")
 For Each process In service.ExecQuery("SELECT * FROM Win32_Process WHERE Name='wscript.exe'")
  If Not IsNull(process.CommandLine) Then
   If InStr(1, process.CommandLine, WScript.ScriptFullName, vbTextCompare) > 0 Then
    If process.Terminate() <> 0 Then
     WScript.Echo "Could not stop the previous dialog-hosted worker. Close it before restarting."
     WScript.Quit 12
    End If
   End If
  End If
 Next
End If
queue = fso.BuildPath(root, "queue")
If Not fso.FolderExists(queue) Then fso.CreateFolder queue
If Not fso.FolderExists("C:\fsx-build") Then fso.CreateFolder "C:\fsx-build"
If Not fso.FolderExists("C:\fsx-build\matching-jobs") Then fso.CreateFolder "C:\fsx-build\matching-jobs"
WScript.Echo "FSX build worker ready. Keep this window open. Ctrl+C stops the worker."
Sub PublishState(status, currentJob)
 Dim stateFile
 Set stateFile = fso.CreateTextFile(fso.BuildPath(root, "worker-state.json"), True)
 stateFile.Write "{""Status"":""" & status & """,""Job"":""" & currentJob & """}"
 stateFile.Close
End Sub
Do
 If fso.FileExists(fso.BuildPath(root, "STOP-WORKER")) Then
  PublishState "stopped", ""
  WScript.Quit 0
 End If
 PublishState "ready", ""
 For Each job In fso.GetFolder(queue).SubFolders
  If fso.FileExists(fso.BuildPath(job.Path, "ready")) Then
   Dim validName
   Set validName = New RegExp
   validName.Pattern = "^[A-Za-z0-9_-]+$"
   If Not validName.Test(job.Name) Then WScript.Quit 10
   claimed = False
   On Error Resume Next
   fso.MoveFile fso.BuildPath(job.Path, "ready"), fso.BuildPath(job.Path, "working")
   If Err.Number = 0 Then claimed = True
   Err.Clear
   On Error GoTo 0
   If claimed Then
    PublishState "building", job.Name
    localDir = fso.BuildPath("C:\fsx-build\matching-jobs", job.Name)
    If fso.FolderExists(localDir) Then
     WScript.Echo "Refusing existing work directory: " & localDir
     WScript.Quit 11
    End If
    fso.CreateFolder localDir
    fso.CopyFile fso.BuildPath(job.Path, "source\*.*"), localDir & "\", False
    fso.CopyFile fso.BuildPath(job.Path, "build.cmd"), fso.BuildPath(localDir, "build.cmd"), False
    shell.CurrentDirectory = localDir
    WScript.Echo "Building " & job.Name
    exitCode = shell.Run("cmd.exe /d /c ""build.cmd > build-log.txt 2>&1""", 0, True)
    fso.CreateFolder fso.BuildPath(job.Path, "output")
    fso.CreateFolder fso.BuildPath(job.Path, "output\compiled-source")
    Dim compiledFile, extension
    For Each compiledFile In fso.GetFolder(localDir).Files
     extension = LCase(fso.GetExtensionName(compiledFile.Name))
     If extension = "cpp" Or extension = "h" Then fso.CopyFile compiledFile.Path, fso.BuildPath(job.Path, "output\compiled-source\" & compiledFile.Name), False
    Next
    For Each filename In Array("build-log.txt", "fsx.exe", "fsx.map", "fsx.pdb", "fsx.exe.manifest", "disassembly.txt", "headers.txt")
     If fso.FileExists(fso.BuildPath(localDir, filename)) Then fso.CopyFile fso.BuildPath(localDir, filename), fso.BuildPath(job.Path, "output\" & filename), False
    Next
    Set stream = fso.CreateTextFile(fso.BuildPath(job.Path, "receipt.json"), False)
    stream.Write "{""Job"":""" & job.Name & """,""ExitCode"":" & CStr(exitCode) & "}"
    stream.Close
    Set stream = fso.CreateTextFile(fso.BuildPath(job.Path, "done"), False)
    stream.Write job.Name
    stream.Close
    WScript.Echo "Finished " & job.Name & " exit=" & exitCode
   End If
  End If
 Next
 WScript.Sleep 1000
Loop
