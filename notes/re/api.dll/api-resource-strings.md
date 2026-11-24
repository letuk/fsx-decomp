# API resource string registry and loading

Live verified in original `api.dll`, SHA256
`9CD195AEADA1505F8CA73C6419AB0F03A8E7E7FC08550B754A4CC4030C370003`.
Seven retained instruction listings totaling 1337 bytes agree with the
original PE. The API IAT entry at `200e1a04` is USER32 LoadStringA.

`201f70b8` starts with count and capacity zero, growth 32 and stride
four. Export 152 at `201384ea` appends a module handle and returns the
same handle through the token output. Duplicate handles are permitted.
Export 153 at `20138509` finds the first matching four-byte token
through the direct specialization of `2013bfe4`. It returns -1 if
absent, otherwise removes the record and returns zero. The source
reuses the existing callback-array append, removal and iteration
bodies. It preserves the original reverse traversal and deferred
removal behavior. Module load and unload ownership remain separate.

Export 154 at `20138550` calls `2013bdee` in reverse order. Visitor
`201386ea` calls LoadStringA on the entry's module and increments
the request's +0xc counter on a nonzero result. The result stops
iteration. If every attempt returns zero, the loader tries the
language module at `2020e028`. EAX retains the successful length
or zero. Existing source call sites ignore the result, but their
declarations now describe the original return.

Export 155 at `201385a8` first clears the output pointer. It uses
the same lookup with capacity 1024. On a hit it allocates length
plus one bytes on the private heap and copies the string. It
retries allocation once and uses the original resource-8 error
path through shared scratch `2020d770` on repeated failure.
An absent string returns zero with the output pointer still null.

Suggested Ghidra names, unapplied. In `201386ea`, `param_1` →
`moduleRecord` and `param_2` → `stringRequest` from the module
dereference and request field accesses. In `201385a8`, `param_2` →
`allocatedText` from the null initialization and heap output store.

## Native result and startup scope

`api-resource-strings-20261109-001` passed 20 existing comparators.
Object SHA256 is
`C8C7AA369FE03B5E235EF0B8211FBCB371B956D0A9D0EA6BDEA5110B9D730DC0`.
The shared loader is now defined for the splash image, language
module, font resources, geometry error and WM_COMMAND source paths.
There is no new accepted byte range or linked runtime test. Ghidra
was used read-only. No other DLL body was analyzed or reconstructed.

This is a shared dependency of API initialization and window
messages. Resume the startup sequence at the API readiness and
multimedia timer setup in `20120d8d..20120ed4`, then connect the
callback at `201228f0` and the corresponding WM_CLOSE teardown.
See api-initialization-flow.md.
