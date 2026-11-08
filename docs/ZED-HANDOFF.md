# Bàn giao EasyKey cho Zed

Ngày 2026-11-06. Người dùng gọi trợ lý là **Zed**. Khi tiếp tục trên máy khác, đọc file này và [AGENTS.md](../AGENTS.md) trước khi sửa.

## Bắt đầu trên máy khác

1. Chạy `git branch --show-current`, `git status -s` và `git log -1 --oneline`. Tại lúc bàn giao, nhánh local là `master`, HEAD là `dc776af` (merge release 1.0.21).
2. Mang theo **working tree**, bao gồm các file mới chưa được Git theo dõi, hoặc một snapshot local chứa chúng. Clone ở commit trên chỉ lấy được nền cũ. Bản bàn giao này mô tả thay đổi, còn mã thực tế nằm trong working tree.
3. Đối chiếu diff và chạy lại kiểm thử trước khi sửa tiếp. Giữ các thay đổi có sẵn. Tuân thủ quy định local-only trong AGENTS.md, bao gồm phạm vi thao tác Git và yêu cầu khai báo trực tiếp C++ standard headers.

## Việc đã làm

| Hạng mục | Thay đổi và vị trí |
|---|---|
| Macro | [Macro.cpp](../Sources/EasyKey/engine/Macro.cpp) giới hạn tên 255 byte UTF-8, nội dung 65.535 byte và bảng 65.535 mục. Kiểm tra biên khi nạp dữ liệu nhị phân, chỉ nhận bảng đọc được đầy đủ. Import bỏ qua dòng vượt giới hạn. [MacroWindow.cpp](../Sources/EasyKey/win32/EasyKey/EasyKey/MacroWindow.cpp) báo lỗi và giữ nội dung nhập khi thêm macro thất bại. |
| Mixed | [Engine.cpp](../Sources/EasyKey/engine/Engine.cpp) giữ quyết định từ quy tắc cụm từ, danh sách ưu tiên và bảo vệ từ ngắn khi bật Fix Last. Các ca hồi quy gồm `banhs test case ` và `mootj too software `. |
| Updater | Quyết định giữ SHA-256 sau khi đánh giá BLAKE3, để duy trì giao thức `.sha256` cho client cũ và tương thích Authenticode/VirusTotal. [Updater.cpp](../Sources/EasyKey/win32/EasyKey/EasyKey/Updater.cpp) kiểm checksum rồi xác minh chữ ký trước khi thay EXE. Staging dùng tên ngẫu nhiên, tạo mới độc quyền và đổi tên bằng chính handle đã xác minh. Nếu đổi tên thất bại, updater thử trả lại bản cũ. |
| Chữ ký | [UpdateSignature.cpp](../Sources/EasyKey/win32/EasyKey/EasyKey/UpdateSignature.cpp) kiểm Authenticode, pin toàn bộ DER nhúng từ chứng thư công khai EasyKey, digest SHA-256 và timestamp RFC 3161. Chứng thư self-signed của publisher dùng chain engine riêng trong bộ nhớ. TSA dùng Windows trust. Chain retrieval dùng cache, không kiểm revocation online. [Workflow release](../.github/workflows/msbuild.yml) dừng nếu thiếu thông tin ký. |
| Dọn backup | [UpdateCleanup.cpp](../Sources/EasyKey/win32/EasyKey/EasyKey/UpdateCleanup.cpp) chạy trên worker sau khi engine và tray khởi tạo. Thử xóa `EasyKey.old.exe` tối đa 120 lần, cách nhau 500 ms, xử lý thuộc tính read-only. Nếu vẫn bị khóa, giữ file và thử lại ở lần khởi động thành công tiếp theo. |
| Build Release | `HostX86/x64` tái hiện `LNK1101: incorrect MSPDB140.DLL version` với một object tối giản, còn `HostX64/x64` liên kết được object đó. Project mặc định `PreferredToolArchitecture=x64`. Clean rebuild đã tạo EXE và PDB, giữ `/DEBUG` và LTCG. Đây là cách khắc phục lựa chọn host tools trên máy đã thử. Nguyên nhân nội bộ của lỗi PDB host x86 chưa được xác định. |

## Build và kiểm thử lại

Từ thư mục gốc repo, dùng Developer PowerShell hoặc Developer Command Prompt của Visual Studio 2022 có MSVC v143, Windows SDK và CMake.

```powershell
msbuild Sources\EasyKey\win32\EasyKey\EasyKey.sln -t:Rebuild -p:Configuration=Release -p:Platform=x64
cmake -S Sources/EasyKey/tests -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

- Output Release nằm ở `Sources/EasyKey/win32/EasyKey/x64/Release/`. Khi chẩn đoán LNK1101, thêm `-p:PreferredToolArchitecture=x64` và xem đường dẫn linker thực tế trong log MSBuild. Xem [hướng dẫn build](BUILD.md).
- Trên Windows MSVC, **351 ca engine, 17 ca chữ ký và 9 ca cleanup** đã có lượt chạy qua. Một ca đổi tên rồi xác minh lại file đã ký từng thất bại, sau đó qua khi chạy riêng. Chưa phân biệt được lỗi ở rename hay verify. Chưa chạy Linux local.
- Test chữ ký dùng `signtool.exe` có chữ ký nhúng từ SDK local. Fixture đó kiểm được luồng WinTrust và các ca từ chối, còn bản EasyKey ký thật với pin self-signed vẫn cần nghiệm thu riêng.

## Việc tiếp theo

1. Nghiệm thu cập nhật đầu-cuối bằng EasyKey ký thật trên Windows sạch, không cài root EasyKey. Kiểm tra timestamp bị sửa, signer hết hạn nhưng còn hợp lệ tại thời điểm timestamp, và chính sách thu hồi/chuyển chứng thư.
2. Xử lý `CreateProcess` thất bại và thử lỗi tại từng bước đổi tên. Giữ ít nhất một bản có thể phục hồi. Thêm readiness handshake trước khi dọn backup. Khởi tạo engine/tray hiện chỉ là mốc bắt đầu dọn.
3. Ghi mã lỗi riêng cho thao tác rename và verify để phân tích test không ổn định. Các hướng nghiên cứu sau đó gồm manifest ký chống rollback, parser HTTP/version/checksum, biên đọc SmartSwitch và báo kết quả import macro.

## Tài liệu mang theo

- [Thiết kế updater](DESIGN.md) và [bản tiếng Anh](DESIGN.en.md).
- [Ghi chú nghiên cứu cho Qwen](QWEN-RESEARCH-NOTES.md) có thứ tự ưu tiên và tiêu chí nghiệm thu.
- [Bản nguồn skill windows-app-updater](../skills/windows-app-updater/SKILL.md) cùng thư mục tham chiếu. Bản cài dùng chung và file Markdown độc lập đã được lưu ngoài repo trên máy cũ. Khi chuyển máy, lấy bản nguồn trong repo để đọc hoặc cài lại.

Tại thời điểm bàn giao, các thay đổi của Zed vẫn ở local, chưa commit, push hay phát hành.
