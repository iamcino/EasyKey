# httpGet truncation audit

**File:** `Sources/EasyKey/win32/EasyKey/EasyKey/Updater.cpp`  
**Function:** `httpGet` (lines 61–99 as committed on `master` at `7666817503ab157d6afd9ea69492a4d06a3014b0`)  
**Auditor:** Viktor (Cursor Cloud Agent pilot)  
**Date:** 2026-11-12  
**Scope:** Report only — no source changes.

---

## 1. Verdict

**Yes.** `httpGet` can return `true` while `body` holds a truncated or incomplete response.

**Exact mechanism:** On HTTP 200, line 85 sets `ok = true` *before* the read loop finishes. The loop at line 86 is:

```cpp
while (WinHttpReadData(request, buffer, sizeof(buffer), &read) && read > 0)
```

If `WinHttpReadData` returns `FALSE` mid-transfer (network drop, reset, timeout, etc.), the `&&` short-circuits, the loop ends, and nothing clears `ok`. Lines 95–98 close handles and `return ok`, so the caller sees success with whatever bytes were already appended (lines 87). The only path that sets `ok = false` after line 85 is the `MAX_DOWNLOAD` overflow at lines 88–90.

There is **no** `Content-Length` (or equivalent) query and **no** comparison of received byte count to an expected length anywhere in `httpGet`.

---

## 2. Read-loop exit paths

| # | Condition | `ok` after exit | Body state | Correct? |
|---|-----------|-----------------|------------|----------|
| A | `WinHttpReadData` → `TRUE`, `read == 0` (normal EOF) | `true` (from L85) | Full body *if* the peer finished cleanly | Intended success; still unverified vs length |
| B | `WinHttpReadData` → `FALSE` mid-transfer | stays `true` | Partial appends so far | **Bug — success + truncated body** |
| C | `WinHttpReadData` → `TRUE`, `read > 0`, then `body.size() > MAX_DOWNLOAD` | set `false` (L89), `break` | Oversized partial body | Failure returned; body dirtied but callers gate on `false` |
| D | Never enter the status-200 block (crack/send/receive fail, or `status != 200`) | stays `false` (L71) | Unchanged (clear only at L82) | Correct failure |

Pre-loop returns: HTTPS crack failure at lines 68–69 returns `false` immediately.

---

## 3. Content-Length

**Not checked.** The function never calls `WinHttpQueryHeaders` with `WINHTTP_QUERY_CONTENT_LENGTH` (or reads that header any other way). Received size is never compared to an advertised length. Completeness is inferred only from the read loop terminating, which path B treats as success.

---

## 4. Call sites and truncated-body impact

### Call site 1 — `Updater::check` (line 131)

```cpp
if (httpGet(hosts[index].versionUrl, data) && parseLatestVersionName(data, newVersion))
```

`parseLatestVersionName` (`VersionInfo.cpp` 61–83) is a string search for `"latestWinVersion"` / `"versionName"` / a quoted value; it does not require well-formed trailing JSON.

- Truncation **before** a complete `"versionName": "x.y.z"` → parse fails → that host is skipped (mitigated for that attempt).
- Truncation **after** a complete version name string → parse can succeed → `workingHost` is updated, `found = true`, and a newer version can be offered from a partial `version.json`.

**Truncated body can be acted on** for update-check UX (offer / quiet notify), not for installing a binary.

### Call site 2 — `download` (line 247)

```cpp
if (httpGet(base + L"EasyKey.exe", job->exe) && httpGet(base + L"EasyKey.exe.sha256", checksum))
```

Then lines 256–264 compare SHA-256 of `job->exe` to the first 64 hex digits of the checksum file, require length 64, and require an `MZ` prefix; only then `install()` runs (which also Authenticode-pins).

- Truncated `EasyKey.exe` with an intact `.sha256` → hash mismatch → `DOWNLOAD_MISMATCH` → not installed.
- Truncated `.sha256` under 64 hex digits → `expected.size() != 64` → mismatch path.
- Truncated bodies are still stored in `job->exe` / `checksum` when `httpGet` wrongly returns `true`, but the checksum / MZ / signature gates keep a truncated exe from being installed under normal host content.

**Truncated exe body is not installed** given those post-checks; the false success still wastes a host attempt and can surface a mismatch error rather than a clean download failure.

---

## 5. Minimal fix (report only — not applied)

Treat a failed `WinHttpReadData` as failure; keep `ok = true` only after a clean EOF. Optionally enforce `Content-Length` when present:

```cpp
		if (status == 200) {
			body.clear();
			char buffer[16384];
			DWORD read = 0;
			DWORD contentLength = 0;
			DWORD lengthSize = sizeof(contentLength);
			bool haveLength = WinHttpQueryHeaders(request,
				WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
				WINHTTP_HEADER_NAME_BY_INDEX, &contentLength, &lengthSize,
				WINHTTP_NO_HEADER_INDEX) != FALSE;

			ok = true;
			for (;;) {
				if (!WinHttpReadData(request, buffer, sizeof(buffer), &read)) {
					ok = false; // mid-transfer read failure must not look like success
					break;
				}
				if (read == 0)
					break;
				body.append(buffer, read);
				if (body.size() > MAX_DOWNLOAD) {
					ok = false;
					break;
				}
			}
			if (ok && haveLength && body.size() != contentLength)
				ok = false;
		}
```

Smaller alternative: only split the `while` into an explicit `if (!WinHttpReadData(...)) { ok = false; break; }` — that alone closes path B.

---

## 6. Secondary issues inside `httpGet`

| Issue | Status | Notes |
|-------|--------|-------|
| `ok = true` before any successful read completes; empty 200 + immediate EOF returns `true` with empty `body` | **confirmed** | Call site 1: parse fails on empty. Call site 2: MZ / hash checks fail. Harmless given callers, still a weak success contract. |
| No `Content-Length` completeness check | **confirmed** | Defense-in-depth gap; primary bug is path B above. |
| `WinHttpQueryHeaders` status-code return value ignored (L79–80) | **confirmed** | `status` starts at 0; query failure keeps `status == 0`, so the 200 body path is skipped (fail-closed). |
| On `MAX_DOWNLOAD` failure, `body` retains oversized partial data while returning `false` | **confirmed** | Callers only proceed when `httpGet` is `true`; still a muddy out-parameter contract. |
| `body` not cleared on early failure paths (only cleared at L82 on 200) | **confirmed** | Prior contents can remain when returning `false`; callers gate on the bool. |
| Comment at L60 claims “following redirects”; no explicit redirect policy is set | **speculative** | WinHTTP GET defaults usually follow redirects; HTTPS→HTTP downgrade policy is not audited here. |
| Whether every transport abort surfaces as `WinHttpReadData == FALSE` vs a soft EOF | **speculative** | Path B is the documented WinHTTP failure mode for incomplete transfers; length check still recommended. |

---

## 7. Summary line

**Yes — `ok` is set true at L85 before the read loop; a mid-transfer `WinHttpReadData` failure leaves `ok` true and returns a truncated `body`, with no Content-Length check; check-site can act on a partial `version.json`, download-site is mostly saved by SHA-256/MZ/signature gates.**
