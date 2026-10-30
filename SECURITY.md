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
