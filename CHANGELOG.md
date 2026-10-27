# Nhật ký thay đổi

**Tiếng Việt** · [English](CHANGELOG.en.md)

## Bản tiếp theo
- Báo có bản mới: EasyKey tự kiểm tra khi khởi động và mỗi 6 giờ (bật sẵn, tắt được trong Cài đặt › Hệ thống). Khi có bản mới, biểu tượng khay có mũi tên xanh, menu khay hiện "Cập nhật lên …" và cuối cửa sổ Cài đặt có dòng cập nhật; EasyKey không tự cài khi chưa hỏi.
- Chế độ game: bật cho từng game trong menu khay ("Chế độ game cho …") hoặc Cài đặt › Ứng dụng › Chế độ game. Trong game, phím đi thẳng vào game nên Q, W, E, R… không bị đổi thành chữ có dấu; bấm Enter mở khung chat thì gõ được tiếng Việt, Enter (gửi) hoặc Esc (đóng) thì trở lại.
- Phím tắt đổi kiểu gõ chạy được cả khi đang gõ tiếng Anh; popup hiện kiểu gõ mới kèm dòng "dùng khi gõ tiếng Việt".

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
