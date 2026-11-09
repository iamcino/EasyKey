<#
.SYNOPSIS
  Builds, signs and publishes an EasyKey release on GitLab, the same way the GitHub MSBuild
  workflow does: signed EasyKey.exe, zip, .sha256 files, SHA256SUMS.txt, the public certificate,
  signature.json and release notes taken from both changelogs (Vietnamese first).

.DESCRIPTION
  Run it on a Windows machine with Visual Studio 2022 (or its Build Tools) and the Windows SDK.
  The release is built from a clean temporary worktree of -Ref, so your own checkout is not touched.

  Signing, first match wins:
    1. the pinned EasyKey certificate in Cert:\CurrentUser\My
    2. <repo>\.signing\EasyKey-codesign.pfx with the DPAPI password file made by new-cert.ps1
    3. -PfxPath (you are asked for the password)

  GitLab access: a token limited to this project (Packages and Registry, Releases, Repository),
  read from GITLAB_TOKEN, from %LOCALAPPDATA%\EasyKey-signing\gitlab-token.clixml (DPAPI-protected,
  see below), or asked for when the script starts. The script never writes it anywhere.

  Same files on both sites: after the GitHub workflow has published a release, -FromGitHub
  downloads that release's files (VirusTotal report included) and publishes exactly those on
  GitLab instead of building again. -AssetsDir does the same from a folder you already have.
  Either way EasyKey.exe must carry the pinned signature and match SHA256SUMS.txt.

.EXAMPLE
  .\tools\release-gitlab.ps1 -FromGitHub      # normal case: mirror the GitHub release
.EXAMPLE
  .\tools\release-gitlab.ps1 -Ref 43ac08d     # GitHub unavailable: build and sign here
