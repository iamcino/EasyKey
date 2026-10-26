# Nhật ký thay đổi

**Tiếng Việt** · [English](CHANGELOG.en.md)

## Bản tiếp theo
- Mở bảng gõ tắt ngay trong cửa sổ cài đặt: "Nâng cao" › "Gõ" › "Bảng gõ tắt".
- Esc trả lại phím vừa gõ giờ chạy được trong khung chat web như Facebook (EasyKey giữ luôn lần nhả phím Esc, nên trang web không đóng khung chat).

## 1.0.0 (Windows 11 x64)
Bản đầu tiên của EasyKey.

- Chỉ hỗ trợ Windows 11 64-bit.
- Tự chép cài đặt và bảng gõ tắt từ OpenKey ở lần chạy đầu.
- Sửa lỗi không gõ được tiếng Việt trên máy có phần mềm gắn dữ liệu vào phím, ví dụ Bitdefender.
- Cửa sổ cài đặt mới kiểu Windows 11 năm 2026: nền Mica, màu nhấn theo Windows, hai nút lớn Tiếng Việt/Tiếng Anh, chọn kiểu gõ bằng một hàng nút, các tuỳ chọn khác gom vào mục "Nâng cao". Thay đổi có hiệu lực ngay. Chế độ sáng/tối theo Windows và đổi ngay khi Windows đổi.
- Thông báo nhỏ ở cuối màn hình khi chuyển Việt/Anh bằng phím tắt, có thể tắt trong "Nâng cao".
- Mặc định dùng Unicode. Bảng mã cũ (Unicode tổ hợp, TCVN3, VNI Windows) chuyển vào "Nâng cao" › "Bảng mã cũ". Bỏ CP 1258 khỏi danh sách, chỉ còn hiện cho ai đang dùng.
- Đặt phím chuyển Việt/Anh bằng cách bấm thẳng tổ hợp phím, ví dụ Ctrl + Shift hoặc Alt + Z.
- Menu khay có chế độ tối.
- DPI theo từng màn hình, font Segoe UI Variable, biểu tượng khay đơn sắc tự đổi màu theo thanh tác vụ.
- Phím chuyển mặc định khi cài mới là Alt + Z (trước đây bị đặt nhầm mã phím của macOS).
- Bỏ tuỳ chọn "gõ trong các ngôn ngữ khác" vì không có tác dụng trên Windows.
- "Sửa lỗi trên Chrome, Edge" bật mặc định; sửa lỗi xoá nhầm ký tự trước con trỏ khi viết hoa đầu câu.
- Bỏ tuỳ chọn "Hỗ trợ ứng dụng Metro" và "Biểu tượng hiện đại".
- Tooltip ngắn gọn hơn.
- Test engine gõ Telex/VNI chạy trên CI.
- Cửa sổ cài đặt gọn hơn (rộng 440 px). Màn hình chính chỉ còn chuyển Việt/Anh, phím chuyển và kiểu gõ; mọi tuỳ chọn khác nằm trong "Nâng cao".
- "Gõ thông minh" gộp kiểm tra chính tả và tự khôi phục phím khi gõ sai thành một công tắc.
- Bấm Esc ngay sau một từ để lấy lại phím vừa gõ, ví dụ "việt" thành "vieejt".
- Lần chạy đầu không hỏi gì, chỉ hiện thông báo nhỏ cách chuyển Việt/Anh; cửa sổ cài đặt không tự mở khi khởi động.
- Cảnh báo khi UniKey, EVKey, OpenKey, GoTiengViet hoặc bàn phím tiếng Việt của Windows đang chạy cùng (có "Không nhắc lại"). EasyKey không tự tắt ứng dụng khác.
- Trang "Có gì mới" hiện một lần sau mỗi lần cập nhật.
- Menu khay chỉ có chữ: Gõ tiếng Việt, Kiểu gõ, Bảng mã, Bảng gõ tắt, Công cụ chuyển mã, Cài đặt, Thoát.
- Giao diện tiếng Anh: chọn Theo Windows, Tiếng Việt hoặc English trong "Nâng cao" › "Giao diện". Áp dụng cho cài đặt, menu khay, thông báo và các hộp thoại.
- Chọn màu nhấn: theo Windows, 7 màu có sẵn (mỗi màu có sắc riêng cho chế độ sáng và tối) hoặc nhập mã hex riêng, tự tạo sắc cho chế độ tối và báo độ tương phản.
- Tự nối lại phím khi Windows ngắt hook của EasyKey (ví dụ sau khi máy chậm hoặc thức dậy từ chế độ ngủ).
- Không hiện thông báo chuyển Việt/Anh khi đang chơi game hay trình chiếu toàn màn hình.
- Báo một lần khi gõ vào cửa sổ chạy quyền Admin, vì Windows không cho EasyKey gõ vào đó.
- Nút "Gỡ EasyKey khỏi Windows": bỏ chạy cùng Windows, xoá lối tắt và cài đặt, rồi mở thư mục để xoá file.
- Cập nhật ngay trong EasyKey.exe: tải bản mới qua HTTPS, kiểm tra SHA-256, luôn hỏi trước khi cài và tự khởi động lại. Bỏ EasyKeyUpdate.exe.
- Bản phát hành có thêm `EasyKey.exe` và `EasyKey.exe.sha256` bên cạnh file zip.
- README có hướng dẫn hết gạch đỏ chính tả tiếng Việt theo từng ứng dụng và hướng dẫn tự build.
