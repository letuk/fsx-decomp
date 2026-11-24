# API menu host tables and teardown

Original `api.dll`. The PE contains 39 function pointers in each
observed table at `200f3370` and `200f3410`, with 34 shared targets.
The five differing slots are listed below. Six instruction listings
totaling 356 bytes were checked against the PE. The detach and
release bodies were also checked in live Ghidra.

| Slot | Default host | Window host |
|---|---|---|
| +0x24 | `2013a11c` | `20139413` |
| +0x2c | `2013a0ad` | `20138ba6` |
| +0x58 | `2013a1ac` | `201395e9` |
| +0x5c | `2013a163` | `2013962a` |
| +0x94 | `2013a0b5` | `20138aeb` |

Both +0x58 targets destroy the optional XUI object at host +8,
remove the root HWND property and forward the reason to the base
object's +0x58 slot. The default path also clears `2020e090`,
unregisters `FSXMenuBar` and clears `2020e09c`.

Shared +0x28 target `20138b76` saves and clears the base pointer,
deletes the wrapper through +0x98 and then calls base +0x28.
The shared deletion target `2013afb3` restores the window host
table and frees the wrapper when flag bit zero is set.
Shared +0x44 target `20138c2c` forwards the HWND query to the base.
Slot zero at `20138f9b` accepts the two observed interface IDs,
returning the wrapper pointer without a separate reference call.

The source provides those targets. Table placement and the
remaining slots still need integration. The +0x98 source entry
uses an explicit receiver while the original uses ECX. XUI object
destruction and class unregistration are modeled as import calls.
The next API boundary is host initialization at `20138fee` and
`201391d1`, where window creation, focus registration and canvas
setup meet. XUI module bodies remain a separate package.

Suggested Ghidra name, unapplied. At `20138b76`, `piVar1` →
`baseHost` from the host +4 read and subsequent +0x28 call.

`api-menu-host-teardown-20261111-001` passed all 20 comparators.
COFF SHA256 is
`EE1E4CCC84F33395B7083032571D04103ECADEC61CD43A8F85CBDF4B61D6E9C8`.
No new byte matches were accepted. Linking and runtime verification
remain open. Ghidra was read-only.
