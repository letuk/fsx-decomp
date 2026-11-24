# API startup readiness and multimedia timer

Live and file evidence targets original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
The retained initialization flow supplies 328 PE-verified instruction
bytes at `20120d8d..20120ed5`. Live bytes for the untyped callback
`201228f0..2012294b` agree with 91 bytes in the PE. WINMM and USER32
IAT identities were checked from the PE. Names are reconstructed.

## Startup tail

The original reaches `20120d8d` after an earlier cancellation
check. It sets `2020dea0`, calls `2013e4d4` to populate version
fields and register a callback on owner `2020bb48`, sets
`2020e048` and invokes the callback at `2020e034` with one.
It clears `2020e040` and returns one immediately if the callback
has set the quit flag. That early return skips activation, timer
setup and the final posted message.

Otherwise it conditionally calls controller slot +0x28 based on
mode slot +4. Then it calls UpdateWindow, SetActiveWindow and SetFocus.
It requests TIMECAPS and selects the lower of the maximum period and
the greater of the minimum period and 50 ms. It starts a timer only
when the selected period is exactly 50 ms. The original calls
timeBeginPeriod(50) and timeSetEvent(50, 20, callback, 0, TIME_PERIODIC),
stores the returned timer ID and does not test either start result.
If TIMECAPS fails or the period differs from 50 ms, the tail sets the
quit flag and posts WM_COMMAND 0x110a with LPARAM one. Both setup paths
post 0x46b and return one. WM_CLOSE uses the same period and timer ID.

The source tail is callable after the original pre-readiness
condition. The outer Initialize wrapper still references the
unfinished larger initialization body. The version callback
`2013e44a` also remains an explicit dependency.

## Timer and messages

The 91-byte callback at `201228f0` ends in RET 0x14 and uses
none of its five stack arguments. It posts 0x465 only if mode slot
+0x30 returns a signed value <= 1 and at least one of `2020e068`
and `2020e08c` is nonnull. Independently it decrements `201f7160`
when gate `2020e03c` is zero, except for counter values zero and -1.
The source preserves both decisions. The callback posts work to the
window thread. The FS98MAIN 0x465 route now calls the reconstructed
`20137b6b`, then forwards the message to DefWindowProcA.
For 0x46b the source enumerates the current thread's windows and
sends 0x467 to class FS98FLOAT before the default procedure, matching
`2012f3e8` and visitor `20125085`.

Suggested Ghidra names, unapplied. At `201228f0` use
`MainWindowMultimediaTimer` from the timeSetEvent argument and
RET 0x14. In the initialization tail use `timerCapabilities`
for the local output at ESP+0x24 based on timeGetDevCaps and the
following min/max reads.

## Native result

`api-startup-timer-20261109-001` passed all 20 existing comparators.
Object SHA256 is
`107B1136C0473D999F69649E8A37273E6DB506C910C397CC849F36D0CB62E17B`.
The object emits the five-argument callback, the timeSetEvent
reference and both message routes. No added accepted bytes, linked
runtime test or Ghidra edits. WINMM remains an import boundary.

See api-timer-window-work.md for the 0x465 follow-up.
