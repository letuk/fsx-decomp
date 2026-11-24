# API file stream selection and CAB fallback

Original `api.dll` SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Evidence uses original-verified listings, pinned PE exports, imports and
jump-table bytes, and read-only live caller checks. Ghidra revision
`1793741866691:2a9d629f:3` was saved and unchanged.

## Export 85 at 20133137

The four stack arguments are output stream slot, path, suffix selector and
mode. Resolution failure returns -1 before changing the output. When the
selector is 1 and the mode is 0, 1, 2 or 6, the original checks the separate
stream cache at `20144003` first. Its hit status is independent of the Clone
return value. A cache hit returns success immediately.

| Mode | Direct access | Sharing and disposition | Post-open path |
|---:|---|---|---|
| 0 | Read | Share read, open existing | Eligible for buffering and CAB fallback |
| 1 | Read, random-access hint | Share read, open existing | Eligible for buffering and CAB fallback |
| 2 | Read, sequential-scan hint | Share read, open existing | Eligible for buffering and CAB fallback |
| 3 | Read/write | Exclusive, open existing | No buffering or CAB fallback |
| 4 | Write | Exclusive, open existing | No buffering or CAB fallback |
| 5 | Write | Exclusive, open always | Seek to end and ignore seek status |
| 6 | Read | Share read, open existing | Eligible for CAB fallback, skips decode and buffering |

Unsupported modes clear the output and return -1. A failed direct open can
fall back to `20144f5f` only in the four eligible modes and when the
selector differs from 0x34. The CAB stream disables subsequent file
buffering. For selector 1 and mode other than 6, `20132e20` probes the
resulting stream even in direct write modes if open succeeded.

Successful decode replaces the output after releasing the old stream and
invokes cache insertion. If decode produces null, the caller keeps the
stream and can try the optional 0x1000-byte wrapper for eligible direct
opens. Wrapper success releases the previous reference and replaces the
output. Selector-1 CAB fallback results also reach cache insertion, including
mode 6. The insertion result is ignored.

The source in `file_stream_open.cpp` connects the existing path resolver,
file stream, CAB extraction, release, seek and wrapper functions. The
decoder and two cache calls were explicit source boundaries at this checkpoint.
Their subsequent source integration is recorded in
[API file stream decoder and cache](api-file-stream-decode-cache.md).

## Dependency evidence

- `20144003` passes Clone output to slot +0x34 after updating the entry's
  hit count and last-use value and moving it to the list head. Its lock at
  `2020e284` and head at `2020d8cc` are distinct from the CAB owner cache.
  `20143f76` checks that cache again under the recursive critical section.
  On a miss it prunes through `201440cc` and allocates an observed
  0x114-byte record. `20144152` retains the stream via AddRef.
- `20132e20` probes 0xbc bytes, checks marker and size fields, then invokes
  a four-argument transform at `util.dll` ordinal 195. The pinned original
  import and local `util.dll` export table resolve to `20f4d270` in
  `util.dll` SHA256
  `A668292DD99BCBEAD89489A0BD4B75B98A376577558C7A1172FB4294CF47576B`.
  The source for this transform remains external to the API slice.
- `20133611` seeks current, end and then back to the saved full position.
  It returns the restore seek's status and copies the end low DWORD.
  The first two seek statuses are ignored. Decode failure normally seeks
  back to zero, but the successful transform branches directly to the
  wrapping call's return. If wrapping returns null there, that return
  bypasses the fallback cleanup. These are instruction-level observations.

## Build and stop point

Run `api-file-stream-open-20261109-001` retained native object SHA256
`FA7284BF940A4AE1AEF3A55365BBFE13359EE99CB776572D6426978879C75709`.
The selection body emits 624 bytes against 600 original bytes and uses a
different mode dispatch and frame. Its COFF references include the actual
CAB entry open and existing stream methods. Fifteen comparators pass at
4935 complete code bytes and 288 metadata bytes. Matching stopped after the
initial source build. Native linking and runtime behavior remain unverified.
The next bounded package is the decoder at `20132e20` with its seek/size
contract and ordinal-195 boundary, followed by the separate stream cache.

Suggested Ghidra names for `20133137` are `param_1` → `streamOutput` and
`param_4` → `openMode`, based on output stores and the verified jump table.
For `20132e20`, `local_118` → `inputStream` follows the read and seek uses.
These suggestions were left unapplied.
