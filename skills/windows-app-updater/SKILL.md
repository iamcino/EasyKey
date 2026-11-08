---
name: windows-app-updater
description: Design, implement, or review native Windows EXE self-updates using SHA-256, Authenticode publisher pinning, staged replacement, recovery, and backup cleanup. Use for in-place application updaters, not generic hashing, certificate issuance, or installer-managed MSIX/MSI updates.
---

# Windows App Updater

Build an updater that rejects unauthentic downloads and preserves a recoverable application across failures. Adapt the workflow to the project's existing lifecycle, trust model, and deployment channel. Do not treat an implementation example as proof that its security policy has been validated.

## Establish the project contract

- Read repository instructions and inspect branch/status/latest commit before changing a checkout. Preserve uncommitted work and use the local checkout as the implementation source.
- Identify the executable layout, installation permissions, supported Windows/CPU targets, single-instance mechanism, hooks, startup sequence, update endpoints, signing pipeline, and existing clients' expectations.
- Keep authorization scoped: implementing an updater does not authorize pushing, publishing, signing with a private key, installing certificate roots, repairing system software, or uploading binaries to analysis services. Follow existing authorization; ask only when an actual additional action needs it.
- Establish acceptance criteria and report implemented, tested, and untested behavior separately. If the user asks only for a review or design, do not silently implement the whole workflow.

## Trust and download

- Keep SHA-256 as the interoperable default for release checksums, Authenticode digests, and service identifiers. A faster hash does not authenticate a publisher. Preserve legacy checksum assets while clients still depend on them.
- Download over HTTPS with bounded metadata/binary sizes, timeouts, explicit read-error handling, and HTTPS-only redirects. Treat version metadata as untrusted input and validate its grammar/schema.
- A checksum fetched next to an EXE detects corruption; compromise of both files bypasses it. Verify the executable signature independently against a publisher identity embedded in the trusted application.
- Embed the project's own public pin at build time. Do not trust a replacement `.cer` or pin downloaded alongside the update, and never copy EasyKey's certificate into another project's trust policy.
- Require verified Authenticode integrity, an allowed signer/digest policy, and a cryptographically verified timestamp if the release policy requires timestamping. Certificate presence and matching thumbprints alone are insufficient.
- For pinned self-signed publishers, use isolated trust in memory rather than asking end users to install roots. Verify TSA trust separately; publisher trust must not become TSA trust.
- Define certificate expiry, revocation, offline/cache behavior, and key rotation explicitly. Do not silently downgrade verification on errors. Read [implementation-and-tests.md](references/implementation-and-tests.md) before implementing Windows verification or changing these policies.

## Stage, replace, and recover

- Perform downloads, signature checks, and retrying file work away from keyboard-hook/UI threads.
- Stage in the destination filesystem using a unique name and exclusive creation. Hold the verified file identity through installation; prevent replacement between verification and rename.
- Check all writes/flushes/renames and retain at least one executable or verified staging artifact that can recover the installation. Describe rollback as attempted unless its result is checked.
- Before restarting, release hooks and coordinate single-instance ownership. If process creation fails, preserve recovery artifacts and restore the current process's usable state where possible.
- Define readiness beyond successful `CreateProcess`: signal that the new process has completed required initialization. Keep the backup until that condition is met; a bounded timeout must retain the backup and enter a documented recovery path.
- Cleanup runs after readiness on a worker with bounded retries for transient locks. Retry on later successful launches when needed. Handle read-only files narrowly; never recursively delete directories or broaden permissions merely to remove a backup. Do not delete the only recoverable copy.
- Coordinate old-process exit and cleanup using process handles or a scoped transaction handshake; window disappearance alone does not prove the image is unlocked. Avoid long main-thread waits after hooks are installed.

## Release compatibility and validation

- Generate checksums after signing and timestamping the final bytes. Keep Authenticode/TSA and VirusTotal's supported identifiers even if a separate updater hash changes.
- A single dual-checksum release does not migrate users who skip it. Keep old assets available, or route legacy clients through a retained bridge endpoint/release. Plan publisher-pin rotation before changing keys.
- Gate publishable releases on successful signing/verification when clients require signed updates. A regular unsigned local build can remain useful for development.
- Run meaningful OS integration tests for signed/unsigned/tampered files, wrong pins, timestamp failures, locked staging, process-start failure, rollback, readiness, and cleanup. Use signed fixtures with provenance; synthetic state-machine tests do not prove WinTrust or filesystem behavior.
- Keep external effects within scope. Offline fixtures should not unexpectedly contact a TSA/CRL server or modify machine certificate stores.
- If Release linking reports `MSPDB140.DLL`, isolate a minimal `/DEBUG` + `/LTCG` link and compare host tool architectures before changing application code or installing software. Read [easykey-evidence.md](references/easykey-evidence.md) for the observed HostX86/HostX64 distinction; it is a diagnosis to test, not a universal DLL repair.

## Completion report

State what changed, actual build/test results, remaining gaps, and the location of recovery/rollout notes. Do not call the updater production-ready solely because it builds or passes mock tests. For concrete examples and known unfinished work from EasyKey, read [easykey-evidence.md](references/easykey-evidence.md).
