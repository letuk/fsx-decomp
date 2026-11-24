# API configuration enumeration and typed queries

Target: original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
The evidence is original-instruction verified. Nineteen listing exports
totaling 2303 bytes were compared against the pinned PE. Export ordinals,
the `%s.%d` format at `200f3100`, selected import identities and live
callers were also checked. Ghidra reported `changed=false`, modification 3.
The source names describe reconstructed roles. This slice covers api.dll.

## Export contracts

| Entry and ordinal | Observed behavior |
|---|---|
| `201316e6` / 62, `20131708` / 181 | Use the supplied path and selector to acquire the parser. Return section or section/key presence. Section selection can update the current section. |
| `2013174d` / 49, `201318cd` / 53 | Reject a null section, key or output with -1. Otherwise read through `2014369a` into 1024 bytes and return zero. Write the scalar only when the read returns a nonzero length, using atoi or strtod with null end pointer. |
| `201317dd` / 51, `2013195f` / 55 | Also reject zero count. With a nonzero read length, make exactly count conversions using base-zero strtoul or strtod and skip commas/spaces with strspn between conversions. No early stop is taken on conversion failure or end of string. With a zero read length, leave the list untouched. |
| `20131ab8` / 188 | Return the byte count from `20143744` in EAX. The helper walks section records in input order. It copies NUL-separated names and appends a final NUL if the emitted count is nonzero. Empty enumeration leaves the buffer untouched. |
| `20131adb` / 225 | Return zero for a null prefix. Otherwise test `prefix.0`, `prefix.1`, and so on, stopping at the first missing section. The last successful section can remain selected. |
| `20131a4d` / 57 | Return zero for null section/key/output or zero capacity. Return the parser's result directly for selector 0x2f or a zero-length read. Other positive-length reads run profile-string expansion and quote collapse. |
| `201308cb` / 39, `20130983` / 43, `201309c5` / 47 | Default configuration integer, double and string reads. The reconstructed string wrapper shares the explicit-path path with null path and selector zero. |
| `201308eb` / 200, `20130927` / 202 | Read the caller's initial WORD or BYTE before calling the integer query. On success write the low bits of the resulting integer, preserving the initial value if the lookup did not write. |

Assembly corrects the exported C's void return for the section-presence,
enumeration and scalar-wrapper entries. Each returns its callee's EAX.
It also confirms that `20130410` receives the path in ECX and the selector
on the stack. Pilot entry points use explicit stack arguments and preserve
the recorded export stack widths. The native object is a source-integration
artifact with no finalized export table.

For enumeration, the fit test compares used bytes plus name length plus one
against capacity. The truncation call receives `capacity - used - 1` as an
unsigned DWORD, and the helper writes a final NUL only when the resulting
used count is nonzero. The zero-capacity underflow on a nonempty table is
retained as an original path. There is no runtime verification of that edge.

## String postprocessing

`20131b08` searches for percent-delimited names and calls
GetPrivateProfileStringA on the `Strings` section in the parser's resolved
profile file. It tries the unquoted name first and the same name surrounded
by double quotes on a zero result. An unresolved pair is restored and
skipped. A successful replacement prepends the replacement to the remaining
suffix and resumes scanning at the replacement start. The original has no
expansion-cycle check. The source retains the 512-byte quoted-key copy
budget and uses a 513-byte scratch array to hold the final terminator when
the key fills that budget. This deliberately avoids modeling the original
one-byte write past the quoted-key region.

`20131cea` collapses doubled quotes in place, counts the emitted bytes
and writes a terminator at the end. The selector bypass covers both this
step and profile expansion. A missing value continues to use the existing
parser output rules.

## Native result

`api-stream-parser-queries-20261109-001` has object SHA256
`AC676B0AA075B4B862E227B77C90F1C8DDD4C6370DAFB2B742D55F60136F26DC`,
receipt exit code zero and 16 passing gates. One source build used the
retained compiler profile. COFF sizes include integer 143/144, double
145/146, integer list 248/240, double list 246/238, enumeration 154/167 and
string expansion 398/482 native/original bytes. The selected string export
is 107/107 but differs outside relocation fields. No new bytes are accepted.
Totals stay at 4972 complete code and 288 metadata bytes, with 123 partial
Initialize bytes separate. There was no linked runtime test.

Suggested Ghidra names, unapplied:

- `201317dd` `param_4` → `elementCount` and `local_40c` → `parseCursor`,
  from the loop bound and the conversion end-pointer write.
- `20131ab8` `param_2` → `outputCapacity`, from the direct forwarding into
  the enumeration fit test.
- `20131a4d` `param_6` → `suffixSelector`, from the parser acquisition
  call and the 0x2f postprocessing bypass.
