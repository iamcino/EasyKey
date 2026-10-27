# Tự build EasyKey

**Tiếng Việt** · [English](BUILD.en.md)

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

File tự build không có chữ ký số nên cũng có thể gặp cảnh báo như phần [Cảnh báo của Windows](../README.md#cảnh-báo-của-windows).
