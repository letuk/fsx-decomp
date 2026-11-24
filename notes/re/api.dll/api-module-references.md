# API dependency and resource references

Consolidated 2026-11-08. These are bounded findings, not module-wide coverage.
Historical artifacts/shared/loaded-ghidra-modules-20261103.json recorded14 open programs.
Today's live list reports16 (fsx.exe plus15 DLLs), including g2d/g3d, api current.
No tabs changed. File identities: artifacts/shared/original-module-identities-20261103.json.
On-disk presence and an open/analyzed Ghidra program are distinct states.

Original language.dll SHA256
121D33C4206A34BC41E71A6F3747E0A2A166B55438AD9DF8836AAC8F6D554F72.
scripts/inspect-pe-string-resources.cjs was rerun today for IDs3,8,f455,f457.
Retained result: artifacts/language.dll/language-selected-strings-20261103.json, language1031.
ID3 is "Microsoft Flight Simulator"; ID8 is the out-of-memory message. IDf455
asks about changing the display to the formatted minimum resolution; API201213c4
supplies1024/720. IDf457 explicitly identifies unsupported24-bit monitors.
This corroborates title/geometry interpretation, not runtime locale/module choice.

Original g2d.dll SHA256
3CBAE511A0B229DD3D3E55D8A24867921C97B84F6A5EC0DAAA469D84CCDB5CD3,
base20660000. Raw PE artifact: artifacts/g2d.dll/g2d-selected-exports-20261103.json.
Unnamed ordinal1 resolves2069e920, ordinal4 resolves20698050, ordinal65 resolves
20699720. Ordinal4 begins MOV EAX,[20709bd8]; RET. These are file-verified export
identities/entry anchors, not behavioral names, complete implementations or DB
edits. Do not assign a name from an ordinal alone.

Current priority: the enumeration backend is internal to api.dll; loading other
DLLs does not resolve its register ABI. Follow CAB path/cache dependencies first.
