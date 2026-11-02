# Chính sách bảo mật · Security policy

**Tiếng Việt** · [English](#english)

## Phiên bản được hỗ trợ

Chỉ bản phát hành mới nhất được sửa lỗi bảo mật. EasyKey tự báo khi có bản mới,
nên hãy cập nhật trước khi báo lỗi.

| Phiên bản | Hỗ trợ |
|-----------|--------|
| Bản 1.0.x mới nhất | ✅ |
| Bản 1.0.x cũ hơn | ❌ |

## Báo cáo lỗ hổng

Vui lòng **không** mở issue công khai cho vấn đề bảo mật. Hãy báo riêng qua
tính năng
[báo cáo lỗ hổng riêng tư](https://github.com/iamcino/EasyKey/security/advisories/new)
của GitHub, hoặc gửi email tới duongtheson@proton.me.

Ghi rõ phiên bản EasyKey, phiên bản Windows (`winver`), các bước tái hiện và ảnh
hưởng bạn quan sát được. Bạn sẽ nhận phản hồi đầu tiên trong vòng 7 ngày. Khi
bản sửa được phát hành, thông báo bảo mật sẽ được công bố và ghi công bạn, trừ
khi bạn không muốn.

Phạm vi: `EasyKey.exe`, trình tự cập nhật và mọi lỗi có thể làm lộ phím đã gõ,
cho phép chương trình khác lạm dụng EasyKey (nhất là khi EasyKey chạy quyền
Admin), hoặc khiến EasyKey cài một file không phải bản phát hành chính thức.

## Chữ ký số Authenticode (Code Signing)

Các bản phát hành chính thức của EasyKey được ký số Authenticode (ECDSA P-256 + SHA-256) và đóng dấu thời gian RFC 3161 (DigiCert/Sectigo).

- **Chứng chỉ công khai**: [`docs/signing/EasyKey-codesign.cer`](docs/signing/EasyKey-codesign.cer)
- **Thumbprint (SHA-1)**: `073FF7FEF09B3E51C8863CA5B2168A79D396976A`
- **Kiểm tra chữ ký**:
  ```powershell
  Get-AuthenticodeSignature .\EasyKey.exe
  ```
- **Lưu ý**: Chứng chỉ hiện tại là self-signed nội bộ nhằm bảo đảm tính toàn vẹn và chống giả mạo nhị phân. Do chưa qua CA thương mại trong Microsoft Trusted Root Program, Windows SmartScreen vẫn có thể hiển thị cảnh báo "Unknown publisher". Tuyệt đối không cài đặt chứng chỉ vào Trusted Root của Windows.

---

## English

### Supported versions

Only the latest release gets security fixes. EasyKey tells you when a new
version is out, so please update before reporting.

| Version | Supported |
|---------|-----------|
| Latest 1.0.x | ✅ |
| Older 1.0.x | ❌ |

### Reporting a vulnerability

Please **do not** open a public issue for security problems. Report them
privately through GitHub's
[private vulnerability reporting](https://github.com/iamcino/EasyKey/security/advisories/new),
or by email to duongtheson@proton.me.

Include the EasyKey version, your Windows version (`winver`), steps to
reproduce and the impact you observed. You should get a first reply within 7
days. Once a fix is released, the advisory is published and you are credited
unless you prefer otherwise.

In scope: `EasyKey.exe`, the update process, and anything that could leak
typed keys, let another program abuse EasyKey (especially when it runs as
administrator), or make EasyKey install a file that is not an official
release.

### Authenticode code signing

Official release binaries are digitally signed with an Authenticode certificate (ECDSA P-256 + SHA-256) and timestamped via RFC 3161 (DigiCert/Sectigo).

- **Public certificate**: [`docs/signing/EasyKey-codesign.cer`](docs/signing/EasyKey-codesign.cer)
- **Pinned thumbprint (SHA-1)**: `073FF7FEF09B3E51C8863CA5B2168A79D396976A`
- **Verify signature**:
  ```powershell
  Get-AuthenticodeSignature .\EasyKey.exe
  ```
- **Note**: The current certificate is self-signed internally to guarantee binary integrity and prevent tampering. Because it is not yet issued by a commercial CA in the Microsoft Trusted Root Program, Windows SmartScreen may still display an "Unknown publisher" warning. Never install this certificate into your machine's Trusted Root Certification Authorities store.

