# API service packet dispatch

Original `api.dll`. Four retained listings totaling 3379
instruction bytes were verified against the PE. The 197
selectors at `200fb0e0` and their jump targets at `200fafbc`
identify 72 handled cases. The source routing table records the
size checks and handler entry addresses from those case blocks.

`200fa39d` publishes packet sequence and client ID at
`2020d914` and `2020d918`. A missing client takes the open
route only for the open command and its exact size. Other
packets from unknown clients can be logged. A known client
must match the version at +0x3c8. Version mismatch sends
exception 5. Failed size checks send exception 2.

The known-client path tries the version-adjusted selector,
then steps down by 0x44 while the command stays in range.
It stops after the first accepted case. The close case only
logs and marks the case handled here. The four range checks
at `200fa337` have inclusive endpoints. Other cases use
the exact size check at `200fa36e`.

The source adds an early return for null or short headers.
The callback entry is present. The startup assignment at
`200f7bb6`, client lookup, exception output and individual
command bodies remain source boundaries. Handler addresses
are identifiers passed to an unresolved source adapter.

Suggested Ghidra names, unapplied. At `200fa39d`, `param_2`
→ `packet` from the header loads, `param_3` → `packetBytes`
from the size checks, and `iVar3` → `client` from lookup and
the version comparison.

`api-service-packet-dispatch-20261111-001` passed all 20
comparators. COFF SHA256 is
`D7330347A369BF7A34AAEEDD4134430C484832680B3FA0E89B772789DADE8F1B`.
New byte matches and runtime verification remain open.
Ghidra was unchanged. The next package is client open and
exception response at `200f7d7a` and `200f7d05`.
