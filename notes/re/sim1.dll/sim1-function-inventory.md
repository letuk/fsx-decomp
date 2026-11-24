# sim1.dll function inventory

The live read-only export on 2026-11-24 covers every function stored in
Ghidra for the original FSX 10.0.61637.0 module at image base
`0x20bb0000`. The original PE SHA-256 is
`18be04c6a85a3ccf3c5dae1411c652fa15ccbea62e0913780a1b784a9d3f3fc8`.
Each internal record has an entry VA, model name, signature, body ranges,
cross-reference counts, thunk status and tags. Calling conventions and
names are Ghidra's current model assignments. The original PE verifies
that all recorded internal ranges are backed by executable bytes.

| Inventory scope | Count |
| --- | ---: |
| Stored internal functions | 2,496 |
| Stored external function declarations | 246 |
| Total Ghidra function records | 2,742 |
| Internal entries with a `FUN_` name | 2,213 |
| Internal entries carrying an `Unwind@` name | 225 |
| Stored thunk functions | 35, including 24 targeting an external declaration |
| Function records with more than one body range | 162 |
| Actual PE exports | 25 destinations, with names on four ordinals |
| Ordinary PE import slots | 250 across 22 DLLs |

The DLL entry point is `0x20c66911`. External symbols are in Ghidra's
`EXTERNAL` address space. Four PE imports lack a function declaration
in the current Ghidra table. The IAT entries are `0x20bb10d8`
(`__clean_type_info_names_internal`), `0x20bb10f4` (`_adjust_fdiv`),
`0x20bb1134` (`_purecall`) and `0x20bb11fc` (`api.dll` ordinal 304).
The PE import and export inventory is independent of Ghidra's symbol
aliases and includes these four entries. The executable section contains
18,569 decoded instructions outside stored function bodies. The extent
of any additional functions in these unreviewed ranges remains open.
The exported bounds reflect the current database model.

The dashboard has all 2,496 internal entries plus the separate 144-byte
history-copy range under its original function. Three full COFF body
matches and the two whole-body differences retain their original and
rebuilt listings. The prior eight-function detailed inventory is
preserved alongside this complete metadata export. The original
function tags were read back, and reviewed subsystem names are
descriptions in the dashboard.

## Reviewed subsystem anchors

| Entry VA | Evidence and scope |
| --- | --- |
| `0x20c30840` | Live verified aircraft update dispatcher with a branch to the flight step at `0x20c30bc0`. |
| `0x20c30bc0` | Live verified flight-step orchestrator. Calls force-vector accumulation, acceleration preparation, velocity integration and the low-motion check before the conditional position and attitude step. |
| `0x20bdec10` | Live verified concrete position and attitude callback at heading transform slot `+0x00`. The step advances geodetic position, altitude and attitude subject to freeze fields. |
| `0x20bccae0` | Live verified pre-integration traversal that invokes per-entry callbacks and sums two vectors of three doubles. The flight-step caller passes the force and moment accumulators at offsets `+0x300` and `+0x318`. The callback target set remains open. |
| `0x20bccef0` | Live verified weight and inertia calculation. Uses weighted position sums, squared offsets and mixed products, with the gravity scale at `0x20c7c180`. |
| `0x20c20430` | Live verified call from the flight step and separate engine-update branches. The dispatch reads the same `+0x46c` field offset used by the engine configuration reader. The mapping from branch values to complete engine models remains open. |
| `0x20c26d60` | Live verified `GeneralEngineData` reader. Reads `engine_type` and writes a byte at `+0x46c`. |
| `0x20c24da0` | Live verified writer for `piston_engine` configuration, including cylinder data, rated RPM and rated power. |
| `0x20c26020` | Live verified lookup and handling of the `turboprop_engine` configuration block. |
| `0x20c186a0` | Live verified bindings for fuselage drag, damping and stability coefficients. |
| `0x20c40020` | Live verified airframe geometry configuration. Accesses `airplane_geometry`, `wing_area` and `wing_span`. |
| `0x20c4b510` | Live verified configuration of contact-point options and the tailwheel lock. |
| `0x20c1a810` | Live verified brake configuration, including parking brake, toe-brake scale and autobrake. |
| `0x20c0c4d0` | Live verified autopilot configuration, including availability, flight director and autothrottle. Runtime control-law boundaries remain open. |
| `0x20bfefa0` | Live verified electrical configuration with entries for autopilot and fuel-pump circuits. |
| `0x20bd6390` | Live verified radio configuration and object setup. Reads COM, NAV, ADF, transponder and marker receiver options. |

These anchors describe verified operations or configuration access.
Most entries still have provisional subsystem ownership. Configuration
references establish a place to enter the subsystem.

