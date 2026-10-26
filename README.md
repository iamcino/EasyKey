# EasyKey
### Bộ gõ tiếng Việt cho Windows 11 (64-bit)
[Tải bản mới nhất](https://github.com/iamcino/EasyKey/releases/latest)

EasyKey là bộ gõ tiếng Việt nguồn mở, miễn phí, viết riêng cho Windows 11 bản 64-bit. EasyKey được phát triển tiếp từ [OpenKey](https://github.com/tuyenvm/OpenKey) của Mai Vũ Tuyên và phát hành theo giấy phép [GPLv3](LICENSE).

Khi dùng EasyKey, hãy tắt các bộ gõ khác (UniKey, EVKey, OpenKey...) vì hai bộ gõ chạy cùng lúc sẽ xung đột.

## Yêu cầu
- Windows 11, 64-bit (x64). Không còn bản 32-bit.

## Kiểu gõ
- Telex
- VNI
- Simple Telex

## Bảng mã
- Unicode dựng sẵn
- TCVN3 (ABC)
- VNI Windows
- Unicode tổ hợp
- Vietnamese Locale CP 1258

## Tính năng
- Đặt dấu kiểu mới (hoà, thuý) hoặc kiểu cũ (hòa, thúy).
- Kiểm tra chính tả, tự khôi phục phím khi gõ sai; bấm Ctrl để tạm bỏ kiểm tra cho một từ.
- Gõ nhanh (cc=ch, gg=gi, kk=kh, nn=ng, qq=qu, pp=ph, tt=th), gõ tắt phụ âm đầu và cuối.
- Gõ tắt (macro) không giới hạn độ dài, tự viết hoa theo từ gõ tắt.
- Nhớ chế độ Việt/Anh và bảng mã riêng cho từng ứng dụng.
- Viết hoa chữ đầu câu.
- Sửa lỗi lặp chữ khi ô nhập có gợi ý tự động (thanh địa chỉ trình duyệt, Excel).
- Công cụ chuyển mã văn bản giữa Unicode, VNI, TCVN3...
- Chạy cùng Windows, chạy với quyền Admin để gõ trong game, tự kiểm tra bản mới.
- Giao diện theo Windows 11: hỗ trợ màn hình DPI cao, thanh tiêu đề tối, biểu tượng khay tự đổi màu theo thanh tác vụ.

[Nhật ký thay đổi](CHANGELOG.md)

## Cài đặt
Tải file `EasyKey<phiên bản>-Windows.zip` ở trang [Releases](https://github.com/iamcino/EasyKey/releases), giải nén vào một thư mục cố định rồi chạy `EasyKey.exe`.

Lần chạy đầu, nếu máy đã có OpenKey, EasyKey tự chép cài đặt và bảng gõ tắt của OpenKey sang. Cài đặt của EasyKey lưu ở `HKEY_CURRENT_USER\Software\EasyKey`.

## Mã nguồn
- Engine gõ dùng chung: `Sources/EasyKey/engine`
- Ứng dụng Windows: `Sources/EasyKey/win32/EasyKey` (Visual Studio 2022, x64)
- Test engine: `Sources/EasyKey/tests` (CMake)

## Tác giả
- EasyKey: duongtheson@proton.me
- Góp ý và báo lỗi: [GitHub Issues](https://github.com/iamcino/EasyKey/issues) hoặc email ở trên.

## Ghi công
EasyKey dựa trên OpenKey, Copyright (C) 2019 Mai Vũ Tuyên ([tuyenvm/OpenKey](https://github.com/tuyenvm/OpenKey)), phát hành theo GPLv3. Các thay đổi so với OpenKey được ghi trong [CHANGELOG.md](CHANGELOG.md) và lịch sử git. Theo GPLv3, mọi bản phân phối lại của EasyKey cũng phải là mã nguồn mở theo cùng giấy phép.
