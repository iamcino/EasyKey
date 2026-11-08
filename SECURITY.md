# Chính sách bảo mật · Security policy

**Tiếng Việt** · [English](#english)

## Phiên bản được hỗ trợ

Chỉ bản phát hành mới nhất được sửa lỗi bảo mật. EasyKey tự báo khi có bản mới,
nên hãy cập nhật trước khi báo lỗi.

## Báo cáo lỗ hổng

Vui lòng **không** mở issue công khai cho vấn đề bảo mật. Hãy báo riêng qua
[báo cáo lỗ hổng riêng tư](https://github.com/iamcino/EasyKey/security/advisories/new)
của GitHub, hoặc gửi email tới duongtheson@proton.me.

Ghi rõ phiên bản EasyKey, phiên bản Windows (`winver`), các bước tái hiện và
ảnh hưởng bạn thấy. Bạn sẽ nhận phản hồi đầu tiên trong vòng 7 ngày. Khi bản
sửa được phát hành, thông báo bảo mật sẽ được công bố và ghi công bạn, trừ khi
bạn không muốn.

Những gì thuộc phạm vi: `EasyKey.exe`, quá trình cập nhật, và mọi lỗi có thể
làm lộ phím đã gõ, cho phép chương trình khác lạm dụng EasyKey (nhất là khi
EasyKey chạy quyền Admin), hoặc khiến EasyKey cài một file không phải bản phát
hành chính thức.

## Kiểm tra bản tải về

Mỗi bản phát hành được build công khai trên GitHub Actions từ mã nguồn trong
repo này, rồi được ký số và kèm các file để bạn tự kiểm chứng.

**Chữ ký số.** `EasyKey.exe` được ký Authenticode (ECDSA P-256, SHA-256) và
đóng dấu thời gian RFC 3161. Chứng chỉ là chứng chỉ tự cấp của dự án:

- Chứng chỉ công khai: [`docs/signing/EasyKey-codesign.cer`](docs/signing/EasyKey-codesign.cer)
- Thumbprint (SHA-1): `073FF7FEF09B3E51C8863CA5B2168A79D396976A`
- Kiểm tra trong PowerShell:
  ```powershell
  (Get-AuthenticodeSignature .\EasyKey.exe).SignerCertificate.Thumbprint
  ```
  Kết quả phải trùng thumbprint ở trên. Trạng thái chữ ký có thể báo "không
  tin cậy" vì chứng chỉ không nằm trong danh sách của Microsoft; điều đó là
  bình thường. **Đừng** cài chứng chỉ này vào Trusted Root của Windows.

Trình cập nhật trong EasyKey (từ bản 1.0.23) chỉ cài file có chữ ký từ đúng
chứng chỉ này. File bị tráo hay bị sửa sẽ bị từ chối.

**Mã SHA-256.** Mỗi bản phát hành kèm `SHA256SUMS.txt` và file `.sha256` cho
từng file tải về:

```powershell
Get-FileHash .\EasyKey.exe -Algorithm SHA256
```

**Chứng thực nguồn gốc.** Chứng minh file được build bởi GitHub Actions từ repo
này. Kiểm tra bằng [GitHub CLI](https://cli.github.com):

```
gh attestation verify EasyKey<phiên bản>-Windows.zip --repo iamcino/EasyKey
```

**Quét virus.** File `virustotal-<phiên bản>.json` trong mỗi bản phát hành có
đường dẫn tới kết quả quét trên VirusTotal.

Vì chưa dùng chứng chỉ thương mại, Windows SmartScreen vẫn có thể báo
"Unknown publisher". Xem [Cảnh báo của Windows](README.md#cảnh-báo-của-windows).

---

## English

### Supported versions

Only the latest release gets security fixes. EasyKey tells you when a new
version is out, so please update before reporting.

### Reporting a vulnerability

Please **do not** open a public issue for security problems. Report them
privately through GitHub's
[private vulnerability reporting](https://github.com/iamcino/EasyKey/security/advisories/new),
or by email to duongtheson@proton.me.

Include the EasyKey version, your Windows version (`winver`), steps to
reproduce and the impact you saw. You should get a first reply within 7 days.
Once a fix is released, the advisory is published and you are credited unless
you prefer otherwise.

In scope: `EasyKey.exe`, the update process, and anything that could leak
typed keys, let another program abuse EasyKey (especially when it runs as
administrator), or make EasyKey install a file that is not an official
release.

### Verifying a download

Every release is built in public on GitHub Actions from the source in this
repository, then signed and shipped with files you can check yourself.

**Digital signature.** `EasyKey.exe` is Authenticode-signed (ECDSA P-256,
SHA-256) and timestamped (RFC 3161). The certificate is the project's own,
self-issued certificate:

- Public certificate: [`docs/signing/EasyKey-codesign.cer`](docs/signing/EasyKey-codesign.cer)
- Thumbprint (SHA-1): `073FF7FEF09B3E51C8863CA5B2168A79D396976A`
- Check it in PowerShell:
  ```powershell
  (Get-AuthenticodeSignature .\EasyKey.exe).SignerCertificate.Thumbprint
  ```
  The output must match the thumbprint above. The signature status may say it
  isn't trusted, because the certificate is not on Microsoft's list; that is
  expected. **Do not** install this certificate into your Trusted Root store.

From 1.0.23 on, EasyKey's built-in updater only installs files signed with
exactly this certificate. A swapped or modified file is rejected.

**SHA-256 hashes.** Each release includes `SHA256SUMS.txt` and a `.sha256` file
for every download:

```powershell
Get-FileHash .\EasyKey.exe -Algorithm SHA256
```

**Build provenance.** Proves the file was built by GitHub Actions from this
repository. Check it with the [GitHub CLI](https://cli.github.com):

```
gh attestation verify EasyKey<version>-Windows.zip --repo iamcino/EasyKey
```

**Virus scan.** The `virustotal-<version>.json` file in each release links to
its VirusTotal results.

Because the certificate is not a commercial one, Windows SmartScreen may still
say "Unknown publisher". See [Windows warnings](README.en.md#windows-warnings).
