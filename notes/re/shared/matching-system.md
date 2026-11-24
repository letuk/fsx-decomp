# Bounded host/XP recompilation system

2026-11-01. Replaces the one-click-per-experiment BUILD.bat workflow. The first
target is the remaining ShowErrorMessage difference, not matching all of startup.

## Ownership and folders

- Canonical source remains in src/. Generation never edits it.
- scripts/matching/start-search.ps1 snapshots canonical source, generates a finite
  set of ordinary C++ variants, dispatches one job at a time and evaluates results.
- scripts/matching/guest-build.cmd contains the pinned SP1 compile/link commands.
- scripts/matching/worker.vbs is the canonical XP worker, deployed in vm-share/.
- In XP run vm-share/RUN-WORKER.bat once and leave it open. Guest builds are local
  under C:\fsx-build\matching-jobs, not performed across the shared folder.
- vm-share/modules/fsx/runs/<id>/ contains REPORT.md, status.json, results.csv, snapshots and
  candidate artifacts. vm-share/queue/ carries ready/working/done jobs and receipts.
- vm-share/modules/fsx/archive/ preserves all previous manual evidence, including the verified
  experiment23 baseline. Legacy input/results folders moved beneath archive/.

## Strategy and match boundary

The first generator has at most 24 proposals: signed int/HRESULT/long formatter
result types, optional zero initialization, and four equivalent success predicates.
These are local source formulations preserving the reviewed direct-copy behavior.
No inline assembly, binary patch, arbitrary constant masking, or runtime execution
of candidate binaries is used. Existing five comparison scripts verify the target,
version helpers, copy/formatter helpers, OLE initializer and its dependencies.

The host pins original SHA256
61FAE3F2B6F4338BB9D45FCC87A8FCAA1C56876088BFD9DD2BF950B1C5A57735
and requires the SP1 compiler banner 14.00.50727.762. Every job uses a new local
directory. Guest publishes the actual compiled source copies; host verifies them
against pre-dispatch hashes, so stale or mismatched inputs are rejected. PDB/MAP,
disassembly, log and executable are required. Helpers must retain their established
raw/address-normalized matches before target differences count toward best result.

Target match means exact body bytes after the existing sixteen independently
checked address mappings, with all regression checks passing. RTTI, external DLL
implementation bytes, whole-file equality and runtime behavior remain outside the
established comparison scope. Structural changes that invalidate strict range or
reference mappings are rejected, not assigned a fabricated score.

## Stop and failure handling

Terminal states: matched, exhausted, stopped, build_failed, baseline_failed,
timed_out, error. Exhausted means the implemented proposals were exhausted, not
that no possible source can match. Source hashes deduplicate identical proposals.
Compilation failure, changed compiler identity, incomplete receipt, mismatched
compiled source or missing output stops dispatch. Unmappable non-baseline variants
are rejected and logged; an invalid baseline stops immediately.

Only one controller per share can hold vm-share/controller.lock. A user STOP file is
checked before every new dispatch and while waiting. scripts/matching/stop-search.ps1
creates it safely for a named run. Worker can be stopped with Ctrl+C or a shared
STOP-WORKER file. Host timeout is fifteen minutes per job; it stops dispatch but
does not forcibly terminate an active guest compiler. Crashed claimed jobs are
preserved without automatic retry. A new run is needed after investigating errors.

This is an explicitly launched local process, not a recurring Codex automation or
an unlimited model-generated rewrite loop. To explore InitInstance, add a reviewed
generator and a comparator that handles its current structural/layout differences;
do not reuse ShowErrorMessage's fixed reference offsets for that function.

## Validation performed

- PowerShell scripts parsed successfully; candidate staging and source snapshots
  checked without running the guest.
- Windows Script Host /validate mode checked VBScript syntax without building or
  touching the guest. Worker source/deployed-file hashes agree.
- Integration replay fixture used saved real experiment23 artifacts to test two
  sequential jobs, regression evaluation, best tracking and exhausted stop state.
  These are orchestration tests, NOT newly compiled candidates or new matches.
- Guest absence produced timed_out with no further dispatch.
- Replayed nonzero compile result produced build_failed with no next candidate.

Actual XP worker/build validation and live search outcome are recorded separately
in ../STATUS.md and the live run reports; do not confuse replay tests with compilation.

## First live result
show-error-search-001 completed all24 XP builds and stopped as exhausted. Twelve original-length437-byte variants retain two instruction bytes of difference. Six438-byte and six439-byte variants were rejected by the strict fixed-offset target comparator. Independent length review excludes exact equality for those layouts, but no instruction-aligned ranking is available. This identifies a real comparator limitation to improve before larger InitInstance searches. Source/receipt/SP1/regression handling worked end-to-end. See vm-share/modules/fsx/runs/show-error-search-001/REVIEW.md and review.csv. Direct vmrun-based guest launch remains deferred.

## 2026-11-02 worker restart smoke test
User manually started RUN-WORKER.bat; the host dispatched worker-smoke-20261102-001 with MaxCandidates1. One real guest build completed successfully and all existing comparisons retained their results. Worker heartbeat building/ready and retained receipts verified. Status exhausted reflects the one-proposal limit, not a failed build. No automatic guest launch or XP autostart was installed; direct vmrun execution remains blocked by the empty guest password. Subsequent jobs can be queued entirely from the host while this worker stays running.
