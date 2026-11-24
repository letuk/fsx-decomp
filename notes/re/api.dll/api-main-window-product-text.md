# API product text and shell launch

Target is original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
The contiguous original-byte inventory in
`artifacts/api.dll/api-product-text-flow-20261109.json` covers
`201205a6..20120869` with 691 instruction bytes and two 8-byte
descriptors. The existing native-flow verifier reports zero gaps.
Listings for the append helper, its bounded scan, and the shell
target add 262 independently checked instruction bytes.

## Product text at 201205a6

Export 16 takes HWND, context ID, destination and capacity as four
stack arguments. The destination is caller-owned and can already
contain text. Helper `2012054a` skips empty fields, adds `?` to an
empty destination or `&` otherwise, then appends the field. The
bounded scan at `201203f4` rejects an absent terminator. Insufficient
space propagates the bounded copy's error and can leave partial
output. The caller stops at the first negative append result.

The observed order is `product=FSX`, `ver=10.00`,
`bld=61637.00`, `lic=%d`, optional `contextID=%04lX`,
optional `cx=%d&cy=%d`, `lcid=%04lX` and optional `tbias=%d`.
Context formatting uses the entire DWORD after testing the second
byte against 2 and the low byte against 4. Client extents are added
for a valid HWND. GetUserDefaultLCID supplies the locale. A zero
GetTimeZoneInformation return skips the bias and preserves the
previous status. Nonzero results use the Bias field.

The stored Ghidra function listing omits the continuation through
the synthetic transfers. The entry transfer ends with RET 4 at
`20120694` and uses `201ecc2f` with descriptor `20120697`.
Execution resumes at `2012069f`. Negative EAX skips directly to
`201206e4` with the previous append status in EDI. Otherwise the
call at `201206a3` obtains EAX from protected target `2014852a`,
then saves it in EDI across the leave gateway `201ecc87` and
descriptor `201206dc`. Both descriptors contain
`b8 1d 00 00 00 00 00 00`. The operand bytes at `201206a4`
are interior to the call and were excluded as a misleading linear
decode. The implementation keeps the entire protected query as an
external source boundary, including the negative-entry fallback.
No protected value or runtime behavior is inferred from the
on-disk bytes at `2014852a`.

## Shell target at 2012629e

Live table field `200f2d44` points to this six-stack-argument
controller method. When byte +0x38 is set it synchronously sends
WM_COMMAND 0x1501 with LPARAM 0x50524f47 to its window. It tests
the caller HWND and substitutes its own HWND if invalid. It then
calls ShellExecuteA through `api.dll` IAT `200e1c70` and leaves
the result in EAX. The body is implemented as a source target.
The caller retains +0x64 virtual dispatch and the original table
remains a separate integration boundary. Shell32 internals were
outside this API-only package.

Suggested Ghidra names, unapplied. In `201205a6`, `param_1` →
`window` from IsWindow/GetClientRect. In `2012629e`,
`param_3` → `verb` and `param_4` → `file` from the ShellExecuteA
argument order.

## Native result

`api-main-window-product-text-20261109-001` passed 20 existing
comparators. Object SHA256 is
`8E5048FDCF1D03BE0BCD749B079808B545C504DCA7548570F2DF91A2C9B50AFC`.
The WM_COMMAND prefilter calls the product-text implementation,
which emits the protected-query reference. The ShellExecuteA
reference is present in the emitted shell target. No new accepted
bytes, linked runtime test or Ghidra edits.

The follow-up package implements the shared resource string loader
at `20138550`. See api-resource-strings.md.
