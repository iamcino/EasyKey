# Quyền riêng tư · Privacy

**Tiếng Việt** · [English](#english)

_Cập nhật lần cuối: 30/10/2026_

EasyKey không thu thập, lưu trữ hay gửi bất kỳ dữ liệu cá nhân nào cho tác giả.
Không có tài khoản, không thu thập dữ liệu sử dụng, không phân tích, không báo
cáo lỗi tự động và không quảng cáo.

## Phím bạn gõ

Bộ gõ nào cũng phải đọc phím bấm để bỏ dấu. EasyKey chỉ giữ trong bộ nhớ vài
phím của từ đang gõ (và từ ngay trước đó, cho chế độ Anh–Việt) để biết cần thêm
dấu gì, rồi bỏ đi. Phím không bao giờ được gửi qua mạng và không được ghi ra
file, trừ khi bạn tự bật nhật ký phím để báo lỗi (xem bên dưới).

## Dữ liệu nằm trên máy bạn

| Mục | Vị trí |
|-----|--------|
| Cài đặt, bảng gõ tắt, danh sách game | `HKCU\Software\EasyKey` |
| Chế độ Việt/Anh và bảng mã nhớ theo ứng dụng (tên file exe, ví dụ `chrome.exe`) | `HKCU\Software\EasyKey` |
| Chạy cùng Windows (nếu bật) | Giá trị `EasyKey` trong `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`, hoặc tác vụ `EasyKey` trong Task Scheduler khi bật chạy quyền Admin |
| Lối tắt trên màn hình nền (nếu bật) | `EasyKey.lnk` trên Desktop |
| Nhật ký phím (chỉ khi bạn tự bật `vDebugKeyLog`) | `%TEMP%\EasyKeyKeyLog.txt` |

**Gỡ EasyKey khỏi Windows** (Cài đặt › Nâng cao › Hệ thống) xoá toàn bộ những
mục trên, trừ nhật ký phím bạn tự bật.

## Các kết nối EasyKey tạo ra

- **Kiểm tra bản mới** (khi khởi động và 6 giờ một lần, trừ khi bạn tắt ở Nâng
  cao › Hệ thống): đọc file `version.json` của dự án trên
  `raw.githubusercontent.com` để biết số phiên bản mới nhất. Không gửi kèm gì
  ngoài yêu cầu tải file đó.
- **Cập nhật** (chỉ khi bạn đồng ý): tải `EasyKey.exe` và `EasyKey.exe.sha256`
  từ trang phát hành của dự án trên `github.com`.
- **Liên kết** trong cửa sổ Giới thiệu và Cài đặt chỉ mở trình duyệt khi bạn
  bấm vào.

GitHub thấy địa chỉ IP của bạn như mọi lần truy cập; xem
[tuyên bố quyền riêng tư của GitHub](https://docs.github.com/site-policy/privacy-policies/github-general-privacy-statement).
EasyKey không tạo kết nối nào khác.

---

## English

_Last updated: 30 October 2026_

EasyKey does not collect, store or send any personal data to its authors.
There is no account, telemetry, analytics, crash reporting or advertising.

### What you type

Every keyboard has to read key presses to add marks. EasyKey keeps only the
keys of the word being typed (and the word before it, for mixed mode) in
memory, to know which marks to add, and then drops them. Keys are never sent
over the network and never written to a file, unless you turn on the key log
for a bug report yourself (see below).

### What stays on your computer

| Item | Location |
|------|----------|
| Settings, shortcut list, game list | `HKCU\Software\EasyKey` |
| Vietnamese/English mode and encoding remembered per app (exe file name, such as `chrome.exe`) | `HKCU\Software\EasyKey` |
| Start with Windows (if on) | Value `EasyKey` under `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`, or Task Scheduler task `EasyKey` when running as administrator |
| Desktop shortcut (if on) | `EasyKey.lnk` on the desktop |
| Key log (only when you set `vDebugKeyLog` yourself) | `%TEMP%\EasyKeyKeyLog.txt` |

**Remove EasyKey from Windows** (Settings › Advanced › System) deletes all of
these, except a key log you turned on yourself.

### Connections EasyKey makes

- **Update check** (at start and every 6 hours, unless you turn it off under
  Advanced › System): reads the project's `version.json` from
  `raw.githubusercontent.com` to learn the latest version number. Nothing is
  sent besides the request for that file.
- **Update** (only when you agree): downloads `EasyKey.exe` and
  `EasyKey.exe.sha256` from the project's releases on `github.com`.
- **Links** in the About and Settings windows open your browser only when you
  click them.

GitHub sees your IP address, as with any visit; see the
[GitHub privacy statement](https://docs.github.com/site-policy/privacy-policies/github-general-privacy-statement).
EasyKey makes no other connections.
