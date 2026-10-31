# Nhật ký thay đổi

**Tiếng Việt** · [English](CHANGELOG.en.md)

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
