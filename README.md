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

## Tính năng
- Kiểu gõ Telex, VNI, Simple Telex.
- Bảng mã Unicode dựng sẵn, Unicode tổ hợp, TCVN3 (ABC), VNI Windows, CP 1258.
- Đặt dấu kiểu mới (hoà, thuý) hoặc kiểu cũ (hòa, thúy).
- Kiểm tra chính tả, tự khôi phục phím khi gõ sai.
- Gõ nhanh, gõ tắt phụ âm đầu và cuối, gõ tắt (macro) không giới hạn độ dài.
- Nhớ chế độ Việt/Anh và bảng mã riêng cho từng ứng dụng.
- Sửa lỗi lặp chữ ở ô có gợi ý tự động, kể cả thanh địa chỉ Chrome và Edge.
- Công cụ chuyển mã văn bản.
- Giao diện Windows 11: hỗ trợ DPI cao, thanh tiêu đề tối, biểu tượng khay theo màu thanh tác vụ.
- Không hiện ô ngôn ngữ trên thanh tác vụ.

## Build từ mã nguồn
- Ứng dụng: mở `Sources/EasyKey/win32/EasyKey/EasyKey.sln` bằng Visual Studio 2022, build `Release|x64`.
- Test engine: `cmake -S Sources/EasyKey/tests -B build && cmake --build build && ctest --test-dir build`.

## Bản quyền
Copyright (C) 2026 Cino. Liên hệ: duongtheson@proton.me

EasyKey là phần mềm tự do, phát hành theo giấy phép [GNU GPL phiên bản 3](LICENSE). EasyKey được phát triển từ [OpenKey](https://github.com/tuyenvm/OpenKey), Copyright (C) 2019 Mai Vũ Tuyên, cũng theo GPLv3. Xem [NOTICE](NOTICE).