## Suggested reconstruction order

### 1. Position and attitude

The world-velocity helper now has a full normalized linked match.
The two following coordinate helpers have matched code ranges.
See [the position-writer results](sim1-position-writers.md).

Start with the three-double helper at `0x20bd1d90`. The direct call at
`0x20bdec67` passes the flight object in ESI and 24 argument bytes.
The 86-byte target writes the vector at object offsets `+0x140`
through `+0x154`, then calculates the square root of the sum of squares
of the first and third components and stores the result at `+0x1ec`.
The original ends with `ret 0x18`. The _CIsqrt thunk target is
`0x20c66fc2`. This is a compact original-ABI comparison target and
also identifies the velocity input to the geographic step.

Then compare the longitude and altitude helpers and the attitude
writers. These calls are directly reachable from the known concrete
position and attitude slot.

| Helper entry | Call site in `0x20bdec10` | Verified operation |
| --- | --- | --- |
| `0x20bd1c90` | `0x20bded3e` | Clamps and publishes altitude and subtracts a virtual query result. The query's concrete target set remains open. |
| `0x20bd1bd0` | `0x20bded6e` | Wraps longitude and refreshes its trigonometric and fixed-point values. |
| `0x20bd1a30` | `0x20bded8e` | Folds latitude near the poles and can update longitude and heading. |
| `0x20bd1910` | `0x20bdee88` | Updates heading and its trigonometric values. |
| `0x20bd17a0` | `0x20bdee97` | Normalizes and publishes an attitude angle with a principal range of ±π/2. |
| `0x20bd1850` | `0x20bdeea6` | Wraps and publishes another attitude angle and refreshes sine and cosine. |

Reconstruct `0x20bdec10` after the helper contracts. The original
performs several x87 operations and uses virtual callbacks, so smaller
comparison scopes can isolate inputs, output fields and call ABI before
matching the complete function. Preserve the existing transform-method
and history-copy checks in the same workflow.

Proposed Ghidra variable names for `FUN_20bdec10` are `param_2` →
`flightState` based on the pointer passed in EBX to stored state and
`param_3` → `step` based on the multiplications before state writes.
These are provisional suggestions. No names or types were changed.

### 2. Force and mass inputs

The callback adapter and force collector now have complete linked
matches. The linked mass and acceleration bodies have matched
subranges. See [forces and moments](sim1-forces-and-moments.md).

Trace the callbacks from `0x20bccae0` and the weight/inertia update at
`0x20bccef0` into the acceleration stage at `0x20c42d90`. Their
caller sites in the flight step are `0x20c30db0` and `0x20c30cae`.
The vector accumulators already map to the force and moment reads in
the acceleration stage. The callback identities and the upstream
mass-property producers remain to be verified. The acceleration stage still has an ESI
instance ABI difference and the velocity integrator still has whole-body
code-generation differences. A linked context that preserves the
observed caller register use is the next matching experiment for that
pair.

### 3. Engine updates

The dispatch from `0x20c20430` is mapped from the configuration
reader through the piston, turbine, propeller and rotor paths.
The original jump table and per-engine and per-propeller force
inputs are live verified. The earlier hypothesis status is
superseded within that scope. The thrust projection and moment
helper at `0x20c01010` has a complete 215-byte normalized linked
match, including a 70-byte exact accumulation block.
[Engine branch mapping](sim1-engine-branches.md).

## Evidence and replay

- [Complete Ghidra function metadata](../../../artifacts/sim1.dll/sim1-complete-inventory/ghidra-functions.json)
- [Original PE imports and exports](../../../artifacts/sim1.dll/sim1-complete-inventory/pe-boundary.json)
- [Ghidra import, export and revision readback](../../../artifacts/sim1.dll/sim1-complete-inventory/ghidra-boundary.json)
- [Subsystem anchors, cross-references and selected listings](../../../artifacts/sim1.dll/sim1-complete-inventory/ghidra-anchors.json)
- [Current state-integration matches](sim1-state-integration.md)
- [Dashboard](../../../tools/re-dashboard/dist/index.html)

`scripts/ghidra/ExportFunctionMetadata.java` writes a fresh raw model
export from the explicitly selected program. It refuses to overwrite
an existing output. `node scripts/inventory-sim1.cjs` rechecks the
retained snapshot and original PE and regenerates the dashboard manifest
and PE boundary report. `node tools/re-dashboard/generate.cjs` rebuilds
the dashboard.

Ghidra stayed read-only. The module remained saved at revision
`1795526504922:962445d7:25` with `unsaved_changes=false` after the
inventory. This pass used the existing build evidence.
