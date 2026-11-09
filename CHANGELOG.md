# Nhật ký thay đổi

**Tiếng Việt** · [English](CHANGELOG.en.md)

## 1.1.0 - 2026-11-09

### Added
- **Cập nhật**: nếu không vào được GitHub, EasyKey tự kiểm tra và tải bản mới từ GitLab. Bản tải về vẫn phải khớp mã SHA-256 và chữ ký số EasyKey như trước.
- Từ bản này, mỗi bản phát hành có trên cả [GitHub](https://github.com/iamcino/EasyKey/releases) và [GitLab](https://gitlab.com/cino-group/easykey/-/releases).
- **Giới thiệu**: thêm trang chủ GitLab và đường dẫn tới các phiên bản trên GitLab.

### Notes
- Bản 1.0.23 trở về trước chỉ tìm cập nhật trên GitHub. Nếu GitHub không vào được, hãy tải 1.1.0 bằng tay từ GitLab; từ đó trở đi EasyKey tự cập nhật từ nơi còn dùng được.

## 1.0.23 - 2026-11-08

### Fixed
- Alt+Tab không còn làm sai chế độ gõ (Việt/Anh) và bảng mã đã nhớ theo từng ứng dụng. Chế độ Anh–Việt giờ cũng được nhớ theo ứng dụng.
- **Game Mode**: không còn mất tác dụng sau khi Alt+Tab ra ngoài rồi quay lại, kể cả khi game tạo lại cửa sổ. Cửa sổ launcher/sảnh chờ có ô nhập liệu không bị nhận nhầm là game.

### Added
- **Game Mode**: chat trong game gõ được tiếng Việt. Bấm Enter để mở chat, phím đi qua bộ gõ theo chế độ đang bật (Việt, Anh–Việt hoặc Anh). Lệnh bắt đầu bằng `/` vẫn giữ phím thô tới dấu cách, Esc đóng chat. Ngoài lúc chat, phím vẫn đi thẳng vào game như trước.
- **Cập nhật**: EasyKey chỉ cài bản cập nhật có chữ ký số của EasyKey; file cũ `EasyKey.old.exe` được dọn sau khi khởi động.
- **Gõ tắt**: từ chối tên hoặc nội dung quá dài ngay khi thêm, thay vì làm mất mục khi lưu.

### Notes
- Sau khi dùng 1.0.23, nếu quay về 1.0.22 trở xuống thì bảng mã nhớ theo ứng dụng có thể bị sai; hãy chọn lại bảng mã cho ứng dụng đó.

## 1.0.22 - 2026-11-05

### Fixed
- Sửa lỗi đọc vượt bộ nhớ khi dữ liệu gõ tắt hoặc danh sách nhớ theo ứng dụng bị hỏng.
- Sửa lỗi EasyKey có thể bị tắt đột ngột khi bật gõ nhanh phụ âm cuối và gõ một từ rất dài.
- Sửa lỗi mất mục khi lưu gõ tắt hoặc danh sách nhớ theo ứng dụng có tên quá dài.

## 1.0.21 - 2026-11-04

### Fixed
- **Game Mode**: giữ liên kết cửa sổ game đã xác nhận suốt vòng đời window, tránh mất chế độ game khi người chơi chỉ di chuột lâu hoặc Alt+Tab ra ngoài rồi quay lại. Phím điều khiển như `W`, `Q`, `E`, `R`, `Alt+W` đi thẳng vào game.
- **Game Mode**: chặn nhầm hotkey khi nhả phím `Alt` / `SysKey`, reset trạng thái modifier khi vào/ra game; `Alt+Enter` và `Ctrl+Enter` không bật/tắt nhầm chat game.
- **Bộ gõ**: sửa trường hợp `q` + `w` tạo thành `qư`, giữ nguyên các tên riêng/công nghệ như `qwen`, đồng thời bảo toàn các từ tiếng Việt hợp lệ như `quơ`, `quở`, `quờ`, `quốc`, `quân`, `quên`, `quê`.

### Added
- **Mixed mode**: bảo vệ các từ tiếng Việt ngắn/khẩu ngữ trong ngữ cảnh trung tính, ví dụ `bus` → `bú`, `us` → `ú`, `too` → `tô`, `er` → `ẻ`.
- **Mixed mode**: nhận diện cụm từ hai từ để giữ đúng ngữ cảnh tiếng Anh hoặc tiếng Việt, ví dụ `xe bus`, `tuyến bus`, `with us`, `contact us`, `me too`, `mập ú`, `béo ú`, `một tô`, `đi ẻ`.
- **Mixed mode**: tôn trọng từ viết tắt ALL-CAPS như `US`, `AI`, `ER`, `UK`, `AS`, `IS`, `IF`, `OR`, trong khi dạng thường vẫn ưu tiên tiếng Việt phù hợp, ví dụ `us` → `ú`, `ai` → `ai`, `er` → `ẻ`.
- **Mixed mode**: bổ sung danh sách tên riêng AI/dev phổ biến như `qwen`, `deepseek`, `claude`, `gemini`, `chatgpt`, `openai`, `llama`, `mistral`, `ollama`, `vllm`, `cursor`, `perplexity`, `anthropic`, `midjourney`.

## 1.0.20 - 2026-11-03

### Added
- **Game Mode window binding**: Game Mode liên kết trực tiếp theo cửa sổ (`HWND` + PID) của game đang kích hoạt; giữ nguyên phím thô trong game và tự động khôi phục chế độ nhớ theo ứng dụng (SmartSwitchKey) khi Alt+Tab sang ứng dụng khác; fast-path rebind trong 5 giây mà không làm chậm hook bàn phím.
- **Game window picker**: Tự động gợi ý Top 3 cửa sổ game đang chạy trên menu khay hệ thống và Cài đặt, tính điểm ưu tiên cửa sổ foreground gần nhất và toàn màn hình đa màn hình; hỗ trợ duyệt file `.exe` thực thi của trận đấu; đảm bảo ranh giới phân quyền an toàn và bảo mật thông tin (tiêu đề cửa sổ chỉ lưu tạm trong RAM khi mở menu, không ghi đĩa, không gửi mạng).
- **VirusTotal Reference Sidecar**: Tích hợp quy trình kiểm tra mã độc độc lập qua VirusTotal API v3 downstream sau khi phát hành bản dựng, xác thực tính toàn vẹn bằng `SHA256SUMS.txt`, đính kèm báo cáo JSON chi tiết `virustotal-<tag>.json` vào release.

### Notes
- Game Mode nhận diện theo tên file thực thi cơ sở (`exeBaseName`); nếu các game khác nhau dùng chung tên file exe, cấu hình sẽ áp dụng chung.
- Một số tựa game UWP / Microsoft Store hoặc game chạy qua host đặc thù (`ApplicationFrameHost.exe`) có thể cần duyệt file `.exe` thủ công nếu Windows cho phép truy cập.
- EasyKey không đọc nội dung game hay ô nhập liệu, không can thiệp bộ nhớ tiến trình và không gửi dữ liệu ra ngoài thiết bị.

## 1.0.19 - 2026-11-02

### Added
- **Browser password field bypass**: EasyKey có thể nhận diện các ô nhập mật khẩu chuẩn trong Chrome, Edge, Firefox và Electron, sau đó chuyển sang gõ phím thô để tránh dấu tiếng Việt, gõ tắt, Quick Telex và tự động viết hoa.
- **Non-blocking detection**: nhận diện ô mật khẩu hiện đại chạy bất đồng bộ trên luồng nền; luồng hook chỉ đọc trạng thái đã cache, không làm chậm phím.
- **Sticky password**: khi một ô đã được xác nhận là mật khẩu, EasyKey giữ phím thô cho tới khi focus rời khỏi ô đó, kể cả khi người dùng bấm “Hiện mật khẩu”.
- **Kill switch**: có thể tắt phát hiện trình duyệt qua registry `vPasswordFieldUIA = 0`; ô mật khẩu Win32 chuẩn vẫn hoạt động.

### Notes
- Hỗ trợ ở mức best-effort cho các password control chuẩn. Canvas input, virtual keyboard, shadow DOM phức tạp, iframe khác nguồn hoặc control tùy chỉnh có thể không được nhận diện.
- Việc truy vấn accessibility API có thể khiến trình duyệt kích hoạt accessibility tree, làm tăng tài nguyên cục bộ.
- EasyKey không đọc nội dung ô nhập, không ghi nhật ký phím và không gửi dữ liệu ra ngoài.

## 1.0.18 - 2026-11-02

### Added
- **Game Mode**: bảo vệ phím điều khiển trong game; chat game mặc định dùng phím thô để tránh biến `afk`, `gg`, `wp`, `how`, `as` thành dấu tiếng Việt.
- **Game Mode**: Esc và click chuột trái/phải/giữa thoát trạng thái chat ngay, trả phím WASD/QWE về game; di chuột, lăn chuột và nút chuột phụ không làm thoát chat.
- **Game Mode**: lệnh bắt đầu bằng `/` như `/ff`, `/w`, `/give`, `/tp` được giữ nguyên phím thô tới phím cách hoặc Enter.
- **Password field bypass**: khi focus vào ô mật khẩu được xác định rõ, EasyKey chuyển sang gõ raw để tránh dấu tiếng Việt, gõ tắt, Quick Telex và tự động viết hoa.
- **Password field bypass**: phát hiện chỉ chạy lúc đổi focus; hook bàn phím/chuột chỉ đọc cờ cache với chi phí không đổi; không đọc nội dung ô nhập, không ghi log phím, không gửi dữ liệu ra ngoài.
- **Code signing**: bổ sung quy trình ký Authenticode, đóng dấu thời gian, kiểm chứng chữ ký và phát hành chứng chỉ công khai để người dùng kiểm tra tính toàn vẹn binary.

### Notes
- Password field bypass mặc định bật và có thể tắt trong Cài đặt › Nâng cao › Gõ.
- Windows logon, UAC secure prompt, secure desktop và một số điều khiển tùy chỉnh có thể nằm ngoài phạm vi hỗ trợ của password field bypass.
- Game Mode được kiểm chứng logic trạng thái tự động; hành vi thực tế trên từng game thuộc phạm vi kiểm thử thủ công.

## 1.0.17 (Windows 11 x64)
- Cửa sổ Cài đặt gọn hơn: thu gọn khoảng cách và cỡ chữ, ba nút chế độ **Tiếng Việt** · **Anh–Việt** · **Tiếng Anh** nằm trên một dòng và tự căn giữa cụm chữ (cả khi dùng 2 chế độ), thanh kiểu gõ chuyển sang dạng tab phẳng có vạch màu nhấn bên dưới, phần **Nâng cao** mở gọn hơn.
- Chữ **EV** ở nút chế độ, popup khi chuyển chế độ và biểu tượng khay được cân lại kích thước cho vừa khung, không chạm viền ở mọi mức phóng to; ở biểu tượng khay cỡ nhỏ (16px) chữ E vẫn tách rõ ba vạch ngang.
- Nhãn ngắn gọn hơn: **Phím chuyển** › **Đổi** (thay cho "Chuyển nhanh › Đổi phím"), menu chuột phải ở khay dùng **Tiếng Việt** và **Anh–Việt**, thêm vạch ngăn trước mục **Bảng gõ tắt…**.
- Chế độ Anh–Việt: gõ `dư dả`, `già dả` không còn bị đổi nhầm thành "dư dar", "già dar".

## 1.0.16 (Windows 11 x64)
- `EasyKey.exe` nhẹ hơn khoảng 32%, từ 939 KB xuống 636 KB: phần chuyển mã chữ và đọc/ghi file dùng API có sẵn của Windows thay cho các thành phần C++ nặng.
- Sửa lỗi ứng dụng có thể bị tắt ngang khi file gõ tắt có chuỗi UTF-8 hỏng. Giờ ký tự hỏng hiện thành "�" và gõ tắt vẫn được nạp.
- Viết lại phần đọc/ghi file gõ tắt, không giới hạn độ dài mỗi dòng. Dữ liệu gõ tắt cũ vẫn dùng được như trước, không cần chuyển đổi.
- Cách gõ Telex, VNI, Simple Telex và chế độ Anh–Việt không đổi.

## 1.0.15 (Windows 11 x64)
- Thêm tuỳ chọn **Chỉ dùng 2 chế độ Anh và Việt** trong Cài đặt › Nâng cao › Gõ: dành cho người chỉ muốn chuyển đổi qua lại giữa Tiếng Việt và Tiếng Anh thuần tuý. Khi bật, chế độ Anh–Việt được ẩn hoàn toàn khỏi giao diện, phím tắt (Alt + Z), biểu tượng khay và popup chỉ xoay vòng giữa 2 chế độ.
- Sửa lỗi gõ sai rồi dùng phím Backspace / phím cách (Space) làm nuốt mất từ trước đó hoặc làm mất chữ trong chuỗi phím.
- Chế độ Anh–Việt: hỗ trợ từ "fix" và các biến thể khi bật tuỳ chọn "Cho phép gõ z, f, w, j" (không còn bị đổi thành "fĩ").
- Gõ nhanh (Quick Telex):
  - Các phụ âm đầu `pp` (ph), `tt` (th), `gg` (gi), `kk` (kh), `qq` (qu) chỉ chuyển đổi khi đứng ở đầu từ; gõ các từ tiếng Anh có phụ âm lặp như `app`, `apple`, `happy`, `attack`, `egg` không còn bị đổi thành `ph`, `th`, `gi`.
  - Hỗ trợ gõ lặp 3 lần để hoàn tác (ví dụ `ppp` ra `pp`, `ccc` ra `cc`, `ttt` ra `tt`).

## 1.0.14 (Windows 11 x64)
- Chế độ Anh–Việt: gõ `test` ngay sau "bánh" hoặc "thịt" giờ ra "tét" (bánh tét, thịt tét) thay vì giữ chữ tiếng Anh. Các câu khác như "chạy test", "test đầu tiên" vẫn giữ "test". Muốn gõ "tét" ở chỗ khác thì gõ `tets`.

## 1.0.13 (Windows 11 x64)
- Chế độ Anh–Việt giữ nguyên tên biến và email khi gõ code: snake_case (`max_size`, `sort_desc`, trước đây thành "mã_size", "sort_déc"), camelCase (`parseJSON`, `iPhone`), chữ lẫn số và địa chỉ email. Trong chế độ này dấu `_` và `@` không còn ngắt từ. Gõ hoa hai chữ đầu như "DDi", "VIeejt" vẫn ra "Đi", "VIệt".
- Chế độ Anh–Việt không còn tự sửa lại từ đứng trước (ví dụ "Thể are" thành "There are"): việc xoá rồi gõ lại từ cũ hay chạy sai ở các ứng dụng chậm hoặc trình soạn thảo phức tạp, nên giờ tắt mặc định.
- Thêm 39 từ công nghệ và từ chat hay gặp vào danh sách tiếng Anh của chế độ Anh–Việt, như install, update, software, warning, merge, commit, token, btw.

## 1.0.12 (Windows 11 x64)
- Phím chuyển chế độ (mặc định Alt + Z) và bấm vào biểu tượng khay giờ lần lượt qua cả ba chế độ: Tiếng Việt → Anh–Việt → Tiếng Anh → Tiếng Việt.
- Biểu tượng khay hiện **EV** khi gõ Anh–Việt, thay vì chữ V như khi gõ tiếng Việt.
- Phím trên biểu tượng khay lại theo màu nhấn: màu nhấn của Windows, hoặc màu bạn chọn trong Cài đặt. Vẫn giữ hình phím bo góc như biểu tượng ứng dụng.
- Mũi tên báo có bản mới đổi thành mũi tên xanh lá có quầng sáng, đặt ở góc trên bên phải biểu tượng khay; mục cập nhật trong menu khay dùng cùng mũi tên.

## 1.0.11 (Windows 11 x64)
- Chế độ mới **Anh–Việt**, bên cạnh Tiếng Việt và Tiếng Anh: gõ tiếng Việt như thường, từ tiếng Anh tự trả lại phím khi gõ xong từ, kể cả từ trùng với chữ Việt như "this" (thí), "test" (tét). EasyKey xét độ phổ biến của từ trong mỗi thứ tiếng và các từ xung quanh, nên "tôi sẽ đi" vẫn là tiếng Việt còn "There are" không thành "Thể are". Bật bằng nút Anh–Việt trong Cài đặt hoặc "Gõ Anh–Việt" ở menu khay; Alt + Z chuyển giữa tiếng Anh và chế độ Việt dùng lần cuối.
- Sửa lỗi bảng mã theo từng ứng dụng: chuyển sang ứng dụng dùng VNI Windows hay TCVN3 thì menu khay vẫn đánh dấu Unicode, trông như bị trả về Unicode. Ứng dụng không cho đọc tên làm bảng mã chọn ở đó bị ghi nhầm sang ứng dụng trước; ứng dụng Microsoft Store giờ được nhớ theo tên riêng thay vì chung một tên ApplicationFrameHost.exe.
- Biểu tượng khay vẽ theo phím trong biểu tượng EasyKey: phím xanh chữ V khi gõ tiếng Việt, cùng hình phím màu trung tính chữ E khi gõ tiếng Anh. Màu không còn đổi theo màu nhấn của Windows.
- Bộ tài liệu viết lại: README giới thiệu ưu thế và đủ tính năng, thêm chính sách quyền riêng tư (PRIVACY.md), bảo mật (SECURITY.md), hướng dẫn đóng góp (CONTRIBUTING.md) và miễn trừ trách nhiệm (DISCLAIMER.md).

## 1.0.10 (Windows 11 x64)
- Sửa lỗi chữ EasyKey gõ lại bị viết hoa lẫn lộn, ví dụ "giúp tôi hoàn thiện" thành "giÚP tÔi hOÀN thIỆN" (gặp ở Gemini trên Firefox). Nguyên nhân là EasyKey lỡ mất lúc nhả phím Shift, ví dụ khi đang khoá màn hình hay mở cửa sổ quyền Admin, rồi cứ tưởng Shift vẫn đang giữ. Giờ EasyKey đọc lại trạng thái thật của Shift, Ctrl, Alt, Win và Caps Lock trước mỗi phím.
- README viết lại cho rõ ràng hơn (bảng thao tác, cập nhật và gỡ bỏ, xử lý sự cố, cách ghi nhật ký phím để báo lỗi). Thêm tài liệu thiết kế cho người phát triển (docs/DESIGN.md). Nút "Windows gạch đỏ chữ Việt?" trong Cài đặt lại mở đúng mục hướng dẫn.

## 1.0.9 (Windows 11 x64)
- Biểu tượng khay mới: ô vuông màu đặc với chữ V hoặc E, vẽ đúng cỡ pixel mà thanh tác vụ cần ở mọi mức phóng to (100%, 125%, 150%, 200%…) nên nét sắc, không bị mờ do thu nhỏ. Gõ tiếng Việt là ô màu nhấn chữ trắng, gõ tiếng Anh là ô trắng (thanh tác vụ tối) hoặc đen (thanh tác vụ sáng).
- Font giao diện đổi sang Be Vietnam Pro (giấy phép OFL), nhúng sẵn trong EasyKey.exe, không cài vào Windows. Windows 10 bản cũ hơn 1703 vẫn dùng Segoe UI.

## 1.0.8 (Windows 11 x64)
- Lúc cập nhật, chuột và bàn phím không còn bị khựng: EasyKey ghi file mới ở luồng riêng và gỡ hook trước khi khởi động bản mới (lúc này phần mềm diệt virus quét file mới).
- Chữ trong mọi cửa sổ (Cài đặt, Giới thiệu, Bảng gõ tắt, Chuyển mã, hộp thoại) vẽ bằng DirectWrite như popup: nét mịn, không vỡ khi Windows phóng to màn hình trên 100%.

## 1.0.7 (Windows 11 x64)
- Sửa lỗi thỉnh thoảng gõ ra phím thô (ví dụ "chaats" thay vì "chất"), hay gặp ở Discord và các app Electron: khi Windows lặng lẽ gỡ hook bàn phím của EasyKey, EasyKey giờ nhận ra ngay ở phím kế tiếp và gắn lại, kể cả khi đang dùng chuột (trước đây phải chờ vài giây và có khi không nhận ra).
- Bấm Alt + Z (hay phím tắt có Alt, Win) không còn làm ứng dụng mở thanh menu, nên chữ gõ ngay sau đó không bị "rơi" vào menu.
- Công cụ chuyển mã giữ nguyên chữ hoa khi Kiểu chữ là "Giữ nguyên", kể cả khi bỏ dấu ("Tiếng Việt" → "Tieng Viet"). Chuyển mã mặc định từ TCVN3 sang Unicode.
- Gõ email và từ có ký tự như @ ! # không còn bị bỏ dấu (user@gmail.com trước đây thành ủe@gmail.com). Gõ "www." giữ đủ ba chữ w.
- Gõ tắt có sẵn vài từ ở lần chạy đầu: ko → không, dc → được, vs → với, mn → mọi người, ntn → như thế nào. Gõ được tiếng Việt trong cửa sổ Bảng gõ tắt.
- Giao diện tự dùng tiếng Việt khi Windows để vùng hoặc định dạng Việt Nam, kể cả Windows tiếng Anh.
- Lần đầu chạy từ thư mục Tải về hoặc thư mục tạm, EasyKey đề nghị chuyển sang thư mục cố định để không mất khi dọn thư mục.
- "Kiểm tra cập nhật" báo "Đang kiểm tra…", và báo "Chưa kiểm tra được" khi mất mạng thay vì "đã là bản mới nhất".
- Nhấp đúp biểu tượng khay chỉ mở Cài đặt, không đổi Việt/Anh hai lần.
- Chế độ game không còn hiện tên nội bộ "TheEasyKeyProject" khi chưa dùng ứng dụng nào. Đường dẫn chạy cùng Windows có ngoặc kép, đúng cả khi tên người dùng có dấu cách.
- Popup đổi Việt/Anh vẽ bằng Direct2D và DirectWrite: nét chữ sắc, không vỡ khi Windows phóng to màn hình trên 100%, chữ trong ô được căn giữa.

## 1.0.6 (Windows 11 x64)
- Báo có bản mới: EasyKey tự kiểm tra khi khởi động và mỗi 6 giờ (bật sẵn, tắt được trong Cài đặt › Hệ thống). Khi có bản mới, biểu tượng khay có mũi tên xanh, menu khay hiện "Cập nhật lên …" và cuối cửa sổ Cài đặt có dòng cập nhật; EasyKey không tự cài khi chưa hỏi.
- Chế độ game: bật cho từng game trong menu khay ("Chế độ game cho …") hoặc Cài đặt › Ứng dụng › Chế độ game. Trong game, phím đi thẳng vào game nên Q, W, E, R… không bị đổi thành chữ có dấu; bấm Enter mở khung chat thì gõ được tiếng Việt, Enter (gửi) hoặc Esc (đóng) thì trở lại.
- Phím tắt đổi kiểu gõ chạy được cả khi đang gõ tiếng Anh; popup hiện kiểu gõ mới kèm dòng "dùng khi gõ tiếng Việt".
- README có link tải thẳng `EasyKey.exe` bản mới nhất.

## 1.0.5 (Windows 11 x64)
- Popup cũng hiện khi đổi bảng mã từ menu ở khay hệ thống hay trong cửa sổ Cài đặt, với tên bảng mã, ví dụ "TCVN3".
- Đang gõ tiếng Anh mà đổi kiểu gõ hay bảng mã thì popup hiện đúng lựa chọn mới, kèm dòng "dùng khi gõ tiếng Việt" (trước đây popup chỉ hiện "Tiếng Anh").
- Bỏ hai tùy chọn "Tạm tắt chính tả bằng phím Ctrl" và "Tạm tắt EasyKey bằng phím Alt". Bấm Esc ngay sau một từ để lấy lại phím vừa gõ vẫn làm được việc này, còn bấm Alt một mình trên Windows lại nhảy vào thanh menu của ứng dụng.
- Phần tiếng Anh của giao diện, README và nhật ký thay đổi thống nhất dùng "character encoding" thay cho "code table".
- README ngắn gọn hơn, có ảnh chụp giao diện; hướng dẫn tự build chuyển sang [docs/BUILD.md](https://github.com/iamcino/EasyKey/blob/master/docs/BUILD.md).

## 1.0.4 (Windows 11 x64)
- Sửa lỗi chọn bảng mã cũ (VNI Windows, TCVN3, Unicode tổ hợp…) hoặc Việt/Anh trong cửa sổ Cài đặt hay menu khay rồi quay lại ứng dụng thì bị trả về lựa chọn cũ, ví dụ về Unicode. EasyKey giờ nhớ lựa chọn cho đúng ứng dụng bạn đang gõ.
- Popup kiểu gõ cũng hiện khi đổi kiểu gõ từ menu ở khay hệ thống hay trong cửa sổ Cài đặt.
- Khi bật "Chạy với quyền admin", EasyKey tự khởi động qua Task Scheduler bằng API của Windows thay cho lệnh schtasks. Tác vụ không còn bị Windows dừng sau 72 giờ, vẫn chạy khi máy dùng pin, và nếu không tạo được tác vụ thì EasyKey vẫn khởi động cùng Windows (không có quyền admin).
- Dọn phần đầu các file mã nguồn: ghi công OpenKey (Mai Vũ Tuyên) được giữ, thêm SPDX GPL-3.0-only; bỏ file mã cho macOS không còn dùng.

## 1.0.3 (Windows 11 x64)
- Kiểm tra cập nhật chạy ngầm, chuột và phím không còn bị khựng trong lúc kiểm tra hay tải bản mới.
- Thêm "Kiểm tra cập nhật…" vào menu ở khay hệ thống.
- Phần Nâng cao: mọi nhóm đều đóng khi mở ra, mỗi lần chỉ mở một nhóm, và mở nhóm nào thì cuộn để thấy trọn nhóm đó.
- Mang lại tùy chọn lối tắt trên màn hình nền (Nâng cao › Hệ thống).
- Icon ở khay hệ thống: chữ V (đang gõ tiếng Việt) theo màu nhấn, chữ E giữ đen trắng.
- Phím tắt đổi kiểu gõ (Nâng cao › Gõ, mặc định tắt): mỗi lần bấm chuyển Telex → VNI → Simple Telex và hiện popup, chỉ có tác dụng khi đang gõ tiếng Việt.
- Popup Việt/Anh cũng hiện khi đổi từ menu ở khay hệ thống hay trong cửa sổ Cài đặt.
- Chọn chế độ màu Theo Windows, Sáng hoặc Tối (Nâng cao › Giao diện).
- Vẽ lại cửa sổ Giới thiệu, Bảng gõ tắt và Công cụ chuyển mã theo kiểu cửa sổ Cài đặt, theo chế độ màu sáng/tối và màu nhấn.
- Các hộp hỏi và thông báo (cập nhật, đặt lại cài đặt, gỡ EasyKey, cảnh báo bộ gõ khác…) cũng theo kiểu EasyKey. Thông báo ngắn như "đã là bản mới nhất" hay "đã chuyển mã" hiện ở popup.

## 1.0.2 (Windows 11 x64)
- Sửa lỗi gõ thừa chữ trên Chrome, Edge ở các trang tự xử lý phím như trang luyện gõ (ví dụ "số" thành "sốôô"), cả Telex và VNI.
- Gõ nhanh (cc → ch, gg → gi, nn → ng…) dùng được với mọi kiểu gõ, kể cả VNI; tên tiếng Anh đổi từ "Quick Telex" thành "Quick typing".

## 1.0.1 (Windows 11 x64)
- Mở bảng gõ tắt ngay trong cửa sổ cài đặt: "Nâng cao" › "Gõ" › "Bảng gõ tắt".
- Esc trả lại phím vừa gõ giờ chạy được trong khung chat web như Facebook (EasyKey giữ luôn lần nhả phím Esc, nên trang web không đóng khung chat).
- Viết hoa đầu câu giờ chạy cả sau dấu "?" và "!", không chỉ sau dấu "." và Enter.
- Viết hoa đầu câu: chữ đầu tiên gõ sau khi chuyển sang cửa sổ khác cũng được viết hoa.
- Phần "Nâng cao" gọn hơn: các nhóm thu lại, bấm vào tên nhóm mới mở ra, nhóm "Gõ" mở sẵn. Tiêu đề nhóm to và đậm hơn.
- Sửa lỗi ô có gợi ý tự động và sửa lỗi trên Chrome, Edge luôn bật, không còn công tắc. Bỏ các mục "Mở cửa sổ này khi EasyKey khởi động", "Tạo lối tắt trên Desktop" và "Kiểm tra bản mới ngay" (đã có trong Giới thiệu). Muốn mở cửa sổ cài đặt, chạy lại EasyKey.exe hoặc chọn Cài đặt ở menu khay.

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
