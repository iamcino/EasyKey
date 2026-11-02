# EasyKey Browser Password Field Test Fixture

File fixture HTML này dùng để kiểm thử tích hợp thủ công và nghiệm thu tính năng **Browser Password Field Bypass** trên các trình duyệt:
- Google Chrome
- Microsoft Edge
- Mozilla Firefox
- Brave Browser
- Các ứng dụng nền Chromium / Electron (VS Code, Discord, Slack...)

---

## 1. Giới hạn và Phạm vi Hỗ trợ (Scope & Limitations)

> **Lưu ý quan trọng**:
> - Hỗ trợ tốt nhất cho các thẻ `input[type="password"]` và password control chuẩn mực trong Chrome, Edge, Firefox và Electron.
> - Một số control tùy chỉnh đặc thù (canvas-based inputs, virtual keyboards), shadow DOM lồng ghép phức tạp, hoặc iframe khác nguồn (cross-origin iframe không cấp quyền accessibility) có thể không được accessibility provider của trình duyệt công bố thuộc tính `UIA_IsPasswordPropertyId` / `STATE_SYSTEM_PROTECTED`. Trong các trường hợp này, EasyKey duy trì trạng thái gõ bình thường để không phá vỡ trải nghiệm người dùng.

---

## 2. Cách Chạy Kiểm thử Thủ công (Manual Matrix)

### Bước 1: Khởi động EasyKey với UIA bật
Nếu cần kiểm thử tính năng UIA trước khi bật mặc định:
- Có thể chạy bản build kiểm thử hoặc cấu hình registry `vPasswordFieldUIA = 1` tại `HKEY_CURRENT_USER\Software\EasyKey`.

### Bước 2: Mở file fixture trong các trình duyệt
Mở file `password_test_fixture.html` trực tiếp bằng trình duyệt:
```cmd
start chrome.exe "d:\projects\EasyKey\Sources\EasyKey\tests\fixtures\password_test_fixture.html"
start msedge.exe "d:\projects\EasyKey\Sources\EasyKey\tests\fixtures\password_test_fixture.html"
start firefox.exe "d:\projects\EasyKey\Sources\EasyKey\tests\fixtures\password_test_fixture.html"
```

---

## 3. Bảng Kiểm Thử Nghiệm Thu (Manual Test Matrix)

| Kịch bản | Thao tác kiểm thử | Kết quả mong đợi | Chrome | Edge | Firefox | Electron | Win32 / Notepad |
|---|---|---|:---:|:---:|:---:|:---:|:---:|
| **1. Standard Password** | Gõ `matkhau@123` vào ô `input[type="password"]` | Ký tự ra nguyên bản `matkhau@123`, không bị bỏ dấu tiếng Việt, không tự sửa khi có `@`. | PASS | PASS | PASS | PASS | PASS (Edit) |
| **2. Tab navigation (Normal -> Password)** | Đang ở ô Username, bấm `Tab` chuyển sang Password -> gõ ngay | Phát hiện password trong <60ms (PW_PENDING raw pass-through bảo vệ ký tự đầu), không bị dính dấu. | PASS | PASS | PASS | PASS | PASS |
| **3. Tab navigation (Password -> Normal)** | Đang ở ô Password, bấm `Tab` sang ô Username -> gõ `tieng viet` | Trở lại gõ tiếng Việt bình thường thành `tiếng việt`. | PASS | PASS | PASS | PASS | PASS |
| **4. Click trực tiếp vào Password** | Click chuột trái vào ô Password -> gõ mật khẩu | Nhận diện password ngay, phím ra raw. | PASS | PASS | PASS | PASS | PASS |
| **5. Show/Hide Toggle (Sticky Password)** | Gõ mật khẩu -> click "Hiện mật khẩu" (`type="text"`) -> gõ tiếp | Vẫn duy trì trạng thái raw (Sticky Password) trên cùng focus key, không bị hạ cấp sang tiếng Việt. | PASS | PASS | PASS | PASS | N/A |
| **6. Continuous Typing in Search/Omnibox** | Gõ liên tục văn bản tiếng Việt vào ô Search / Omnibox | Gõ mượt mà, gợi ý popup không gây giật caret hay ngắt buffer tiếng Việt (nhờ throttle <150ms). | PASS | PASS | PASS | PASS | N/A |
| **7. Contenteditable** | Gõ văn bản tiếng Việt vào ô `contenteditable` | Gõ tiếng Việt bình thường thành `văn bản phong phú`. | PASS | PASS | PASS | PASS | N/A |
| **8. Dynamic Injected Password** | Bấm nút chèn ô mật khẩu mới -> focus -> gõ mật khẩu | Reprobe nhận diện ô mới và bypass chuẩn xác. | PASS | PASS | PASS | PASS | N/A |
| **9. Nested Iframe Password** | Focus vào ô mật khẩu trong iframe -> gõ | Nhận diện password xuyên qua frame tree nội bộ. | PASS | PASS | PASS | PASS | N/A |
| **10. Shadow DOM Password** | Focus vào ô mật khẩu trong Web Component | Trình duyệt không expose thuộc tính password của node trong Shadow DOM lên UIA/MSAA focused element; EasyKey fail-safe giữ chế độ gõ bình thường (Known Limitation). | LIMITATION | LIMITATION | LIMITATION | LIMITATION | N/A |
| **11. Alt-Tab khi đang PW_PENDING** | Chuyển cửa sổ khi đang pending probe | Không bị kẹt raw, state tự về `PW_NORMAL` sau 60ms deadline. | PASS | PASS | PASS | PASS | PASS |
| **12. Lock/Resume / Session Change** | Khóa màn hình (`Win + L`) hoặc đổi user | Worker không treo, state về `PW_NORMAL` sau khi mở khóa. | PASS | PASS | PASS | PASS | PASS |

---

## 4. Tiêu chí An toàn & Zero Telemetry
- File fixture hoàn toàn offline, không kết nối mạng, không dùng CDN.
- Tuyệt đối không dùng `example.com` làm link hay endpoint.
- EasyKey UIA/MSAA worker chỉ đọc duy nhất cờ boolean `IsPassword` / `STATE_SYSTEM_PROTECTED`. Tuyệt đối không đọc `accName`, `accValue`, hay nội dung ký tự người dùng nhập.
