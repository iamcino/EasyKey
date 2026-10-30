# Đóng góp · Contributing

**Tiếng Việt** · [English](#english)

Cảm ơn bạn đã quan tâm tới EasyKey! Mọi báo lỗi, góp ý và pull request đều
được hoan nghênh.

## Báo lỗi

Mở [issue](https://github.com/iamcino/EasyKey/issues) kèm:

- phiên bản EasyKey (Cài đặt › Giới thiệu) và Windows (`winver`)
- chế độ (Tiếng Việt, Anh–Việt), kiểu gõ, bảng mã và ứng dụng đang gõ
- bạn đã gõ phím gì, mong đợi gì và thực tế ra gì
- với lỗi gõ ra chữ sai, nhật ký phím (xem
  [Gõ ra chữ sai](README.md#gõ-ra-chữ-sai))

Vấn đề bảo mật vui lòng báo theo [SECURITY.md](SECURITY.md), không mở issue
công khai.

## Phát triển

1. Cài Visual Studio 2022 với workload **Desktop development with C++** (xem
   [docs/BUILD.md](docs/BUILD.md)).
2. Mở `Sources/EasyKey/win32/EasyKey/EasyKey.sln`, build `Release|x64` và chạy.
   Nhớ thoát bản EasyKey đang chạy trước.
3. Đọc [tài liệu thiết kế](docs/DESIGN.md) để hiểu đường đi của một phím.
4. Trước khi mở pull request:
   - chạy test engine (`cmake -S Sources/EasyKey/tests -B build`,
     `cmake --build build`, `ctest --test-dir build`), chạy được cả trên Linux
   - thử trên Windows 11 thật ở vài ứng dụng: Notepad, một trình duyệt
     Chromium, một ứng dụng Electron (Discord, VS Code)
   - ghi rõ bạn đã thử những gì, trên bản Windows nào

Nguyên tắc:

- **Một file, gọn nhẹ.** Không thêm DLL, runtime hay trình cài đặt. Tài nguyên
  (font, biểu tượng, dữ liệu) nhúng vào `EasyKey.exe`.
- **Hook phải nhanh.** Không gọi mạng, đọc file hay chờ lâu trong hook bàn
  phím; việc chậm chạy ở luồng riêng.
- **Engine thuần C++.** `Sources/EasyKey/engine` không gọi Windows API. Sửa
  engine thì thêm ca test cho lỗi vừa sửa vào `EngineTests.cpp`.
- **Giao diện song ngữ.** Mọi chữ mới trên giao diện cần cả bản tiếng Việt và
  tiếng Anh (`TR("…", "…")`). Phần tiếng Anh dùng "character encoding", không
  dùng "code table".
- **Không cần quyền Admin.** Chỉ ghi vào `HKCU` và thư mục của người dùng.
- **Nhật ký thay đổi.** Ghi thay đổi người dùng thấy được vào cả
  [CHANGELOG.md](CHANGELOG.md) và [CHANGELOG.en.md](CHANGELOG.en.md), viết cho
  người dùng chứ không phải cho lập trình viên.
- **Ghi công.** Giữ nguyên dòng bản quyền OpenKey ở đầu các file có sẵn.

Khi đóng góp, bạn đồng ý rằng phần đóng góp được cấp phép theo
[GNU GPL phiên bản 3](LICENSE) (và MIT với các file ghi
`GPL-3.0-only OR MIT`).

---

## English

Thanks for your interest in EasyKey! Bug reports, ideas and pull requests are
all welcome.

### Reporting bugs

Open an [issue](https://github.com/iamcino/EasyKey/issues) with:

- the EasyKey version (Settings › About) and Windows version (`winver`)
- the mode (Vietnamese, Mixed), input method, encoding and the app you typed in
- the keys you typed, what you expected and what you got
- for wrong text, a key log (see [Wrong text](README.en.md#wrong-text))

Security problems go through [SECURITY.md](SECURITY.md), not public issues.

### Development

1. Install Visual Studio 2022 with the **Desktop development with C++**
   workload (see [docs/BUILD.en.md](docs/BUILD.en.md)).
2. Open `Sources/EasyKey/win32/EasyKey/EasyKey.sln`, build `Release|x64` and
   run it. Quit the EasyKey you already run first.
3. Read the [design document](docs/DESIGN.en.md) to follow a key through the
   app.
4. Before opening a pull request:
   - run the engine tests (`cmake -S Sources/EasyKey/tests -B build`,
     `cmake --build build`, `ctest --test-dir build`); they also run on Linux
   - try it on real Windows 11 in a few apps: Notepad, a Chromium browser, an
     Electron app (Discord, VS Code)
   - say what you tried and on which Windows build

Guidelines:

- **One small file.** No extra DLLs, runtimes or installer. Resources (fonts,
  icons, data) are embedded in `EasyKey.exe`.
- **Keep the hook fast.** No network, file access or long waits inside the
  keyboard hook; slow work runs on its own thread.
- **Plain C++ engine.** `Sources/EasyKey/engine` makes no Windows API calls.
  When you fix the engine, add a test case for the bug to `EngineTests.cpp`.
- **Bilingual interface.** Every new piece of UI text needs a Vietnamese and an
  English version (`TR("…", "…")`). English text says "character encoding",
  never "code table".
- **No administrator rights.** Write only to `HKCU` and the user's folders.
- **Changelog.** Add user-visible changes to both
  [CHANGELOG.md](CHANGELOG.md) and [CHANGELOG.en.md](CHANGELOG.en.md), written
  for users rather than developers.
- **Credits.** Keep the OpenKey copyright lines at the top of existing files.

By contributing, you agree that your contribution is licensed under the
[GNU GPL version 3](LICENSE) (and MIT for files marked
`GPL-3.0-only OR MIT`).