#>
# SPDX-License-Identifier: GPL-3.0-only OR MIT
param(
    [string]$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path,
    [string]$Ref = 'master',
    [string]$Project = $(if ($env:CI_PROJECT_PATH) { $env:CI_PROJECT_PATH } else { 'cino-group/easykey' }),
    [string]$GitLab = $(if ($env:CI_SERVER_URL) { $env:CI_SERVER_URL } else { 'https://gitlab.com' }),
    [string]$PfxPath,
    [switch]$FromGitHub,
    [string]$AssetsDir,
    [switch]$SkipUpdaterCheck,
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2

function Step($text) { Write-Host "`n== $text" -ForegroundColor Cyan }
function Quiet { $old = $ErrorActionPreference; $ErrorActionPreference = 'Continue'; try { & $args[0] @($args | Select-Object -Skip 1) 2>&1 | Out-Null } finally { $ErrorActionPreference = $old } }
function Run($exe) {
    & $exe @args
    if ($LASTEXITCODE -ne 0) { throw "$exe failed with exit code $LASTEXITCODE" }
}

$projectId = [Uri]::EscapeDataString($Project)
$api = "$GitLab/api/v4/projects/$projectId"

# ---------------------------------------------------------------- token
$token = $env:GITLAB_TOKEN
# saved once with: Read-Host -AsSecureString | Export-Clixml "$env:LOCALAPPDATA\EasyKey-signing\gitlab-token.clixml"
# (DPAPI: only this Windows user on this machine can read it)
$tokenFile = Join-Path $env:LOCALAPPDATA 'EasyKey-signing\gitlab-token.clixml'
if (-not $token -and (Test-Path $tokenFile)) {
    $token = [Runtime.InteropServices.Marshal]::PtrToStringBSTR(
        [Runtime.InteropServices.Marshal]::SecureStringToBSTR((Import-Clixml $tokenFile)))
}
$headers = @{ 'PRIVATE-TOKEN' = $token }
if (-not $token -and $env:CI_JOB_TOKEN) {
    # inside GitLab CI the job token can upload packages and create the release
    $headers = @{ 'JOB-TOKEN' = $env:CI_JOB_TOKEN }
} elseif (-not $token -and -not $DryRun) {
    $secure = Read-Host 'GitLab access token (scope: api)' -AsSecureString
    $token = [Runtime.InteropServices.Marshal]::PtrToStringBSTR(
        [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secure))
    $headers = @{ 'PRIVATE-TOKEN' = $token }
}

# ---------------------------------------------------------------- worktree
Step "Preparing a clean copy of $Ref"
$commit = (git -C $Repo rev-parse --verify "$Ref^{commit}").Trim()
if ($LASTEXITCODE -ne 0) { throw "Unknown ref $Ref" }
$work = Join-Path $env:TEMP "easykey-release-$($commit.Substring(0, 8))"
if (Test-Path $work) {
    Quiet git -C $Repo worktree remove --force $work
    Remove-Item -Recurse -Force $work -ErrorAction SilentlyContinue
}
Run git -C $Repo worktree add --detach $work $commit

try {
    Set-Location $work
    $version = (Get-Content version.json -Raw | ConvertFrom-Json).latestWinVersion.versionName
    Write-Host "Version $version, commit $commit"
    if ($env:CI_COMMIT_TAG -and $env:CI_COMMIT_TAG -ne $version) {
        throw "Tag $($env:CI_COMMIT_TAG) does not match version.json versionName $version"
    }

    if (-not $DryRun) {
        $exists = $true
        try { Invoke-RestMethod -Headers $headers "$api/releases/$version" | Out-Null }
        catch { $exists = $false }
        if ($exists) { throw "GitLab release $version already exists. Delete it first or raise versionName." }
    }

    $out = Join-Path $work 'release-out'
    if ($FromGitHub -or $AssetsDir) {
        New-Item -ItemType Directory -Force $out | Out-Null
        if ($FromGitHub) {
            Step "Downloading GitHub release $version"
            $gh = Invoke-RestMethod -Headers @{ 'User-Agent' = 'easykey-release' } "https://api.github.com/repos/iamcino/EasyKey/releases/tags/$version"
            foreach ($asset in $gh.assets) {
                Invoke-WebRequest -UseBasicParsing -Headers @{ 'User-Agent' = 'easykey-release' } -Uri $asset.browser_download_url -OutFile (Join-Path $out $asset.name)
                Write-Host "  $($asset.name)"
            }
        } else {
            Copy-Item (Join-Path $AssetsDir '*') $out
        }
        Step 'Checking the files'
        . .\tools\signing\common.ps1
        & .\tools\signing\verify.ps1 -Path (Join-Path $out 'EasyKey.exe')
        foreach ($line in Get-Content (Join-Path $out 'SHA256SUMS.txt')) {
            if (-not $line.Trim()) { continue }
            $hash, $name = $line -split '\s+', 2
            $actual = (Get-FileHash (Join-Path $out $name) -Algorithm SHA256).Hash.ToLower()
            if ($actual -ne $hash.ToLower()) { throw "$name does not match SHA256SUMS.txt" }
            Write-Host "  $name OK"
        }
    } else {
        # ------------------------------------------------------------ build
        Step 'Building Release|x64'
        $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
        $msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
        if (-not $msbuild) { throw 'MSBuild not found. Install Visual Studio 2022 with Desktop development with C++.' }
        $sln = 'Sources\EasyKey\win32\EasyKey\EasyKey.sln'
        # slash switches: PowerShell splits "-name:value" arguments passed through a function
        Run $msbuild /m /nologo /v:minimal /t:Rebuild /p:Configuration=Release /p:Platform=x64 $sln
        $built = 'Sources\EasyKey\win32\EasyKey\x64\Release\EasyKey.exe'

        # ------------------------------------------------------------ sign
        Step 'Signing'
        . .\tools\signing\common.ps1
        $pinned = Get-PinnedThumbprint
        $localPfx = Join-Path $Repo '.signing\EasyKey-codesign.pfx'
        if (Test-Path "Cert:\CurrentUser\My\$pinned") {
            Write-Host 'Using the certificate in CurrentUser\My'
            & .\tools\signing\sign.ps1 -Path $built
        } else {
            if (-not $PfxPath -and (Test-Path $localPfx)) { $PfxPath = $localPfx }
            if (-not $PfxPath) { throw "No signing key: certificate $pinned is not in CurrentUser\My and no PFX was given (-PfxPath)." }
            if (-not $env:EASYKEY_SIGN_PFX_PASSWORD) {
                $pwFile = Join-Path $env:LOCALAPPDATA 'EasyKey-signing\pfx-password.clixml'
                $securePw = if (Test-Path $pwFile) { Import-Clixml $pwFile } else { Read-Host 'PFX password' -AsSecureString }
                $env:EASYKEY_SIGN_PFX_PASSWORD = [Runtime.InteropServices.Marshal]::PtrToStringBSTR(
                    [Runtime.InteropServices.Marshal]::SecureStringToBSTR($securePw))
            }
            Write-Host "Using $PfxPath"
            try { & .\tools\signing\sign.ps1 -Path $built -PfxPath $PfxPath }
            finally { Remove-Item Env:\EASYKEY_SIGN_PFX_PASSWORD -ErrorAction SilentlyContinue }
        }
        # absolute path: .NET file calls use the process directory, not PowerShell's location
        & .\tools\signing\verify.ps1 -Path $built -JsonPath (Join-Path $work 'signature.json')

        # the exact check the 1.0.23+ updater runs; a release that fails it strands every user
        if (-not $SkipUpdaterCheck) {
            Step "Checking the signature the way the updater does"
            $cmake = (Get-Command cmake -ErrorAction SilentlyContinue | Select-Object -First 1).Source
            if (-not $cmake) {
                # the copy Visual Studio ships with its "C++ CMake tools" component
                $vsRoot = & $vswhere -latest -products * -property installationPath
                $candidate = Join-Path $vsRoot 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
                if (Test-Path $candidate) { $cmake = $candidate }
            }
            if (-not $cmake) {
                throw 'cmake not found. Run "winget install Kitware.CMake", open a new PowerShell and try again (or pass -SkipUpdaterCheck).'
            }
            Run $cmake -S Sources/EasyKey/tests -B build/signature-check
            Run $cmake --build build/signature-check --config Release --target UpdateSignatureTests
            Run (Join-Path $work 'build\signature-check\Release\UpdateSignatureTests.exe') (Get-SignTool) (Resolve-Path $built).Path
        }

        # ------------------------------------------------------------ package
        Step 'Packaging'
        New-Item -ItemType Directory -Force $out | Out-Null
        Copy-Item $built (Join-Path $out 'EasyKey.exe')
        Copy-Item (Join-Path $work 'signature.json') $out
        Copy-Item docs\signing\EasyKey-codesign.cer $out
        Push-Location $out
        $zip = "EasyKey$version-Windows.zip"
        Compress-Archive -Path EasyKey.exe -DestinationPath $zip -Force
        $hashExe = (Get-FileHash EasyKey.exe -Algorithm SHA256).Hash.ToLower()
        $hashZip = (Get-FileHash $zip -Algorithm SHA256).Hash.ToLower()
        # the in-app updater reads "<sha256>  EasyKey.exe"
        [IO.File]::WriteAllText("$out\EasyKey.exe.sha256", "$hashExe  EasyKey.exe`n")
        [IO.File]::WriteAllText("$out\$zip.sha256", "$hashZip  $zip`n")
        [IO.File]::WriteAllText("$out\SHA256SUMS.txt", "$hashExe  EasyKey.exe`n$hashZip  $zip`n")
        Pop-Location
        Write-Host "EasyKey.exe sha256 $hashExe"

    }

    # ------------------------------------------------------------ notes
    function Get-Section($file) {
        $section = @(); $found = $false
        foreach ($line in Get-Content $file -Encoding utf8) {
            if ($line.StartsWith('## ')) {
                if ($found) { break }
                $found = $line -eq "## $version" -or $line.StartsWith("## $version ")
            } elseif ($found) { $section += $line }
        }
        if (-not $found) { throw "$file has no section for $version" }
        ($section -join "`n").Trim()
    }
    $blob = "$GitLab/$Project/-/blob/master"
    $notes = (Get-Section CHANGELOG.md) + "`n`n**English**`n`n" + (Get-Section CHANGELOG.en.md) +
        "`n`n[Nhật ký thay đổi]($blob/CHANGELOG.md) · [Changelog]($blob/CHANGELOG.en.md)`n"
    [IO.File]::WriteAllText("$out\release-notes.md", $notes, (New-Object Text.UTF8Encoding $false))

    $files = @(Get-ChildItem $out -File | Where-Object Name -ne 'release-notes.md' | ForEach-Object Name)
    if ($DryRun) {
        Step "Dry run: nothing uploaded. Files are in $out"
        return
    }

    # ------------------------------------------------------------ upload
    Step 'Uploading to the GitLab package registry'
    $links = @()
    foreach ($f in $files) {
        $url = "$api/packages/generic/easykey/$version/$f"
        Invoke-RestMethod -Method Put -Headers $headers -InFile (Join-Path $out $f) -Uri $url | Out-Null
        Write-Host "  $f"
        $links += @{ name = $f; url = $url; direct_asset_path = "/$f"; link_type = $(if ($f -like '*.exe' -or $f -like '*.zip') { 'package' } else { 'other' }) }
    }

    Step "Creating release $version"
    $body = @{
        name = "EasyKey $version"
        tag_name = $version
        ref = $commit
        description = $notes
        assets = @{ links = $links }
    } | ConvertTo-Json -Depth 5
    $release = Invoke-RestMethod -Method Post -Headers $headers -Uri "$api/releases" `
        -ContentType 'application/json; charset=utf-8' -Body ([Text.Encoding]::UTF8.GetBytes($body))
    Write-Host "Done: $GitLab/$Project/-/releases/$version"
    Write-Host "Latest download: $GitLab/$Project/-/releases/permalink/latest/downloads/EasyKey.exe"
}
finally {
    Set-Location $Repo
    Quiet git -C $Repo worktree remove --force $work
}
