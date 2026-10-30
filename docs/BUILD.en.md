# Build EasyKey yourself

[Tiếng Việt](BUILD.md) · **English**

You can build EasyKey yourself instead of downloading a release. How EasyKey works and how the source is organized is explained in the [design document](DESIGN.en.md).

**Option 1: build on your PC**
1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) (the free Community edition) with the **Desktop development with C++** workload (it includes MSVC v143 and the Windows SDK).
2. Get the source: `git clone https://github.com/iamcino/EasyKey.git`, or Code › Download ZIP on GitHub.
3. Open `Sources/EasyKey/win32/EasyKey/EasyKey.sln`, choose the `Release` configuration and the `x64` platform, then Build › Build Solution.
4. The exe is at `Sources/EasyKey/win32/EasyKey/x64/Release/EasyKey.exe`.

Or build from the command line in "Developer PowerShell for VS 2022":

```
msbuild Sources\EasyKey\win32\EasyKey\EasyKey.sln -p:Configuration=Release -p:Platform=x64
```

**Option 2: build on GitHub, nothing to install**
Fork this repository. Every push or pull request to `master` makes GitHub Actions run the engine tests and build `EasyKey.exe`. Open the Actions tab, open the latest run and download `EasyKey` under Artifacts. Pushing a tag that matches `versionName` in `version.json` (for example `1.0.0`), or running the MSBuild workflow by hand on `master` (Actions › MSBuild › Run workflow), publishes a release.

**Run the engine tests** (needs [CMake](https://cmake.org) and a C++ compiler, works on Windows and Linux):

```
cmake -S Sources/EasyKey/tests -B build
cmake --build build --config Release
ctest --test-dir build -C Release
```

A build of your own is not signed either, so you may see the warnings described in [Windows warnings](../README.en.md#windows-warnings).
