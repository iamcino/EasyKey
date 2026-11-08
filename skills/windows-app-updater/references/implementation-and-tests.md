# Windows verification, recovery, and acceptance

Read the sections relevant to the current task. This is a design and verification guide, not a prevalidated drop-in verifier.

## Authenticode policy

Record these decisions before implementing:

| Decision | Questions to resolve |
|---|---|
| Publisher pin | Full certificate DER or reviewed public-key pin? How is it embedded and rotated? |
| Signatures | Embedded signature only? One primary signer or explicitly supported multiple signatures? |
| Digest | SHA-256 for the primary signature; what timestamp algorithms are accepted? |
| Timestamp | RFC 3161 required? Does the token bind to the primary signature? |
| Time validity | Was the signer valid at the authenticated timestamp? What happens after expiry? |
| TSA trust | Which trust engine/roots and EKU policy apply independently of publisher pinning? |
| Revocation | Online, cache-only, or explicitly absent? How are revoked/unknown/stale/offline states handled? |

Important API semantics:

- `WinVerifyTrust` returns `LONG`; success is exactly zero. Do not use `SUCCEEDED()`.
- `CertVerifyCertificateChainPolicy` returning `TRUE` means it performed the check. Acceptance also requires `CERT_CHAIN_POLICY_STATUS.dwError == 0`.
- `Get-AuthenticodeSignature` returning a signer and timestamp certificate does not prove that all required integrity, trust, time, and binding checks passed. Broad exceptions for `UnknownError` or `NotTrusted` require a separate rigorous verification path; matching a pin is not that path by itself.
- Verify the Authenticode content digest and CMS signature, not just the X.509 certificate. Reject malformed pins and unsupported or ambiguous signature layouts.
- For self-signed publisher trust, an isolated chain engine with an exclusive in-memory root store can express local trust without global root installation. It must still enforce the intended usage/time/signature policy. Do not reuse that private root for the TSA chain.
- WinTrust generic-chain callbacks are specialized machinery. Validate callback invocation, earlier trust-step errors, signer/chain policy, TSA policy, and authenticated verification time with real fixtures. Do not broadly ignore chain errors to make a test pass.
- A recognized RFC 3161 attribute is necessary for an RFC 3161-only policy but is not sufficient proof of token validity or binding.
- Distinguish the Authenticode PE digest from a whole-file SHA-256 checksum; they are not interchangeable.

Do not import roots or contact online services just to make fixtures pass. If policy needs network revocation, declare that behavior, bound it, and test the offline outcome. Pin the publisher at build time; use private keys only in separately authorized signing operations.

## Update transaction

Design states and recovery before implementing file mutations. An illustrative sequence is:

1. Validate bounded metadata and choose a compatible newer version.
2. Download, verify checksum, stage uniquely, and verify the held file's signature/publisher/timestamp.
3. Persist enough transaction identity to distinguish current, backup, and staging artifacts if crash recovery needs it.
4. Release input hooks and coordinate single-instance ownership before handoff.
5. Rename current to backup, then promote the held verified staging file. Check both operations and every attempted reversal.
6. Start the new executable with scoped handoff information. On failure, restore a usable version or leave documented recovery artifacts; do not report success and unconditionally exit.
7. Await a readiness signal bound to this update, with a deadline. Avoid a deadlock where the child waits for the parent to exit while the parent waits for child readiness.
8. Cleanup the backup only after readiness and old image release. On failure, retain it and retry later.

Adapt ordering to the application. `CreateProcess` success, a new window, and tray initialization are different readiness levels; choose and test the required level rather than assuming they are equivalent.

Staging should use same-volume atomic operations where available. Unique names plus exclusive creation avoid truncating an existing file/link. Keep file identity locked through verification and promotion, or use an equally strong validated mechanism. Test actual filesystem APIs and sharing semantics on the target Windows version.

Cleanup details:

- Use a worker with configurable, bounded retries. About one minute was an EasyKey choice, not a universal requirement.
- A missing backup is success. A file still locked or inaccessible at the deadline is a retained artifact, not successful deletion.
- Treat read-only metadata separately from ACLs. Do not reset directory permissions, kill unrelated processes, or schedule broad deletions to force cleanup.
- Bind cleanup to the intended installation/transaction; avoid deleting a newer backup created by a different transaction.
- Preserve backup when readiness is not confirmed. Document cleanup of abandoned staging artifacts separately.

## Test matrix

| Scenario | Expected observable result |
|---|---|
| Project-signed EXE, required timestamp, matching embedded pin | Accepted on a clean target machine without installing the publisher root |
| Unsigned, wrong publisher, malformed pin, modified signed content | Rejected before current/backup mutations |
| Missing, tampered, wrong-digest or unbound timestamp | Rejected under the configured timestamp policy |
| Expired signer + timestamp from its valid period | Accepted if the chosen policy and TSA validation permit it |
| Signer invalid at timestamp | Rejected |
| Revoked, stale-cache, unknown/offline revocation | Explicit policy outcome; never mislabeled as fully checked |
| Multiple/nested/catalog signatures | Explicit supported behavior or rejection; no accidental acceptance |
| Truncated HTTP, read error, oversized metadata/file, insecure redirect | Rejected, no installation |
| File swapped after verification | Replacement prevented or detected before install |
| Transient write/delete lock, read-only backup | Bounded retries; later cleanup succeeds when permitted |
| Permanent lock/ACL denial | Backup remains; retry/recovery state is visible in diagnostics |
| Disk full or crash at each rename/flush boundary | At least one recoverable executable remains |
| `CreateProcess` failure, readiness timeout, early child crash | No false success and no premature backup cleanup |
| Legacy client skips bridge release | Update still works through retained assets or explicit bridge routing |

Use fault injection for filesystem/process failures. Record fixture origin and expected certificate/timestamp properties. SDK tools can provide genuine signed fixtures, but a Microsoft-signed positive fixture cannot prove pinned self-signed publisher behavior. Catalog-signed system executables are unsuitable for embedded-signature-only tests. Differentiate test infrastructure failures and intermittent file locks from deterministic product failures; log error codes instead of retrying until a green run hides the issue.

## References

- [WinVerifyTrust](https://learn.microsoft.com/en-us/windows/win32/api/wintrust/nf-wintrust-winverifytrust)
- [Certificate chain policy](https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certverifycertificatechainpolicy)
- [SignTool](https://learn.microsoft.com/en-us/windows/win32/seccrypto/signtool)
- [VirusTotal file report identifiers](https://docs.virustotal.com/reference/file-info)

Check current primary documentation when changing platform/API assumptions; never upload internal source for research.
