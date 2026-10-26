# EasyKey

**Tiếng Việt** · [English](README.en.md)

Bộ gõ tiếng Việt cho Windows 11 (64-bit).

[Tải bản mới nhất](https://github.com/iamcino/EasyKey/releases/latest) · [Nhật ký thay đổi](CHANGELOG.md) · [Báo lỗi](https://github.com/iamcino/EasyKey/issues)

## Yêu cầu
Windows 11 x64.

## Cài đặt
1. Tải `EasyKey<phiên bản>-Windows.zip` ở trang [Releases](https://github.com/iamcino/EasyKey/releases).
2. Giải nén vào một thư mục cố định và chạy `EasyKey.exe`. Không cần thiết lập gì: mặc định là Telex, Unicode, chạy cùng Windows, bấm Alt + Z để chuyển Việt/Anh.
3. Tắt các bộ gõ khác (UniKey, EVKey, OpenKey...) để tránh xung đột. EasyKey sẽ nhắc nếu thấy bộ gõ khác đang chạy.

Nếu máy đã có OpenKey, lần chạy đầu EasyKey tự chép cài đặt và bảng gõ tắt sang. Cài đặt lưu ở `HKEY_CURRENT_USER\Software\EasyKey`.

## Cảnh báo của Windows khi chạy
EasyKey hiện **chưa có chữ ký số** (code signing). Vì vậy, khi tải về hoặc chạy:
- Windows SmartScreen có thể hiện "Windows protected your PC". Bấm **More info** › **Run anyway** để chạy.
- Windows Defender hoặc phần mềm diệt virus khác có thể chặn, cách ly hoặc xoá `EasyKey.exe`.

Đây không phải lỗi và cũng không phải vấn đề bảo mật của EasyKey. Bộ gõ nào cũng phải đọc phím bấm và gửi lại chữ đã bỏ dấu, nên phần mềm diệt virus dễ nghi ngờ các chương trình chưa có chữ ký. Mã nguồn công khai, và mỗi bản phát hành được build trên GitHub Actions kèm chứng thực nguồn gốc (build provenance). Bạn có thể tự kiểm tra file zip hoặc `EasyKey.exe` tải về bằng [GitHub CLI](https://cli.github.com):

```
gh attestation verify EasyKey<phiên bản>-Windows.zip --repo iamcino/EasyKey
```

Hãy cân nhắc trước khi dùng. Nếu tin tưởng, bạn có thể thêm thư mục EasyKey vào danh sách loại trừ của phần mềm diệt virus. Các bản sau sẽ được ký số khi có chứng chỉ.

## Tính năng
- Kiểu gõ Telex, VNI, Simple Telex.
- Bảng mã Unicode. Bảng mã cũ Unicode tổ hợp, TCVN3 (ABC), VNI Windows nằm trong mục Nâng cao.
- Đặt dấu kiểu mới (hoà, thuý) hoặc kiểu cũ (hòa, thúy).
- Gõ thông minh: chỉ bỏ dấu khi đúng chính tả, gõ sai thì tự trả lại phím. Bấm Esc ngay sau một từ để lấy lại phím vừa gõ (việt → vieejt).
- Gõ nhanh, gõ tắt phụ âm đầu và cuối, gõ tắt (macro) không giới hạn độ dài.
- Nhớ chế độ Việt/Anh và bảng mã riêng cho từng ứng dụng.
- Sửa lỗi lặp chữ ở ô có gợi ý tự động, kể cả thanh địa chỉ Chrome và Edge.
- Công cụ chuyển mã văn bản và bảng gõ tắt, mở từ menu khay.
- Giao diện Windows 11 gọn trong một trang, tiếng Việt hoặc tiếng Anh, chế độ sáng/tối theo Windows, chọn màu nhấn hoặc mã màu riêng, hỗ trợ DPI cao, biểu tượng khay theo màu thanh tác vụ.
- Tự kiểm tra và cài bản mới ngay trong EasyKey (luôn hỏi trước, kiểm tra mã SHA-256).
- Gỡ sạch khỏi Windows bằng một nút trong Cài đặt.
- Không hiện ô ngôn ngữ trên thanh tác vụ.

## Kiểm tra chính tả tiếng Việt
Chữ tiếng Việt bị gạch đỏ là do trình kiểm tra chính tả của từng ứng dụng đang dò theo tiếng Anh, không phải lỗi của EasyKey. Cách nhanh nhất là thêm tiếng Việt vào phần kiểm tra chính tả của chính ứng dụng đó:

- **Chrome, Edge, Cốc Cốc:** mở Cài đặt › Ngôn ngữ (Languages), thêm Tiếng Việt vào danh sách ngôn ngữ ưu tiên, rồi bật kiểm tra chính tả cho Tiếng Việt. Nếu trình duyệt không có từ điển tiếng Việt, hãy tắt kiểm tra chính tả cho tiếng Anh hoặc tắt hẳn tính năng này.
- **Word, Outlook, PowerPoint:** chọn đoạn văn, vào Review › Language › Set Proofing Language, chọn Vietnamese. Nếu Office chưa có bộ kiểm tra tiếng Việt, tích "Do not check spelling or grammar" cho đoạn đó.
- **Notepad:** vào Cài đặt (biểu tượng bánh răng) và tắt Spell check.

Cách thêm tiếng Việt trong Cài đặt Windows › Ngôn ngữ cũng hết gạch đỏ ở nhiều ứng dụng, nhưng sẽ làm hiện lại ô ngôn ngữ (ENG/VIE) trên thanh tác vụ, nên EasyKey không khuyên dùng.

## Tự build từ mã nguồn
Bạn có thể tự build EasyKey thay vì tải bản có sẵn.

**Cách 1: build trên máy**
1. Cài [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) (bản Community miễn phí), chọn workload **Desktop development with C++** (có sẵn MSVC v143 và Windows SDK).
2. Tải mã nguồn: `git clone https://github.com/iamcino/EasyKey.git` hoặc Code › Download ZIP trên GitHub.
3. Mở `Sources/EasyKey/win32/EasyKey/EasyKey.sln`, chọn cấu hình `Release` và nền `x64`, rồi Build › Build Solution.
4. File chạy nằm ở `Sources/EasyKey/win32/EasyKey/x64/Release/EasyKey.exe`.

Hoặc build bằng dòng lệnh trong "Developer PowerShell for VS 2022":

```
msbuild Sources\EasyKey\win32\EasyKey\EasyKey.sln -p:Configuration=Release -p:Platform=x64
```

**Cách 2: build trên GitHub, không cần cài gì**
Fork repo này. Mỗi lần push hoặc mở pull request vào nhánh `master`, GitHub Actions tự chạy test engine rồi build `EasyKey.exe`. Vào tab Actions, mở lần chạy mới nhất và tải `EasyKey` trong mục Artifacts. Khi push tag trùng `versionName` trong `version.json` (ví dụ `1.0.0`), hoặc chạy tay workflow MSBuild trên nhánh `master` (tab Actions › MSBuild › Run workflow), Actions tự tạo bản phát hành.

**Chạy test engine** (cần [CMake](https://cmake.org) và một trình biên dịch C++, chạy được trên cả Windows và Linux):

```
cmake -S Sources/EasyKey/tests -B build
cmake --build build --config Release
ctest --test-dir build -C Release
```

File tự build không có chữ ký số nên cũng có thể gặp cảnh báo như phần [Cảnh báo của Windows khi chạy](#cảnh-báo-của-windows-khi-chạy).

## Bản quyền
Copyright (C) 2026 Cino. Liên hệ: duongtheson@proton.me

EasyKey là phần mềm tự do, phát hành theo giấy phép [GNU GPL phiên bản 3](LICENSE). EasyKey được phát triển từ [OpenKey](https://github.com/tuyenvm/OpenKey), Copyright (C) 2019 Mai Vũ Tuyên, cũng theo GPLv3. Xem [NOTICE](NOTICE).

Một số file do EasyKey viết mới hoàn toàn, không chứa code OpenKey (khung giao diện, popup, hộp thoại, giao diện sáng/tối, bộ cập nhật), được cấp thêm giấy phép [MIT](LICENSES/MIT.txt) để có thể dùng lại ở dự án khác. Đầu mỗi file này có dòng `SPDX-License-Identifier: GPL-3.0-only OR MIT`; danh sách nằm trong [NOTICE](NOTICE). EasyKey nói chung vẫn theo GPLv3.
