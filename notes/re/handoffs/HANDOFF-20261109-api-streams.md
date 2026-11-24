# Handoff for API.DLL stream reconstruction

Prepared 2026-11-09 for a new chat. Workspace is G:\Projects\Reversing\fsx-decomp.
Read AGENTS.md, notes/re/STATUS.md, notes/re/SOURCES.md and the topic notes
first. Write project documentation in English and speak German with the user.

## Resume here

Continue api.dll with decoder 20132e20 and seek/size helper 20133611.
Start with notes/re/api.dll/api-file-stream-open.md and the original-verified ASM
and C exports under artifacts/api.dll/api-source-pass-20261107/functions.
DecodeFileStreamSourcePilot in src/api-pilot/file_stream_open.cpp is
the explicit source boundary.

1. Verify the 0xbc-byte probe, marker-dependent offsets and size arithmetic.
   At 20133611 distinguish the two ignored seek statuses from the returned
   restore-seek status and low DWORD size output.
2. Trace buffer ownership through allocation, transform, failure cleanup and
   wrapper creation. A successful transform returns directly from the wrapper
   call, including when wrapping returns null. Preserve the evidenced paths
   and keep unspecified failure outputs explicit.
3. The four-argument transform is imported from util.dll ordinal 195.
   Local original/util.dll resolves it to 20f4d270. Its SHA256 is
   A668292DD99BCBEAD89489A0BD4B75B98A376577558C7A1172FB4294CF47576B.
   Keep the transform boundary explicit until the contract is established.
4. Build and rerun the gates. After this slice, take the independent
   file-stream cache 20144003, 20143f76, 201440cc and 20144152. Its lock
   and head are 2020e284 and 2020d8cc.

## Last verified checkpoint

- Target original/api.dll SHA256
  9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003.
- Last successful build is vm-share/modules/api/runs/api-file-stream-open-20261109-001.
  The api-factory.obj SHA256 is
  FA7284BF940A4AE1AEF3A55365BBFE13359EE99CB776572D6426978879C75709.
  Receipt exit code is 0 and source-pass-regression.json records 15 passing
  comparators. The object is a comparison artifact.
- Accepted coverage is 4935 complete code bytes and 288 metadata bytes,
  with 123 partial Initialize bytes tracked separately. The latest addition
  was the 17-byte CAB registration body at 201ee724. Accepted ranges are
  tracked in notes/re/api.dll/api-code-ledger.md.
- Export 85 at 20133137 connects direct open, CAB fallback and buffered
  wrapping. Its body is 624 native versus 600 original bytes and is outside
  match acceptance. The decoder and separate stream cache remain explicit.
  See notes/re/api.dll/api-file-stream-open.md.
- CAB extraction connects 20144b0e, 20144f5f and dispatcher 20144924 to the
  FDI callbacks. 20144f5f returns a stream, superseding the size-consumer
  hypothesis. Final FDI linking and owner reload at 201449c4 remain open.
  Reload has two verified unwritten output paths. See notes/re/api.dll/api-cab-extraction.md,
  notes/re/api.dll/api-cab-owner-metadata.md and notes/re/api.dll/api-cab-buffer-containers.md.
- Last live api.dll Ghidra revision was 1793741866691:2a9d629f:3, saved
  with no pending changes. Recent inspection was read-only. Recheck current
  state, specify the program explicitly and let the user control tab changes.

## Build and scope

Choose a fresh API run ID. The commands below illustrate the next run.

```powershell
$run = 'api-file-stream-decode-20261109-001'
./scripts/matching/start-api-window-pilot.ps1 -FactoryObject -SourcePass -RunId $run
./scripts/matching/test-api-source-pass.ps1 -BuildDirectory "vm-share/modules/api/runs/$run"
```

Keep the API runs and ledger separate from fsx.exe. The earlier handoff's
consolidated executable Baseline 41 ledger is notes/re/fsx.exe/coverage-baseline41.md.
The active focus is api.dll. InitInstance, layout and import fine-tuning are
parked. Use structured C++ where possible. State the hypothesis and expected
observation before each matching trial. Limit each question to three targeted
attempts, stopping earlier on success, identical output, loss of reference
mapping or regression. Require complete body evidence and validated relocation
targets for new byte acceptance. The latest selection build stopped after one
trial. Do not run a replacement binary.

The user prefers autonomous execution across related steps. This checkpoint
is a user-requested cut. The pinned decoder inputs are already in the workspace.
