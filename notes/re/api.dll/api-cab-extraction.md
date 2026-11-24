# API CAB entry extraction and FDI callbacks

Original `api.dll` SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Live caller and callback references were read at saved Ghidra revision
`1793741866691:2a9d629f:3`. Instruction and callee flow evidence comes from
the original-verified listings in `artifacts/api.dll/api-source-pass-20261107/functions`
and the pinned PE string bytes. No Ghidra edits were applied.

## Extraction path

- `20144f5f` returns a stream pointer. `20133137` stores the return in its
  stream output at `20133237` and subsequently passes it to stream wrappers
  and release. `2014391b` calls its Read slot after using it as a fallback
  for the file-stream open. This supersedes the previous size-consumer
  hypothesis. The notification's byte count determines the output stream
  length.
- `20144b0e` extracts into an optional existing stream, with EAX owner and
  two stack arguments. Both callers reject a directory or missing entry.
  They initialize a four-word callback context containing owner, relative
  path, output stream and completion flag. The existing output receives
  AddRef before dispatch and Release only on completion. On an incomplete
  operation the original branch skips that Release. The source preserves
  the observed calls and returns null if completion was never marked.
- `20144924` runs under the owner lock at `2020e29c`. It temporarily
  selects a single entry in the image, publishes the active owner at
  `2020d8e4`, and resets the open counter at `2020d8e0`. The seven stack
  arguments to `2014a337` match the SDK FDICopy interface. PE strings
  `200f374c` and `200f24c0` contain `X` and an empty path respectively.
  On normal return, dispatch restores the entry offset and count from the
  owner, clears the active owner and unlocks. The two callers use the
  callback's completion flag independently of the FDICopy result.
- `201447d8` handles COPY_FILE (2) by returning the existing stream or
  creating a memory stream sized from the notification. CLOSE_FILE_INFO
  (3) seeks back to the beginning, ignores the seek result and sets the
  completion flag. Its return is zero, including for event 3. Original
  `2014a1e7` treats that zero as failure after the completion store and
  clears its output handle. A write failure before notification instead
  reaches the FDI close adapter, leaving the completion flag clear.
- `201445fe` is the callback passed by Initialize at `201204b4`. Its
  first open wraps the prefix image. Later opens resolve the archive path,
  open the file and optionally wrap it in a 0x1000-byte buffered stream
  before combining the prefix with the backing stream. The open counter
  increments even when the final wrap returns null. Missing owner, image,
  path or file takes the -1 return. The observed prefix length is read
  from image +0x24, or from image +0x28 plus the WORD at +0x24 when flags
  are 4.

## Native result and remaining work

The source uses the installed VC8 SDK callback types and connects
`FdiOpenBoundaryPilot` to the existing Initialize callback reference.
The image header model shares its observed offsets with the prefix loader.
The native object retains a normal external FDICopy reference. The original
library body and final library link remain separate verification work.
The CAB owner reload boundary remains explicit.

Run `api-cab-extraction-20261109-001` retained object SHA256
`51FC14860C2087A1C2E5D398C0870890D80D975AB40BB4FC318A0BBAEF769700`.

| Original address | Original/native bytes | Result |
|---|---:|---|
| `201445fe` | 334/334 | Different register/branch scheduling, visible before the first owner null branch |
| `201447d8` | 99/89 | Different frame and instruction selection |
| `20144924` | 160/146 | Original register arguments differ from source ABI |
| `20144b0e` | 141/151 | Original register arguments and lookup call differ |
| `20144f5f` | 191/195 | Different frame and lookup/dispatch call shapes |

Fifteen comparators pass at 4935 complete code bytes and 288 metadata bytes.
Stop matching trials after the initial build. The next useful package is
the caller at `20133137`, starting with the direct-file versus CAB fallback
selection and the subsequent wrapper/decode ownership transitions.

Suggested Ghidra names are `20144924` `unaff_ESI` → `cabOwner` and
`in_EAX` → `selectedEntry`, based on the owner field accesses and image
offset calculation. At `201447d8`, `iVar1` → `extractionContext` follows
from the notification's user pointer. These suggestions were left unapplied.
