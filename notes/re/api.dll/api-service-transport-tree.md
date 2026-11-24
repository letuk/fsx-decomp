# API service transport tree insertion

Scope: FSX Acceleration `api.dll`. Live verified 1,742 instruction
bytes against the local PE. Names describe the reconstruction.

| Address | Live-verified behavior |
|---|---|
| `201165f2`, `201167f5`, `20116bb1` | A signed 32-bit key selects a client slot. An existing key returns its slot without allocation. A missing key inserts a pair with a null client value. The original passes a lower-bound hint and can fall back to a general insert. |
| `20110660`, `20116c87` | Allocates a 0x18-byte node, sets sentinel children, parent and payload, and starts it red. The header's sentinel tracks the minimum, root and maximum at +0, +4 and +8 respectively. Header count at +8 increments once per new key. |
| `20110867`, `201108ba`, `20116c87` | Left and right rotations repair parent and root links without changing the sentinel's root link when a moved child is the sentinel. Insertion repairs red-parent cases by recoloring or rotating and leaves the root black. Node color +0x14 is zero for red and one for black. |
| `20115314`, `20115ad8`, `20116676`, `201112ae` | Both listeners publish the client in the returned slot. Outbound lookup searches the same tree by key. Shutdown frees tree nodes while the client groups release client references. |

The source collapses the original lower-bound and hint paths into a
single signed-key walk and reuses the existing header and node layouts.
A failed insertion leaves the group-owned client available for shutdown.
The source returns no slot at the size limit or on allocation failure.
The original explicitly throws at the size limit.

Suggested Ghidra names, unapplied. In `201165f2`, `puVar4` →
`candidate` for the lower-bound result. In `20116c87`, `piVar2`
→ `insertedNode` based on the allocation return and the iterator output.

VC8 x86 build `api-service-transport-tree-20261112-001` passed all
20 existing comparators. COFF SHA256 is
`21E133D58DCCE9C0CF8C5BBF438F769C067DE75DB6BAB22C7D94F59337FB9979`.
New byte matches and runtime verification remain open. Ghidra was unchanged.

The next API boundary is connection reset and close at `2011a7ed`
and `2011a8d4`, including their handoff to the service window.
