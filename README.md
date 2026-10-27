# EasyKey

**Tiếng Việt** · [English](README.en.md)

Bộ gõ tiếng Việt gọn nhẹ cho Windows 11: một file `EasyKey.exe` chưa tới 1 MB, không cần cài đặt, giao diện theo phong cách Windows 11.

[**Tải EasyKey.exe mới nhất**](https://github.com/iamcino/EasyKey/releases/latest/download/EasyKey.exe) · [Trang phát hành](https://github.com/iamcino/EasyKey/releases/latest) · [Nhật ký thay đổi](CHANGELOG.md) · [Báo lỗi](https://github.com/iamcino/EasyKey/issues)

<p align="center">
  <img src="docs/screenshots/settings-light.png" alt="Cửa sổ Cài đặt, chế độ sáng" width="400">
  <img src="docs/screenshots/settings-dark.png" alt="Cửa sổ Cài đặt, chế độ tối" width="400">
</p>
<p align="center">
  <img src="docs/screenshots/flyout.png" alt="Popup khi chuyển Việt/Anh" width="300">
</p>

## Yêu cầu
- Windows 11, bản 64-bit (x64).
- Không cần quyền Admin, .NET hay thư viện cài thêm.
- Nên tắt các bộ gõ tiếng Việt khác (UniKey, EVKey, OpenKey...). EasyKey sẽ nhắc nếu thấy bộ gõ khác đang chạy.

## Tính năng nổi bật
- **Telex, VNI, Simple Telex**, bảng mã Unicode (bảng mã cũ TCVN3, VNI Windows, Unicode tổ hợp trong mục Nâng cao).
- **Gõ thông minh:** chỉ bỏ dấu khi đúng chính tả, bấm Esc để lấy lại phím vừa gõ (việt → vieejt). Có gõ nhanh và gõ tắt không giới hạn độ dài.
- **Nhớ Việt/Anh và bảng mã theo từng ứng dụng.**
- **Không lặp chữ** ở thanh địa chỉ Chrome, Edge và các ô có gợi ý tự động.
- **Giao diện Windows 11:** sáng/tối, chọn màu nhấn, tiếng Việt hoặc tiếng Anh, popup nhỏ khi chuyển Việt/Anh.
- **Chế độ game:** phím kỹ năng (Q, W, E, R…) không bị bỏ dấu, bấm Enter mở chat là gõ được tiếng Việt.
- **Không hiện ô ngôn ngữ ENG/VIE** trên thanh tác vụ.
- **Tự cập nhật** ngay trong ứng dụng (luôn hỏi trước, kiểm tra SHA-256), gỡ sạch bằng một nút.

## Cài đặt
1. Tải [`EasyKey.exe`](https://github.com/iamcino/EasyKey/releases/latest/download/EasyKey.exe) (hoặc file zip ở [trang phát hành](https://github.com/iamcino/EasyKey/releases/latest)).
2. Đặt vào một thư mục cố định rồi chạy.
3. Xong. Mặc định là Telex, Unicode, chạy cùng Windows, bấm **Alt + Z** để chuyển Việt/Anh.

Nếu máy đã có OpenKey, EasyKey tự chép cài đặt và bảng gõ tắt sang ở lần chạy đầu.

## Cảnh báo của Windows
EasyKey **chưa có chữ ký số**, nên SmartScreen có thể hiện "Windows protected your PC" (bấm **More info** › **Run anyway**), và phần mềm diệt virus có thể chặn `EasyKey.exe`. Lý do là bộ gõ nào cũng phải đọc phím bấm, nên dễ bị nghi ngờ khi chưa ký số.

<details>
<summary>Kiểm tra file tải về</summary>

Mã nguồn công khai, mỗi bản phát hành được build trên GitHub Actions kèm chứng thực nguồn gốc (build provenance). Kiểm tra bằng [GitHub CLI](https://cli.github.com):

```
gh attestation verify EasyKey<phiên bản>-Windows.zip --repo iamcino/EasyKey
```

Nếu tin tưởng, bạn có thể thêm thư mục EasyKey vào danh sách loại trừ của phần mềm diệt virus. Các bản sau sẽ được ký số khi có chứng chỉ.
</details>

<details>
<summary>Chữ tiếng Việt bị gạch đỏ chính tả</summary>

Gạch đỏ là do trình kiểm tra chính tả của từng ứng dụng đang dò theo tiếng Anh, không phải lỗi của EasyKey:

- **Chrome, Edge, Cốc Cốc:** Cài đặt › Ngôn ngữ (Languages), thêm Tiếng Việt và bật kiểm tra chính tả cho Tiếng Việt, hoặc tắt kiểm tra chính tả tiếng Anh.
- **Word, Outlook, PowerPoint:** chọn đoạn văn, Review › Language › Set Proofing Language › Vietnamese (hoặc tích "Do not check spelling or grammar").
- **Notepad:** Cài đặt (bánh răng) › tắt Spell check.

Thêm tiếng Việt trong Cài đặt Windows › Ngôn ngữ cũng hết gạch đỏ, nhưng làm hiện lại ô ENG/VIE trên thanh tác vụ, nên không khuyên dùng.
</details>

## Tự build
Xem [docs/BUILD.md](docs/BUILD.md).

## Bản quyền
Copyright (C) 2026 Cino. Liên hệ: duongtheson@proton.me

EasyKey phát hành theo [GNU GPL phiên bản 3](LICENSE), phát triển từ [OpenKey](https://github.com/tuyenvm/OpenKey) của Mai Vũ Tuyên (Copyright (C) 2019, GPLv3). Một số file EasyKey viết mới hoàn toàn được cấp thêm giấy phép [MIT](LICENSES/MIT.txt). Chi tiết trong [NOTICE](NOTICE).
