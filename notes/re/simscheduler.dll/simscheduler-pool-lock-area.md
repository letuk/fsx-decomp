# simscheduler.dll shared pool lock

Live verified in simscheduler.dll from the original listings and PE.
`FUN_20cc6580` builds the shared `0x28`-byte lock. It creates a
reader semaphore and an auto-reset writer event and publishes
signature `0x4c664c6b`. Partial construction closes any handle
already created and releases the raw allocation. The rebuilt layout
is a minimum model. State occupies `+0x00` through `+0x07`. Owner
mode and thread ID are at `+0x08/+0x0c`, handles at `+0x10/+0x14`,
signature and writer recursion depth at `+0x18/+0x1c`, and raw
storage and allocator at `+0x20/+0x24`.

The packed state keeps queued writers in bits 0–19. Bits 20–23
carry the exclusive, promotion, spin and closing flags. Queued and
active readers occupy bits 24–43 and 44–63, respectively. The acquire
dispatch at `FUN_20cc54b0` handles shared, exclusive, downgrade
and two promotion modes. Mode 3 checks for queued writers before
promotion. Bit 3 of the request selects a non-waiting attempt.
Blocked paths use the relevant Win32 object or the supplied wait
callback. The timed-out promotion restores one shared acquisition.
The C++ uses the packed state and explicit CAS transitions and
separates the read, write, downgrade and promotion paths in source.
Names for the fields and modes are reconstructed.

`FUN_20cc61c0` releases a reader or writer and wakes the appropriate
waiters. `FUN_20cc63e0` invalidates the signature first. The busy path
returns without closing the handles. An idle lock is marked closing,
then its handles and backing storage are released. `FUN_20cc4a10`
uses the low timestamp counter bits to vary its pause loop, and grows
the mask until it reaches the cap.

| Entry | Operation | Original / rebuilt bytes |
| --- | --- | --- |
| `0x20cc4a10` | Spin backoff | 41 / 39 |
| `0x20cc54b0` | Acquire or convert | 3325 / 2564 |
| `0x20cc61c0` | Release | 538 / 383 |
| `0x20cc63e0` | Destroy | 396 / 273 |
| `0x20cc6580` | Create | 242 / 165 |

All five VC8 SP1 native COFF comparisons are diagnostic. The contiguous
acquire and release ranges include one and eight skipped alignment bytes,
respectively. The first source snapshot could claim a queued acquisition
before waiting even with spinning disabled. The second gates that path
on spinning or a wait signal. The retained third build also inlines the
mode helpers into the acquire body. The original selects locked or
unlocked `cmpxchg8b` from `DAT_20cc8010`. The C++ uses the VC8
interlocked intrinsic in both cases and replaces the original hidden
EDI acquire and EAX destroy arguments with explicit parameters.
The reports retain paired and unpaired address fields. Exact wake
ordering, races, timeouts and linked runtime behavior remain unverified.

- [Original Ghidra listings](../../../artifacts/simscheduler.dll/pool-lock-ghidra.json)
- [Retained worker receipt](../../../vm-share/modules/simscheduler/runs/simscheduler-pool-lock-20261116-003/receipt.json)
- [Acquire comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-pool-lock-20261116-003/comparison-20cc54b0.json)
- [Release comparison](../../../vm-share/modules/simscheduler/runs/simscheduler-pool-lock-20261116-003/comparison-20cc61c0.json)

The 49 preceding native match gates and retained Flight save and GPS
comparisons pass. Ghidra was read-only and reported no unsaved changes.
The next area is PE startup and CRT dispatch from `0x20cc6988` to
`0x20cc686d`.
