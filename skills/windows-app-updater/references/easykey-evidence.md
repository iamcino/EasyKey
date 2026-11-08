# EasyKey lessons and evidence limits

Recorded 2026-11-04 from the local EasyKey working tree based on commit `dc776af`. These are case-study observations, not a security certification or universal project defaults. This skill does not depend on that checkout and does not package its signing certificate or private material.

## Implemented and locally exercised

- Kept whole-file SHA-256 checksums and the existing release asset protocol.
- Embedded the public publisher certificate as RCDATA and added Windows verification with an isolated self-signed publisher trust engine, separate TSA chain, SHA-256 primary digest, and RFC 3161 timestamp requirement.
- Used random, exclusively created staging files and held handles through verification/promotion.
- Added failure when publishable releases lack signing credentials instead of publishing binaries the new updater would reject.
- Moved `.old.exe` deletion after engine/tray initialization to a worker: 120 attempts, 500 ms spacing, read-only handling, and retry on later launches. Previous behavior attempted deletion once after a window-based wait and could leave a locked backup.
- Exercised 351 engine tests, 17 signature tests, and 9 cleanup tests. The signature suite uses an embedded-signed SDK executable, not a real EasyKey release.

## Known gaps: do not teach these as finished solutions

- No full end-to-end acceptance with an actual EasyKey release signed by the pinned self-signed publisher on a clean machine without that root installed.
- No completed expired-signer/authenticated-timestamp, TSA revocation, or publisher-rotation fixture matrix.
- Chain retrieval is cache-only without online revocation checking. Do not describe this as proof that a certificate has not been revoked.
- `CreateProcess` failure recovery and a full readiness handshake remain unfinished. Current cleanup starts after engine/tray initialization; that is weaker than confirmed application readiness and does not prove survival of an immediate later crash.
- Signed metadata/anti-rollback and several HTTP/version/parser hardening items remain proposals.
- A signature test once failed at rename-and-reverify and passed on rerun. The precise failing OS operation was not isolated. Improve diagnostics and stability before treating the suite as reliably green.

## Release linker diagnosis that worked

The default MSBuild invocation selected `HostX86/x64/link.exe`. On this machine:

1. The same minimal `/GL` object linked successfully with `/DEBUG` and `/LTCG` using `HostX64/x64`.
2. `HostX86/x64` reproduced `LNK1101: incorrect MSPDB140.DLL version` on that object.
3. A full clean EasyKey Release build succeeded with `PreferredToolArchitecture=x64`.
4. Setting the project's default host tool architecture to x64 also made the ordinary MSBuild command succeed, producing both EXE and PDB without disabling debug symbols or LTCG.

`Platform=x64` chooses the output architecture; it does not necessarily choose the host architecture. Initialize the developer environment and inspect the actual tool path in a diagnostic log. A conditional project default can preserve explicit caller overrides:

```xml
<PreferredToolArchitecture Condition="'$(PreferredToolArchitecture)' == ''">x64</PreferredToolArchitecture>
```

This identifies a working toolchain selection, not the exact internal cause of the host-x86 PDB failure. DLLs beside both linkers had matching versions; an older `mspdbsrv.exe` also existed in the IDE directory, but its involvement was not proven. Do not infer that every LNK1101 has this cause. Do not download DLLs from third-party sites, copy them into System32, or disable PDB generation as an unexplained fix.
