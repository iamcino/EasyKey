# EasyKey

**Tiếng Việt** · [English](README.en.md)

Bộ gõ tiếng Việt cho Windows 11. Chỉ một file `EasyKey.exe` chưa tới 1 MB, không cần cài đặt, giao diện theo phong cách Windows 11.

[**Tải EasyKey.exe mới nhất**](https://github.com/iamcino/EasyKey/releases/latest/download/EasyKey.exe) · [Trang phát hành](https://github.com/iamcino/EasyKey/releases/latest) · [Nhật ký thay đổi](CHANGELOG.md) · [Báo lỗi](https://github.com/iamcino/EasyKey/issues)

<p align="center">
  <img src="docs/screenshots/settings-light.png" alt="Cửa sổ Cài đặt, chế độ sáng" width="400">
  <img src="docs/screenshots/settings-dark.png" alt="Cửa sổ Cài đặt, chế độ tối" width="400">
</p>
<p align="center">
  <img src="docs/screenshots/flyout.png" alt="Popup khi chuyển Việt/Anh" width="300">
</p>

## Mục lục
- [Yêu cầu](#yêu-cầu)
- [Cài đặt](#cài-đặt)
- [Sử dụng](#sử-dụng)
- [Tính năng](#tính-năng)
- [Cập nhật và gỡ bỏ](#cập-nhật-và-gỡ-bỏ)
- [Xử lý sự cố](#xử-lý-sự-cố)
- [Tài liệu cho người phát triển](#tài-liệu-cho-người-phát-triển)
- [Bản quyền](#bản-quyền)

## Yêu cầu
| | |
|---|---|
| Hệ điều hành | Windows 11, bản 64-bit (x64). |
| Quyền | Không cần quyền Admin. |
| Phần mềm kèm theo | Không cần .NET, Visual C++ Redistributable hay thư viện nào khác. |
| Bộ gõ khác | Nên tắt UniKey, EVKey, OpenKey... vì hai bộ gõ chạy cùng lúc sẽ bỏ dấu hai lần. EasyKey nhắc khi thấy bộ gõ khác đang chạy. |

## Cài đặt
1. Tải [`EasyKey.exe`](https://github.com/iamcino/EasyKey/releases/latest/download/EasyKey.exe).
2. Chạy file vừa tải. Nếu Windows hiện cảnh báo, xem [Cảnh báo của Windows](#cảnh-báo-của-windows).
3. Lần đầu chạy từ thư mục Tải về, EasyKey đề nghị chuyển sang `%LOCALAPPDATA%\Programs\EasyKey` để không bị mất khi dọn thư mục. Bấm **Chuyển** là xong.

Cài đặt mặc định: kiểu gõ Telex, bảng mã Unicode, chạy cùng Windows, bấm **Alt + Z** để chuyển Việt/Anh.

Nếu máy đã dùng OpenKey, lần đầu chạy EasyKey tự chép cài đặt và bảng gõ tắt của OpenKey sang.

## Sử dụng
EasyKey chạy dưới khay hệ thống (góc phải thanh tác vụ). Biểu tượng là ô vuông có chữ **V** khi gõ tiếng Việt và chữ **E** khi gõ tiếng Anh.

| Muốn làm gì | Cách làm |
|---|---|
| Chuyển Việt/Anh | Bấm **Alt + Z**, hoặc bấm vào biểu tượng ở khay |
| Mở Cài đặt | Bấm đúp vào biểu tượng ở khay, hoặc bấm phải › Cài đặt… |
| Đổi kiểu gõ, bảng mã | Bấm phải vào biểu tượng ở khay › Kiểu gõ / Bảng mã, hoặc trong Cài đặt |
| Lấy lại phím vừa gõ khi bị bỏ dấu nhầm | Bấm **Esc** ngay sau từ đó (việt → vieejt) |
| Bật chế độ game cho game đang chơi | Khi đang ở trong game, bấm phải vào biểu tượng ở khay › Chế độ game cho … |
| Đổi phím tắt chuyển Việt/Anh | Cài đặt › Chuyển nhanh › Đổi phím |

Cửa sổ Cài đặt có hai phần:
- **Màn hình chính:** chọn Việt/Anh, phím tắt chuyển, kiểu gõ (Telex, VNI, Simple Telex).
- **Nâng cao:** các nhóm Gõ, Giao diện, Bảng mã, Chính tả, Gõ nhanh và gõ tắt, Ứng dụng, Hệ thống. Mỗi mục có một dòng giải thích ngay bên dưới.

## Tính năng
- **Kiểu gõ:** Telex, VNI, Simple Telex.
- **Bảng mã:** Unicode. Bảng mã cũ (Unicode tổ hợp, TCVN3, VNI Windows) nằm trong Nâng cao › Bảng mã, chỉ cần khi dùng font cũ.
- **Gõ thông minh:** chỉ bỏ dấu khi từ đúng chính tả tiếng Việt, gõ từ tiếng Anh thì giữ nguyên phím.
- **Gõ tắt** không giới hạn độ dài (ko → không), có sẵn vài từ thông dụng. **Gõ nhanh** (cc → ch, f → ph...) tuỳ chọn.
- **Nhớ theo từng ứng dụng:** Việt/Anh và bảng mã của mỗi ứng dụng được nhớ riêng.
- **Không lặp chữ** ở thanh địa chỉ Chrome, Edge, Brave và các ô có gợi ý tự động.
- **Chế độ game:** phím kỹ năng (Q, W, E, R…) đi thẳng vào game, bấm Enter mở khung chat là gõ được tiếng Việt.
- **Giao diện Windows 11:** sáng/tối, màu nhấn tuỳ chọn, tiếng Việt hoặc tiếng Anh, popup nhỏ khi chuyển Việt/Anh.
- **Không hiện ô ngôn ngữ ENG/VIE** trên thanh tác vụ, vì EasyKey không thêm bàn phím vào Windows.

## Cập nhật và gỡ bỏ
**Cập nhật.** EasyKey kiểm tra bản mới lúc khởi động và 6 giờ một lần. Khi có bản mới, biểu tượng ở khay có thêm mũi tên xanh. Bấm phải › **Cập nhật lên…** để cập nhật. EasyKey luôn hỏi trước, chỉ tải từ trang phát hành của dự án và kiểm tra mã SHA-256 trước khi thay file. Tắt tự kiểm tra trong Nâng cao › Hệ thống.

**Gỡ bỏ.** Vào Nâng cao › Hệ thống › **Gỡ EasyKey khỏi Windows**. EasyKey xoá mục chạy cùng Windows, lối tắt trên màn hình nền, cài đặt và bảng gõ tắt rồi thoát. Sau đó xoá file `EasyKey.exe`. EasyKey không ghi gì vào thư mục hệ thống.

## Xử lý sự cố

### Cảnh báo của Windows
EasyKey **chưa có chữ ký số**. Vì vậy SmartScreen có thể hiện "Windows protected your PC" (bấm **More info** › **Run anyway**), và phần mềm diệt virus có thể chặn `EasyKey.exe`. Bộ gõ nào cũng phải đọc phím bấm để bỏ dấu, nên dễ bị nghi ngờ khi chưa ký số.

Mã nguồn công khai. Mỗi bản phát hành được build trên GitHub Actions kèm chứng thực nguồn gốc (build provenance). Kiểm tra bằng [GitHub CLI](https://cli.github.com):

```
gh attestation verify EasyKey<phiên bản>-Windows.zip --repo iamcino/EasyKey
```

Nếu tin tưởng, bạn có thể thêm thư mục EasyKey vào danh sách loại trừ của phần mềm diệt virus.

### Kiểm tra chính tả tiếng Việt
Chữ tiếng Việt bị gạch đỏ là do trình kiểm tra chính tả của ứng dụng đang dò theo tiếng Anh, không phải lỗi của EasyKey:

- **Chrome, Edge, Cốc Cốc:** Cài đặt › Ngôn ngữ (Languages), thêm Tiếng Việt và bật kiểm tra chính tả cho Tiếng Việt, hoặc tắt kiểm tra chính tả tiếng Anh.
- **Word, Outlook, PowerPoint:** chọn đoạn văn, Review › Language › Set Proofing Language › Vietnamese (hoặc tích "Do not check spelling or grammar").
- **Notepad:** Cài đặt (bánh răng) › tắt Spell check.

Thêm tiếng Việt trong Cài đặt Windows › Ngôn ngữ cũng hết gạch đỏ, nhưng làm hiện lại ô ENG/VIE trên thanh tác vụ, nên không khuyên dùng.

### Không gõ được tiếng Việt trong một ứng dụng
- **Ứng dụng chạy quyền Admin** (Task Manager, trình cài đặt...): Windows không cho ứng dụng thường gửi phím vào đó. Bật Nâng cao › Hệ thống › **Chạy với quyền Admin**. EasyKey hiện thông báo khi bạn mở một ứng dụng như vậy.
- **Ứng dụng nhận sai chữ khi bỏ dấu** (mất chữ, thừa chữ): thử bật Nâng cao › Ứng dụng › **Gửi chữ qua Clipboard**. Cách này thay nội dung clipboard mỗi lần bỏ dấu, nên chỉ bật khi cần.
- **Game:** bật chế độ game cho game đó (xem [Sử dụng](#sử-dụng)).

### Gõ ra chữ sai
Nếu gặp chữ sai lặp lại (phím thô như "chaats", chữ hoa lẫn lộn...), bạn có thể ghi nhật ký phím để gửi kèm báo lỗi:

1. Mở PowerShell và chạy `reg add HKCU\Software\EasyKey /v vDebugKeyLog /t REG_DWORD /d 1 /f`.
2. Thoát EasyKey (bấm phải vào biểu tượng ở khay › Thoát EasyKey) rồi mở lại.
3. Gõ lại cho tới khi lỗi xảy ra. Nhật ký nằm ở `%TEMP%\EasyKeyKeyLog.txt`.
4. Tắt lại bằng `reg delete HKCU\Software\EasyKey /v vDebugKeyLog /f` và mở lại EasyKey.

Nhật ký ghi mã phím bạn bấm, nên chỉ bật khi cần, đừng gõ mật khẩu trong lúc bật, và xem lại trước khi gửi.

## Tài liệu cho người phát triển
- [Tài liệu thiết kế](docs/DESIGN.md): EasyKey hoạt động thế nào, cấu trúc mã nguồn, lưu cài đặt, cập nhật, phát hành.
- [Tự build](docs/BUILD.md): build bằng Visual Studio hoặc GitHub Actions, chạy test engine.
- [Nhật ký thay đổi](CHANGELOG.md).

## Bản quyền
Copyright (C) 2026 Cino. Liên hệ: duongtheson@proton.me

EasyKey phát hành theo [GNU GPL phiên bản 3](LICENSE), phát triển từ [OpenKey](https://github.com/tuyenvm/OpenKey) của Mai Vũ Tuyên (Copyright (C) 2019, GPLv3). Một số file EasyKey viết mới hoàn toàn được cấp thêm giấy phép [MIT](LICENSES/MIT.txt). Font giao diện Be Vietnam Pro dùng giấy phép [OFL 1.1](LICENSES/OFL-1.1.txt). Chi tiết trong [NOTICE](NOTICE).
