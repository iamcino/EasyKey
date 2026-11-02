# Shared helpers for the EasyKey signing scripts. Dot-source this file.
# SPDX-License-Identifier: GPL-3.0-only OR MIT

$ErrorActionPreference = 'Stop'

$script:RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
# public certificate, committed; its thumbprint pins every signature
$script:PublicCertPath = Join-Path $script:RepoRoot 'docs\signing\EasyKey-codesign.cer'
# private material, never committed (.gitignore + CI guard)
$script:SigningDir = Join-Path $script:RepoRoot '.signing'
$script:LocalPfxPath = Join-Path $script:SigningDir 'EasyKey-codesign.pfx'
# PFX password, protected with DPAPI for the current Windows user, kept outside the repo
$script:PasswordPath = Join-Path $env:LOCALAPPDATA 'EasyKey-signing\pfx-password.clixml'

$script:DefaultTimestampUrls = @('http://timestamp.digicert.com', 'http://timestamp.sectigo.com')

function Get-SignTool {
    $root = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\bin'
    $tool = Get-ChildItem -Path $root -Filter signtool.exe -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.Directory.Name -eq 'x64' } |
        Sort-Object { [version]($_.Directory.Parent.Name -replace '[^0-9.]', '') } -Descending |
        Select-Object -First 1
    if (-not $tool) { throw 'signtool.exe not found. Install the Windows 10/11 SDK.' }
    return $tool.FullName
}

function Get-PinnedThumbprint {
    if (-not (Test-Path $script:PublicCertPath)) {
        throw "Public certificate not found: $script:PublicCertPath. Run tools\signing\new-cert.ps1 first."
    }
    $cert = New-Object System.Security.Cryptography.X509Certificates.X509Certificate2($script:PublicCertPath)
    return $cert.Thumbprint.ToUpperInvariant()
}
