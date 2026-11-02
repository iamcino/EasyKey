<#
.SYNOPSIS
  Copies the GitHub Actions signing secrets to the clipboard, one at a time, without printing them.

.DESCRIPTION
  1. EASYKEY_SIGN_PFX_BASE64   (the PFX, base64)
  2. EASYKEY_SIGN_PFX_PASSWORD (the PFX password)
  Paste each into Settings > Secrets and variables > Actions. The clipboard is cleared at the end.
#>
# SPDX-License-Identifier: GPL-3.0-only OR MIT

. (Join-Path $PSScriptRoot 'common.ps1')

if (-not (Test-Path $script:LocalPfxPath)) { throw "PFX not found: $script:LocalPfxPath" }
if (-not (Test-Path $script:PasswordPath)) { throw "Password file not found: $script:PasswordPath" }

try {
    Set-Clipboard -Value ([Convert]::ToBase64String([IO.File]::ReadAllBytes($script:LocalPfxPath)))
    Read-Host 'EASYKEY_SIGN_PFX_BASE64 is on the clipboard. Paste it into GitHub, then press Enter'

    $secure = Import-Clixml -Path $script:PasswordPath
    Set-Clipboard -Value ([System.Net.NetworkCredential]::new('', $secure).Password)
    Read-Host 'EASYKEY_SIGN_PFX_PASSWORD is on the clipboard. Paste it into GitHub, then press Enter'
} finally {
    Set-Clipboard -Value ' '
    Write-Host 'Clipboard cleared.'
}
Write-Host "Also set the variable EASYKEY_EXPECTED_THUMBPRINT to $(Get-PinnedThumbprint)"
