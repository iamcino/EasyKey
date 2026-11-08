# EasyKey: đề xuất nghiên cứu cho Qwen

Ngày: 2026-11-04. Gốc local: `master`, commit `dc776af`.

## Phạm vi và quyết định đã chốt

- Source local là nguồn làm việc chính. Giữ nguyên các thay đổi chưa commit; không reset theo GitHub.
- Giữ SHA-256 cho updater, Authenticode, timestamp, VirusTotal và provenance; không chuyển sang BLAKE3.
- Mọi thao tác git chỉ local. Không push/tạo PR/phát hành/cài root certificate cho người dùng.
- Đọc `AGENTS.md` trước khi sửa. Các đề xuất dưới đây cần nghiên cứu, không phải yêu cầu triển khai toàn bộ ngay.

## Những thay đổi đã làm trong working tree

### Engine và macro

- `engine/Macro.cpp`: kiểm tra biên khi đọc dữ liệu nhị phân, chỉ nhận toàn bộ bảng hợp lệ; kiểm tra giới hạn tên 255 byte UTF-8, nội dung 65535 byte, số mục 65535 trước khi thêm; thay thế buffer đầu ra khi lưu.
- `MacroWindow.cpp`: không báo lưu thành công và không xóa nội dung người dùng khi `addMacro` thất bại.
- `engine/Engine.cpp`: không xét lại bằng score mặc định những từ đã được quyết định bằng phrase/curated/protection rules. Tái hiện cũ: `banhs test case ` bị đổi thành “bánh test case”; `mootj too software ` bị đổi thành “một too software”.
- Bộ engine hiện có 351 ca qua trên Windows MSVC. Chưa chạy Linux local.

### Xác minh updater

- SHA-256 và asset `.sha256` giữ nguyên.
- `UpdateSignature.cpp/.h`: xác minh Authenticode bằng WinTrust trước khi thay EXE; yêu cầu primary digest SHA-256, đúng toàn bộ DER chứng thư công khai đã ghim, RFC 3161 timestamp và chain timestamp hợp lệ.
- Chứng thư `docs/signing/EasyKey-codesign.cer` được nhúng thành RCDATA. Không đọc pin từ mạng hoặc từ file cạnh EXE.
- Chứng thư EasyKey hiện là self-signed. Chain engine chỉ dành cho signer dùng exclusive root trong bộ nhớ; chain timestamp dùng Windows trust thông thường. Không sửa certificate stores của máy, không bỏ qua chung lỗi untrusted root/hash mismatch.
- Chỉ chấp nhận một primary signer; multi-signature chưa được hỗ trợ.
- Staging dùng tên ngẫu nhiên, CREATE_NEW, khóa ghi/xóa và rename bằng chính handle đã xác minh, tránh đổi file giữa verify và install.
- Workflow release dừng nếu thiếu signing credentials, thay vì phát hành unsigned rồi bị updater mới từ chối.
- Windows integration tests dùng signtool.exe có embedded signature từ SDK local: kiểm tra thành công, unsigned, sai pin, malformed pin, missing file, sửa nội dung, thiếu recognized timestamp, khóa file và rename giữ nguyên danh tính file.

### Dọn backup `.old.exe` (bổ sung theo yêu cầu người dùng)

- Trước đây chỉ gọi DeleteFile một lần trước khi instance mới khởi tạo, sau tối đa 5 giây chờ window cũ: process/antivirus vẫn có thể giữ image, khiến `.old.exe` còn lại.
- Hiện startup sau update chờ process cũ tối đa 30 giây trước single-instance check; chỉ bắt đầu dọn backup sau khi instance mới đã khởi tạo engine và tray.
- `UpdateCleanup.cpp` chạy worker, retry tối đa 120 lần cách 500 ms, xử lý read-only; file không còn được coi là thành công. Nếu hết retry vẫn không xóa được, giữ backup và thử lại lần khởi động thành công tiếp theo.
- 9 ca test cleanup đã qua: xóa bình thường, file đã mất, lock hết thời gian giữ nguyên file, lock được nhả sau đó thì xóa được, read-only, không xóa thư mục. Không thể đảm bảo xóa khi ACL/antivirus vẫn chặn; chưa có handshake xác nhận toàn bộ readiness của process mới.

### Giới hạn xác minh cần biết

