# EasyKey

Bộ gõ tiếng Việt cho Windows 11 (64-bit).

[Tải bản mới nhất](https://github.com/iamcino/EasyKey/releases/latest) · [Nhật ký thay đổi](CHANGELOG.md) · [Báo lỗi](https://github.com/iamcino/EasyKey/issues)

## Yêu cầu
Windows 11 x64.

## Cài đặt
1. Tải `EasyKey<phiên bản>-Windows.zip` ở trang [Releases](https://github.com/iamcino/EasyKey/releases).
2. Giải nén vào một thư mục cố định và chạy `EasyKey.exe`.
3. Tắt các bộ gõ khác (UniKey, EVKey, OpenKey...) để tránh xung đột.

Nếu máy đã có OpenKey, lần chạy đầu EasyKey tự chép cài đặt và bảng gõ tắt sang. Cài đặt lưu ở `HKEY_CURRENT_USER\Software\EasyKey`.

## Cảnh báo của Windows khi chạy
EasyKey hiện **chưa có chữ ký số** (code signing). Vì vậy, khi tải về hoặc chạy:
- Windows SmartScreen có thể hiện "Windows protected your PC". Bấm **More info** › **Run anyway** để chạy.
- Windows Defender hoặc phần mềm diệt virus khác có thể chặn, cách ly hoặc xoá `EasyKey.exe`.

Đây không phải lỗi và cũng không phải vấn đề bảo mật của EasyKey. Bộ gõ nào cũng phải đọc phím bấm và gửi lại chữ đã bỏ dấu, nên phần mềm diệt virus dễ nghi ngờ các chương trình chưa có chữ ký. Mã nguồn công khai, và mỗi bản phát hành được build trên GitHub Actions kèm chứng thực nguồn gốc (build provenance). Bạn có thể tự kiểm tra file zip tải về bằng [GitHub CLI](https://cli.github.com):

```
gh attestation verify EasyKey<phiên bản>-Windows.zip --repo iamcino/EasyKey
```

Hãy cân nhắc trước khi dùng. Nếu tin tưởng, bạn có thể thêm thư mục EasyKey vào danh sách loại trừ của phần mềm diệt virus. Các bản sau sẽ được ký số khi có chứng chỉ.

## Tính năng
- Kiểu gõ Telex, VNI, Simple Telex.
- Bảng mã Unicode. Bảng mã cũ Unicode tổ hợp, TCVN3 (ABC), VNI Windows nằm trong mục Nâng cao.
- Đặt dấu kiểu mới (hoà, thuý) hoặc kiểu cũ (hòa, thúy).
- Kiểm tra chính tả, tự khôi phục phím khi gõ sai.
- Gõ nhanh, gõ tắt phụ âm đầu và cuối, gõ tắt (macro) không giới hạn độ dài.
- Nhớ chế độ Việt/Anh và bảng mã riêng cho từng ứng dụng.
- Sửa lỗi lặp chữ ở ô có gợi ý tự động, kể cả thanh địa chỉ Chrome và Edge.
- Công cụ chuyển mã văn bản.
- Giao diện Windows 11 gọn trong một trang, chế độ sáng/tối theo Windows, hỗ trợ DPI cao, biểu tượng khay theo màu thanh tác vụ.
- Không hiện ô ngôn ngữ trên thanh tác vụ.

## Build từ mã nguồn
- Ứng dụng: mở `Sources/EasyKey/win32/EasyKey/EasyKey.sln` bằng Visual Studio 2022, build `Release|x64`.
- Test engine: `cmake -S Sources/EasyKey/tests -B build && cmake --build build && ctest --test-dir build`.

## Bản quyền
Copyright (C) 2026 Cino. Liên hệ: duongtheson@proton.me

EasyKey là phần mềm tự do, phát hành theo giấy phép [GNU GPL phiên bản 3](LICENSE). EasyKey được phát triển từ [OpenKey](https://github.com/tuyenvm/OpenKey), Copyright (C) 2019 Mai Vũ Tuyên, cũng theo GPLv3. Xem [NOTICE](NOTICE).
