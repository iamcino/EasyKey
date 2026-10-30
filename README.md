<div align="center">

<img src="docs/images/easykey.png" width="96" alt="EasyKey">

# EasyKey

**Bộ gõ tiếng Việt gọn nhẹ cho Windows 11: một file chưa tới 1 MB, gõ lẫn được tiếng Anh, giao diện đúng chất Windows 11.**

**Tiếng Việt** · [English](README.en.md)

[![Phiên bản](https://img.shields.io/github/v/release/iamcino/EasyKey?label=phi%C3%AAn%20b%E1%BA%A3n&color=1f6fe5)](https://github.com/iamcino/EasyKey/releases/latest)
![Nền tảng](https://img.shields.io/badge/n%E1%BB%81n%20t%E1%BA%A3ng-Windows%2011%20x64-1f6fe5)
![C++](https://img.shields.io/badge/C%2B%2B-Win32-00599c?logo=cplusplus)
[![Giấy phép: GPL v3](https://img.shields.io/badge/gi%E1%BA%A5y%20ph%C3%A9p-GPL%20v3-green)](LICENSE)

[**Tải EasyKey.exe**](https://github.com/iamcino/EasyKey/releases/latest/download/EasyKey.exe) · [Trang phát hành](https://github.com/iamcino/EasyKey/releases/latest) · [Nhật ký thay đổi](CHANGELOG.md) · [Báo lỗi](https://github.com/iamcino/EasyKey/issues)

</div>

---

## Mục lục

- [EasyKey là gì?](#easykey-là-gì)
  - [Giao diện ứng dụng](#giao-diện-ứng-dụng)
- [Vì sao chọn EasyKey](#vì-sao-chọn-easykey)
- [Tính năng](#tính-năng)
- [Yêu cầu hệ thống](#yêu-cầu-hệ-thống)
- [Cài đặt](#cài-đặt)
- [Bắt đầu sử dụng](#bắt-đầu-sử-dụng)
- [Cách hoạt động](#cách-hoạt-động)
- [Quyền riêng tư](#quyền-riêng-tư)
- [Gỡ cài đặt](#gỡ-cài-đặt)
- [Build từ mã nguồn](#build-từ-mã-nguồn)
- [Câu hỏi thường gặp và xử lý sự cố](#câu-hỏi-thường-gặp-và-xử-lý-sự-cố)
- [Miễn trừ trách nhiệm](#miễn-trừ-trách-nhiệm)
- [Giấy phép và ghi công](#giấy-phép-và-ghi-công)

## EasyKey là gì?

EasyKey là bộ gõ tiếng Việt cho Windows 11, phát triển từ engine
[OpenKey](https://github.com/tuyenvm/OpenKey) và được viết lại phần ứng dụng
để hợp với Windows hiện đại. Bạn tải một file `EasyKey.exe`, chạy, và gõ được
ngay: Telex, VNI hay Simple Telex, bảng mã Unicode, bấm **Alt + Z** để chuyển
Việt/Anh.

EasyKey không thêm bàn phím vào Windows, nên thanh tác vụ không hiện ô
ENG/VIE. Nó chạy ở khay hệ thống, không cần quyền Admin, không cần .NET hay
thư viện cài thêm, và gỡ sạch bằng một nút.

### Giao diện ứng dụng

| Cửa sổ Cài đặt (chế độ sáng) | Cửa sổ Cài đặt (chế độ tối) |
|:---:|:---:|
| <img src="docs/screenshots/settings-light.png" width="390" alt="Cửa sổ Cài đặt, chế độ sáng"> | <img src="docs/screenshots/settings-dark.png" width="390" alt="Cửa sổ Cài đặt, chế độ tối"> |

<p align="center">
  <strong>Popup khi chuyển Việt/Anh</strong><br><br>
  <img src="docs/screenshots/flyout.png" width="280" alt="Popup khi chuyển Việt/Anh">
</p>

## Vì sao chọn EasyKey

| Ưu thế | Ý nghĩa với bạn |
|--------|-----------------|
| **Một file, chưa tới 1 MB** | Không trình cài đặt, không DLL, không runtime. Chép đi đâu cũng chạy, bỏ vào USB cũng được. |
| **Gõ Anh–Việt trong cùng một câu** | Chế độ Anh–Việt tự nhận ra từ tiếng Anh và trả lại phím gốc khi bạn gõ xong từ, kể cả từ trùng với chữ Việt như "this" (thí), "test" (tét). Không phải bấm chuyển chế độ giữa câu. |
| **Gõ ổn định ở nơi bộ gõ hay lỗi** | Không lặp chữ ở thanh địa chỉ Chrome, Edge, Brave; tự gắn lại hook khi Windows gỡ mất (lỗi "chaats" ở Discord); chế độ game để phím kỹ năng đi thẳng vào game. |
| **Nhớ theo từng ứng dụng** | Mỗi ứng dụng giữ chế độ Việt/Anh và bảng mã riêng, kể cả ứng dụng Microsoft Store. Mở Word dùng TCVN3, quay lại Chrome vẫn là Unicode. |
| **Giao diện Windows 11 thật sự** | Mica, sáng/tối theo Windows, màu nhấn tuỳ chọn, chữ DirectWrite sắc nét ở mọi mức phóng to, font Be Vietnam Pro nhúng sẵn. Giao diện tiếng Việt hoặc tiếng Anh. |
| **Không ô ENG/VIE trên thanh tác vụ** | Windows vẫn chỉ có một bàn phím, nên không có ô ngôn ngữ chiếm chỗ hay nhảy lung tung. |
| **Cập nhật an toàn** | Chỉ tải qua HTTPS từ trang phát hành của dự án, đối chiếu mã SHA-256, luôn hỏi trước khi cài. Mỗi bản phát hành được build công khai trên GitHub Actions kèm chứng thực nguồn gốc. |
| **Riêng tư** | Không tài khoản, không thu thập dữ liệu, không ghi lại phím bạn gõ. Kết nối duy nhất là kiểm tra bản mới trên GitHub. |
| **Mã nguồn mở** | GPL v3. Engine gõ có bộ test chạy trên cả Windows và Linux mỗi lần đổi mã. |

## Tính năng

### Gõ tiếng Việt

- **Kiểu gõ:** Telex, VNI, Simple Telex. Đổi bằng menu khay, trong Cài đặt,
  hoặc bằng phím tắt (tuỳ chọn).
- **Gõ thông minh:** chỉ bỏ dấu khi từ đúng chính tả tiếng Việt, gõ từ tiếng
  Anh thì giữ nguyên phím. Bấm **Esc** ngay sau một từ để lấy lại phím vừa gõ
  (việt → vieejt).
- **Dấu kiểu mới** (hoà, thuý) hoặc kiểu cũ (hòa, thúy).
- **Gõ tắt** không giới hạn độ dài, có sẵn vài từ thông dụng (ko → không,
  dc → được, vs → với, mn → mọi người, ntn → như thế nào), tự viết hoa theo
  từ gõ tắt, dùng được cả khi đang gõ tiếng Anh.
- **Gõ nhanh** (tuỳ chọn): cc → ch, gg → gi, nn → ng; phụ âm đầu f → ph,
  j → gi, w → qu; phụ âm cuối g → ng, h → nh, k → ch.
- **Viết hoa chữ cái đầu câu**, kể cả sau dấu ? và ! và khi chuyển cửa sổ.
- **Bảng mã:** Unicode. Bảng mã cũ (Unicode tổ hợp, TCVN3, VNI Windows) nằm
  trong Nâng cao › Bảng mã, chỉ cần khi dùng font cũ.
- **Công cụ chuyển mã** giữa các bảng mã, đổi hoa/thường, bỏ dấu, kèm phím
  tắt chuyển mã nhanh nội dung clipboard.

### Chế độ Anh–Việt

Chế độ thứ ba bên cạnh Tiếng Việt và Tiếng Anh, dành cho người gõ lẫn hai thứ
tiếng (lập trình, công việc, học tập):

- Bạn gõ tiếng Việt như bình thường. Khi gõ xong một từ (dấu cách, dấu câu),
  EasyKey xét xem từ đó là tiếng Anh hay tiếng Việt. Nếu là tiếng Anh, chữ đã
  bỏ dấu được thay bằng đúng các phím bạn gõ.
- Nhận ra cả những từ tiếng Anh trùng với chữ Việt khi gõ Telex: "this"
  (thí), "test" (tét), "mix" (mĩ).
- Xét theo ngữ cảnh: "tôi sẽ đi" vẫn là tiếng Việt, còn "There are" không
  thành "Thể are". Một từ tiếng Anh rõ ràng còn sửa lại từ ngay trước nó.
- Bật bằng nút **Anh–Việt** (badge EV) trong Cài đặt hoặc mục **Gõ Anh–Việt**
  ở menu khay. **Alt + Z** hay bấm vào biểu tượng khay lần lượt
  chuyển qua cả ba chế độ: Việt, Anh–Việt, Anh.

### Theo từng ứng dụng

- **Nhớ Việt/Anh và bảng mã cho từng ứng dụng.** Đổi cửa sổ là EasyKey đổi
  theo, menu khay luôn đánh dấu đúng bảng mã của ứng dụng đang dùng. Ứng dụng
  Microsoft Store được nhớ theo tên riêng của nó; ứng dụng không đọc được tên
  thì giữ nguyên lựa chọn hiện tại thay vì ghi nhầm sang ứng dụng khác.
- **Chế độ game:** bật cho từng game từ menu khay khi đang chơi. Phím Q, W, E,
  R… đi thẳng vào game; bấm Enter mở khung chat thì gõ được tiếng Việt, Enter
  hoặc Esc thì trở lại.
- **Không lặp chữ** ở thanh địa chỉ Chrome, Edge, Brave và các ô có gợi ý tự
  động.
- **Ứng dụng quyền Admin:** EasyKey báo khi bạn mở một ứng dụng như vậy, và có
  tuỳ chọn chạy với quyền Admin (qua Task Scheduler, không hỏi UAC mỗi lần đăng
  nhập).
- **Gửi chữ qua Clipboard** cho những ứng dụng nhận sai khi bỏ dấu.

### Giao diện

- Cửa sổ Cài đặt gọn: màn hình chính để chọn chế độ, phím tắt, kiểu gõ; phần
  **Nâng cao** gồm các nhóm Gõ, Giao diện, Bảng mã, Chính tả, Gõ nhanh và gõ
  tắt, Ứng dụng, Hệ thống, mỗi mục có một dòng giải thích.
- Sáng, tối hoặc theo Windows; màu nhấn theo Windows, 7 màu có sẵn hoặc mã màu
  tuỳ chọn (tự tạo màu cho chế độ tối, có kiểm tra độ tương phản).
- Giao diện tiếng Việt hoặc tiếng Anh, tự chọn theo vùng của Windows.
- Popup nhỏ khi chuyển Việt/Anh, đổi kiểu gõ hay bảng mã (không hiện khi đang
  chơi game toàn màn hình). Tuỳ chọn kêu bíp khi chuyển.
- Biểu tượng khay vẽ đúng cỡ pixel ở mọi mức phóng to: phím màu nhấn chữ **V** khi
  gõ tiếng Việt, **EV** khi gõ Anh–Việt, ô trung tính chữ **E** khi gõ tiếng
  Anh, mũi tên xanh lá phát sáng khi có bản mới.

### Ứng dụng

- **Tự cập nhật** ngay trong ứng dụng, luôn hỏi trước, kiểm tra SHA-256.
- **Tự gắn lại hook** khi Windows gỡ mất, nhận ra ngay ở phím kế tiếp.
- Cảnh báo khi UniKey, EVKey, OpenKey… đang chạy cùng lúc.
- Lần đầu chạy từ thư mục Tải về, đề nghị chuyển sang thư mục cố định.
- Đã dùng OpenKey? Lần đầu chạy, EasyKey tự chép cài đặt và bảng gõ tắt sang.

## Yêu cầu hệ thống

| | |
|---|---|
| Hệ điều hành | Windows 11, bản 64-bit (x64). |
| Quyền | Không cần quyền Admin. |
| Phần mềm kèm theo | Không cần .NET, Visual C++ Redistributable hay thư viện nào khác. |
| Bộ gõ khác | Nên tắt UniKey, EVKey, OpenKey… vì hai bộ gõ chạy cùng lúc sẽ bỏ dấu hai lần. |

## Cài đặt

1. Tải [`EasyKey.exe`](https://github.com/iamcino/EasyKey/releases/latest/download/EasyKey.exe)
   (hoặc file zip ở [trang phát hành](https://github.com/iamcino/EasyKey/releases/latest)).
2. Chạy file vừa tải. Nếu Windows hiện cảnh báo, xem
   [Cảnh báo của Windows](#cảnh-báo-của-windows).
3. Lần đầu chạy từ thư mục Tải về, EasyKey đề nghị chuyển sang
   `%LOCALAPPDATA%\Programs\EasyKey` để không bị mất khi dọn thư mục. Bấm
   **Chuyển** là xong.

Mặc định: kiểu gõ Telex, bảng mã Unicode, chạy cùng Windows, **Alt + Z** để
chuyển Việt/Anh.

### Cập nhật

EasyKey kiểm tra bản mới khi khởi động và 6 giờ một lần (tắt ở Nâng cao › Hệ
thống). Khi có bản mới, biểu tượng khay có mũi tên xanh lá, menu khay hiện
**Cập nhật lên …** và cuối cửa sổ Cài đặt có dòng báo. Không có gì được cài cho
tới khi bạn đồng ý. Lúc đó EasyKey tải `EasyKey.exe` mới từ trang phát hành,
đối chiếu mã SHA-256, thay file đang chạy rồi khởi động lại. Bấm phải › **Kiểm
tra cập nhật…** để kiểm tra ngay.

## Bắt đầu sử dụng

1. Chạy EasyKey. Biểu tượng xuất hiện ở khay hệ thống (góc phải thanh tác vụ).
2. Gõ thử trong bất kỳ ứng dụng nào. Bấm **Alt + Z** để chuyển Việt/Anh.
3. Hay gõ lẫn tiếng Anh? Bấm phải vào biểu tượng › **Gõ Anh–Việt**.
4. Bấm đúp vào biểu tượng để mở Cài đặt và chọn kiểu gõ, màu, phím tắt.

| Muốn làm gì | Cách làm |
|---|---|
| Chuyển Việt → Anh–Việt → Anh | Bấm **Alt + Z**, hoặc bấm vào biểu tượng ở khay |
| Gõ lẫn tiếng Anh trong câu tiếng Việt | Bấm phải vào biểu tượng ở khay › Gõ Anh–Việt, hoặc chọn Anh–Việt trong Cài đặt |
| Mở Cài đặt | Bấm đúp vào biểu tượng ở khay, hoặc bấm phải › Cài đặt… |
| Đổi kiểu gõ, bảng mã | Bấm phải vào biểu tượng ở khay › Kiểu gõ / Bảng mã, hoặc trong Cài đặt |
| Lấy lại phím vừa gõ khi bị bỏ dấu nhầm | Bấm **Esc** ngay sau từ đó (việt → vieejt) |
| Bật chế độ game cho game đang chơi | Khi đang ở trong game, bấm phải vào biểu tượng ở khay › Chế độ game cho … |
| Đổi phím tắt chuyển Việt/Anh | Cài đặt › Chuyển nhanh › Đổi phím |
| Sửa bảng gõ tắt | Cài đặt › Nâng cao › Gõ › Bảng gõ tắt |

## Cách hoạt động

```
 Bàn phím
    │
    ▼
 Hook bàn phím cấp thấp ──► Engine (Telex / VNI / Simple Telex, chính tả, Anh–Việt)
    │                              │
    │                              ▼
    │                     số ký tự cần xoá + chữ mới
    │                              │
    ▼                              ▼
 Ứng dụng đang gõ ◄──── Backspace + ký tự Unicode (hoặc qua clipboard)
```

EasyKey để phím đi vào ứng dụng như bình thường. Khi cần thêm dấu, nó xoá chữ
cũ và gửi chữ mới. Vì không đăng ký bàn phím với Windows nên không có ô
ENG/VIE. Khi bạn đổi cửa sổ, EasyKey đọc lại chế độ và bảng mã đã nhớ cho ứng
dụng đó. Chi tiết trong [tài liệu thiết kế](docs/DESIGN.md).

| Thư mục | Vai trò |
|---------|---------|
| `Sources/EasyKey/engine` | Engine gõ tiếng Việt, C++ thuần, không gọi Windows API |
| `Sources/EasyKey/tests` | Test engine (CMake, chạy trên Windows và Linux) |
| `Sources/EasyKey/win32` | Ứng dụng Windows: hook, giao diện, cài đặt, cập nhật |
| `tools` | Script tạo biểu tượng, font và danh sách từ Anh–Việt |

## Quyền riêng tư

EasyKey không có tài khoản, không thu thập dữ liệu sử dụng, không quảng cáo và
không ghi lại phím bạn gõ. Cài đặt nằm trong `HKCU\Software\EasyKey` trên máy
bạn. Kết nối duy nhất EasyKey tự tạo là tới GitHub để kiểm tra và tải bản mới
(tắt được). Xem [PRIVACY.md](PRIVACY.md).

## Gỡ cài đặt

Vào Cài đặt › Nâng cao › Hệ thống › **Gỡ EasyKey khỏi Windows**. EasyKey xoá
mục chạy cùng Windows, tác vụ Task Scheduler (nếu có), lối tắt trên màn hình
nền, cài đặt và bảng gõ tắt rồi thoát. Sau đó xoá file `EasyKey.exe`. EasyKey
không ghi gì vào thư mục hệ thống.

## Build từ mã nguồn

Cần Visual Studio 2022 với workload **Desktop development with C++**:

```powershell
msbuild Sources\EasyKey\win32\EasyKey\EasyKey.sln -p:Configuration=Release -p:Platform=x64
# Sources\EasyKey\win32\EasyKey\x64\Release\EasyKey.exe
```

Test engine chạy được trên cả Windows và Linux:

```bash
cmake -S Sources/EasyKey/tests -B build
cmake --build build --config Release
ctest --test-dir build -C Release
```

Hướng dẫn đầy đủ, kể cả build trên GitHub không cần cài gì, ở
[docs/BUILD.md](docs/BUILD.md). Mọi đóng góp đều được hoan nghênh; vui lòng
đọc [CONTRIBUTING.md](CONTRIBUTING.md).

## Câu hỏi thường gặp và xử lý sự cố

**EasyKey khác OpenKey thế nào?**
EasyKey giữ engine gõ của OpenKey và viết lại gần như toàn bộ phần ứng dụng
Windows: giao diện mới, chế độ Anh–Việt, chế độ game, tự cập nhật, gắn lại
hook, sửa lỗi ở trình duyệt và nhiều lỗi khác. Xem [nhật ký thay
đổi](CHANGELOG.md).

**Có cần gỡ UniKey hay EVKey không?**
Không cần gỡ, chỉ cần tắt. Hai bộ gõ chạy cùng lúc sẽ bỏ dấu hai lần. EasyKey
nhắc khi thấy bộ gõ khác đang chạy.

**EasyKey có gửi phím tôi gõ đi đâu không?**
Không. Phím chỉ được xử lý trong máy. EasyKey không ghi phím ra file, trừ khi
bạn tự bật nhật ký phím để báo lỗi (xem [Gõ ra chữ sai](#gõ-ra-chữ-sai)).

**Có bản cho macOS, Linux hay ARM64 không?**
Chưa. EasyKey hiện chỉ tập trung cho Windows 11 x64 cho tới khi thật ổn định.

### Cảnh báo của Windows

EasyKey **chưa có chữ ký số**. Vì vậy SmartScreen có thể hiện "Windows
protected your PC" (bấm **More info** › **Run anyway**), và phần mềm diệt virus
có thể chặn `EasyKey.exe`. Bộ gõ nào cũng phải đọc phím bấm để bỏ dấu, nên dễ
bị nghi ngờ khi chưa ký số.

Mã nguồn công khai. Mỗi bản phát hành được build trên GitHub Actions kèm chứng
thực nguồn gốc (build provenance). Kiểm tra bằng
[GitHub CLI](https://cli.github.com):

```
gh attestation verify EasyKey<phiên bản>-Windows.zip --repo iamcino/EasyKey
```

Nếu tin tưởng, bạn có thể thêm thư mục EasyKey vào danh sách loại trừ của phần
mềm diệt virus.

### Kiểm tra chính tả tiếng Việt

Chữ tiếng Việt bị gạch đỏ là do trình kiểm tra chính tả của ứng dụng đang dò
theo tiếng Anh, không phải lỗi của EasyKey:

- **Chrome, Edge, Cốc Cốc:** Cài đặt › Ngôn ngữ (Languages), thêm Tiếng Việt
  và bật kiểm tra chính tả cho Tiếng Việt, hoặc tắt kiểm tra chính tả tiếng
  Anh.
- **Word, Outlook, PowerPoint:** chọn đoạn văn, Review › Language › Set
  Proofing Language › Vietnamese (hoặc tích "Do not check spelling or
  grammar").
- **Notepad:** Cài đặt (bánh răng) › tắt Spell check.

Thêm tiếng Việt trong Cài đặt Windows › Ngôn ngữ cũng hết gạch đỏ, nhưng làm
hiện lại ô ENG/VIE trên thanh tác vụ, nên không khuyên dùng.

### Không gõ được tiếng Việt trong một ứng dụng

- **Ứng dụng chạy quyền Admin** (Task Manager, trình cài đặt…): Windows không
  cho ứng dụng thường gửi phím vào đó. Bật Nâng cao › Hệ thống › **Chạy với
  quyền Admin**.
- **Ứng dụng nhận sai chữ khi bỏ dấu** (mất chữ, thừa chữ): thử bật Nâng cao ›
  Ứng dụng › **Gửi chữ qua Clipboard**. Cách này thay nội dung clipboard mỗi lần
  bỏ dấu, nên chỉ bật khi cần.
- **Game:** bật chế độ game cho game đó (xem
  [Bắt đầu sử dụng](#bắt-đầu-sử-dụng)).
- **Từ tiếng Việt bị trả về phím tiếng Anh** (hoặc ngược lại) trong chế độ
  Anh–Việt: bấm **Esc** ngay sau từ đó, hoặc chuyển về chế độ Tiếng Việt cho
  đoạn văn thuần Việt. Hãy [báo lỗi](https://github.com/iamcino/EasyKey/issues)
  kèm câu đã gõ để danh sách từ được chỉnh.

### Gõ ra chữ sai

Nếu gặp chữ sai lặp lại (phím thô như "chaats", chữ hoa lẫn lộn…), bạn có thể
ghi nhật ký phím để gửi kèm báo lỗi:

1. Mở PowerShell và chạy
   `reg add HKCU\Software\EasyKey /v vDebugKeyLog /t REG_DWORD /d 1 /f`.
2. Thoát EasyKey (bấm phải vào biểu tượng ở khay › Thoát EasyKey) rồi mở lại.
3. Gõ lại cho tới khi lỗi xảy ra. Nhật ký nằm ở `%TEMP%\EasyKeyKeyLog.txt`.
4. Tắt lại bằng `reg delete HKCU\Software\EasyKey /v vDebugKeyLog /f` và mở
   lại EasyKey.

Nhật ký ghi mã phím bạn bấm, nên chỉ bật khi cần, đừng gõ mật khẩu trong lúc
bật, và xem lại trước khi gửi.

## Miễn trừ trách nhiệm

EasyKey là phần mềm miễn phí, được cung cấp **"nguyên trạng", không kèm bất kỳ
bảo đảm nào**, theo giấy phép GPL v3. Tóm tắt:

- EasyKey đọc phím bấm qua hook bàn phím của Windows để bỏ dấu; phím chỉ được
  xử lý trên máy bạn.
- Bạn tự chịu trách nhiệm dùng EasyKey đúng pháp luật và quy định nơi bạn làm
  việc, học tập hay phần mềm bạn dùng (ví dụ quy định của game).
- Tác giả không chịu trách nhiệm về mất dữ liệu, chữ gõ sai hay thiệt hại khác
  khi dùng EasyKey.
- `EasyKey.exe` chưa có chữ ký số. Chỉ tải từ
  [trang phát hành chính thức](https://github.com/iamcino/EasyKey/releases).
- EasyKey là dự án độc lập, không liên kết với Microsoft hay các bộ gõ khác,
  và không do tác giả OpenKey phát hành.

Toàn văn ở [DISCLAIMER.md](DISCLAIMER.md). Nếu không đồng ý, vui lòng không
sử dụng EasyKey.

## Giấy phép và ghi công

Copyright (C) 2026 Cino. Liên hệ: duongtheson@proton.me

EasyKey phát hành theo [GNU GPL phiên bản 3](LICENSE), phát triển từ
[OpenKey](https://github.com/tuyenvm/OpenKey) của Mai Vũ Tuyên (Copyright (C)
2019, GPLv3). Một số file EasyKey viết mới hoàn toàn được cấp thêm giấy phép
[MIT](LICENSES/MIT.txt). Thành phần bên thứ ba:

- Font giao diện **Be Vietnam Pro**: [SIL OFL 1.1](LICENSES/OFL-1.1.txt).
- Danh sách từ của chế độ Anh–Việt lấy từ dữ liệu
  [wordfreq](https://github.com/rspeer/wordfreq): CC BY-SA 4.0.

Chi tiết trong [NOTICE](NOTICE). Báo lỗi bảo mật theo [SECURITY.md](SECURITY.md). Các thay đổi được ghi trong
[nhật ký thay đổi](CHANGELOG.md).