- Test thành công bằng chữ ký Microsoft không thay thế nghiệm thu trên một bản EasyKey thật được ký bởi public pin hiện tại và timestamp RFC 3161.
- Chưa có fixture local cho các ca: self-signed EasyKey hợp lệ trên máy không trust root, certificate đã hết hạn nhưng timestamp hợp lệ, revoked signer/TSA, TSA không có chain cached, RFC 3161 bị sửa và primary certificate/key rotation.
- Verifier hiện dùng cache-only chain retrieval và không kiểm tra revocation online. Điều này tránh Windows tự truy cập mạng khi verify, nhưng không cung cấp phát hiện thu hồi chứng thư đầy đủ. Không diễn giải “signature OK” thành “đã kiểm tra revocation”.
- Full application Release link từng thất bại LNK1101 / MSPDB140.DLL do đường build mặc định chọn host x86 trên máy local. Đã tái hiện bằng cùng một object tối giản: linker `HostX86/x64` lỗi 1101, `HostX64/x64` tạo EXE/PDB thành công. Full Release với `PreferredToolArchitecture=x64` đã link thành công; project hiện mặc định chọn host x64, giữ nguyên `/DEBUG` và LTCG. Đây là sửa lựa chọn host tools, không phải chứng minh DLL trong bộ cài bị hỏng.
- Người dùng đang chạy updater cũ vẫn chưa có kiểm tra chữ ký mới cho bước cập nhật đầu tiên.

## Ưu tiên nghiên cứu

### P1 — Nghiệm thu chính sách chữ ký và timestamp

Xem `UpdateSignature.cpp` và `tools/signing/verify.ps1`.

1. Nghiệm thu bằng bản EasyKey đã ký thật trên Windows sạch, không cài public root EasyKey; phải qua chỉ nhờ pin trong ứng dụng.
2. Chứng minh primary chain được kiểm tra tại authenticated timestamp, không phải tại thời gian hệ thống hiện tại hoặc unauthenticated signingTime. Bổ sung fixture expired signer + valid timestamp và timestamp bị giả mạo.
3. Quyết định revocation rõ ràng: online/offline, timeout, stale cache, hành vi khi CRL/OCSP không tới được. Không tự đổi sang fail-open.
4. Audit `verify.ps1`: hiện chấp nhận trạng thái `UnknownError`/`NotTrusted` khi pin khớp và có `TimeStamperCertificate`. Cần chứng minh mọi lỗi cryptographic/policy thực sự bị từ chối; không chỉ kiểm tra sự hiện diện của certificate.
5. Đánh giá ECDSA Authenticode trong cấu hình chứng thư hiện tại trên các phiên bản Windows và công cụ xác minh mục tiêu.

Nghiệm thu: unsigned, tampered, wrong publisher, missing/invalid timestamp phải bị từ chối dù checksum đúng; không đụng EXE đang chạy khi verify thất bại; không import root toàn hệ thống.

### P1 — Recovery khi cập nhật và restart thất bại

Xem `Updater.cpp`: `install`, `restartAs`, `finishPreviousUpdate`.

- `restartAs` hiện giải phóng engine và thoát ứng dụng kể cả khi `CreateProcess` thất bại. Nghiên cứu phục hồi engine, báo lỗi và giữ phiên hiện tại hoạt động.
- Kiểm tra rollback thất bại, antivirus giữ file, mất điện giữa hai lần rename, đĩa đầy và thư mục không có quyền ghi.
- Nâng điều kiện xác nhận bản mới từ engine/tray initialized thành readiness handshake đầy đủ trước khi xóa `.old.exe`; tránh xóa bản cứu hộ nếu process mới crash ngay sau startup.
- Quy hoạch dọn file staging ngẫu nhiên sót sau crash; không xóa file đang được một phiên update sử dụng.

Nghiệm thu: fault injection tại từng bước I/O/startup không làm mất cả bản chạy được và bản backup; không giữ keyboard hooks trên một thao tác UI/main-thread chậm.

### P1 — Signed metadata và chống rollback

Chữ ký EXE chứng minh nguồn phát hành, chưa ràng buộc nó với version quảng bá trong `version.json`.

- Nghiên cứu manifest được ký, ràng buộc version, SHA-256, kích thước, kiến trúc, URL và hạn hiệu lực. Dùng thư viện/giao thức đã có thay vì tự thiết kế crypto.
- So sánh version của EXE tải về với version đã chọn, cấm hạ cấp ngoài thao tác recovery chủ động.
- Lập kế hoạch key/certificate rotation trước khi đổi pin: phát hành bản cầu nối bằng key cũ, chấp nhận pin mới có kiểm soát; người bỏ qua bản cầu nối cần một đường cập nhật còn hoạt động.

Nghiệm thu: metadata giả trỏ đến bản EasyKey cũ đã ký thật vẫn không gây downgrade; sai kiến trúc/version bị chặn trước install.

### P2 — HTTP, checksum và parser version

Xem `Updater.cpp`: `httpGet`, `isVersionName`, `versionNumber`, `check`, `download`.

- `httpGet` đặt `ok=true` trước vòng đọc; WinHttpReadData thất bại có thể bị coi như EOF thành công. Cần phân biệt lỗi đọc và kết thúc bình thường.
- Đặt timeout rõ ràng và kiểm thử HTTP redirect vẫn HTTPS, quá giới hạn tải, chunked/truncated body và proxy.
- Checksum hiện đọc 64 ký tự đầu. Nghiên cứu parser chặt cho hex, tên file và số dòng; giới hạn riêng cho checksum/metadata thay vì dùng chung 32 MiB.
- Parser version chỉ lọc chữ số/dấu chấm, `sscanf_s` không kiểm tra đủ ba thành phần, minor/patch bị mask 8 bit. Định nghĩa grammar và phạm vi rõ ràng, tránh overflow/truncation.
- Parse JSON bằng parser nhỏ đã được kiểm chứng, thay vì tìm chuỗi có thể đi qua ranh giới object.

Nghiệm thu: truncated response không được báo thành công; malformed/oversized versions và duplicate/conflicting metadata bị xử lý xác định.

### P2 — Hardening dữ liệu local và trải nghiệm macro

- `engine/SmartSwitchKey.cpp::initSmartSwitchKey` chưa kiểm tra biên từng entry theo `size`. Áp dụng parser có bounds và test mọi điểm truncation giống macro.
- Import macro hiện bỏ qua entry bị `addMacro` từ chối nhưng chưa báo số entry bị bỏ. Nghiên cứu trả kết quả import gồm accepted/skipped/errors và giữ nguyên dữ liệu người dùng khi có lỗi I/O.
- Xác định cách xử lý dữ liệu macro nhị phân đã hỏng từ phiên bản cũ: lưu bản backup để phục hồi thay vì ghi đè bảng rỗng ngay khi người dùng thao tác tiếp.
- Fuzz binary parsers và UTF-8, chạy ASan/UBSan trên Linux; mọi standard header phải include trực tiếp theo AGENTS.md.

### P2 — Toolchain và kiểm thử tích hợp

- LNK1101 đã có cách khắc phục local bằng native x64 tools; xem `docs/BUILD.md`. Không cần repair để build được ở host x64. Nếu tiếp tục nghiên cứu nguyên nhân bên trong nhánh host x86, kiểm tra PDB server/DLL thực sự được nạp; các DLL cạnh cả hai linker có version 14.44.35229.0, còn `Common7/IDE/mspdbsrv.exe` là 14.44.35215.0, nhưng chưa chứng minh file cũ này gây lỗi. Không tải DLL rời hoặc chép vào System32.
- Giữ quy trình build xác định rõ MSVC/SDK/host architecture/configuration và log tool versions; không tắt debug symbols hoặc LTCG để che lỗi PDB.
- Một số test password/game hiện mô phỏng logic trong test thay vì gọi implementation Win32 thật; nghiên cứu tách state machine dùng chung để tránh hai bản logic trôi nhau.
- Signature test hiện cần SDK signtool.exe có embedded RFC 3161 signature và Windows root/intermediate cache tương ứng. Nghiên cứu fixtures có provenance, chính sách cập nhật fixture và điều kiện chạy offline tái lập được.
- Sau khi sửa lựa chọn host tools, clean rebuild Release bằng lệnh MSBuild thông thường đã tạo đủ `Sources/EasyKey/win32/EasyKey/x64/Release/EasyKey.exe` (677888 byte) và `EasyKey.pdb` (9269248 byte); log xác nhận linker `HostX64/x64`, `/DEBUG` và `/LTCG`. Log local: `build/link-diagnostics/msbuild-default-fixed.log`.
- Trong lần kiểm tra hồi quy cùng phiên build này, test signature có một lần fail tại “verified locked file can be renamed and still verifies” (16/17), sau đó chạy riêng lại qua 17/17. Chưa xác định lỗi ở rename hay verify sau rename; cần thêm mã lỗi/diagnostic và kiểm tra khả năng lock tạm thời trước khi coi suite hoàn toàn ổn định. Engine và cleanup vẫn qua. Hiện tượng này tách biệt với LNK1101 đã được xử lý.

## Tài liệu tham khảo công khai

- WinTrust: https://learn.microsoft.com/en-us/windows/win32/api/wintrust/nf-wintrust-winverifytrust
- Certificate chain policy: https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certverifycertificatechainpolicy
- SignTool: https://learn.microsoft.com/en-us/windows/win32/seccrypto/signtool
- VirusTotal file identifiers: https://docs.virustotal.com/reference/file-info

Qwen nên phân loại riêng: lỗi đã tái hiện, rủi ro suy luận cần kiểm chứng và cải tiến tùy chọn. Không coi tài liệu này là chứng nhận bảo mật hay kết quả audit toàn bộ repo.
