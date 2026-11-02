<#
.SYNOPSIS
  Signs a file with the EasyKey code signing certificate, timestamps it (RFC 3161) and verifies it.

.DESCRIPTION
  Local : uses the certificate in CurrentUser\My that matches docs\signing\EasyKey-codesign.cer.
  CI    : pass -PfxPath; the password is read from the EASYKEY_SIGN_PFX_PASSWORD environment variable.
  Each timestamp server is tried 3 times before the next one. No timestamp means failure.
#>
# SPDX-License-Identifier: GPL-3.0-only OR MIT
param(
    [Parameter(Mandatory)][string]$Path,
    [string]$PfxPath,
    [string]$ExpectedThumbprint,
    [string[]]$TimestampUrl
)

. (Join-Path $PSScriptRoot 'common.ps1')

$Path = (Resolve-Path $Path).Path
$pinned = Get-PinnedThumbprint
if ($ExpectedThumbprint -and $ExpectedThumbprint.ToUpperInvariant() -ne $pinned) {
    throw "Expected thumbprint $ExpectedThumbprint does not match the committed public certificate ($pinned)."
}
$urls = @($TimestampUrl | Where-Object { $_ })
if ($urls.Count -eq 0) { $urls = $script:DefaultTimestampUrls }
foreach ($url in $urls) {
    if ($url -match 'example\.(com|org|net)') { throw "Refusing documentation-only domain as timestamp server: $url" }
}

$signtool = Get-SignTool
if ($PfxPath) {
    if (-not $env:EASYKEY_SIGN_PFX_PASSWORD) { throw 'EASYKEY_SIGN_PFX_PASSWORD is not set.' }
    $pfxCert = New-Object System.Security.Cryptography.X509Certificates.X509Certificate2($PfxPath, $env:EASYKEY_SIGN_PFX_PASSWORD)
    if ($pfxCert.Thumbprint.ToUpperInvariant() -ne $pinned) {
        throw "The PFX certificate ($($pfxCert.Thumbprint)) is not the pinned certificate ($pinned)."
    }
    $certArgs = @('/f', $PfxPath, '/p', $env:EASYKEY_SIGN_PFX_PASSWORD)
} else {
    if (-not (Test-Path "Cert:\CurrentUser\My\$pinned")) {
        throw "Certificate $pinned not found in CurrentUser\My."
    }
    $certArgs = @('/sha1', $pinned, '/s', 'My')
}

$signed = $false
foreach ($url in $urls) {
    for ($attempt = 1; $attempt -le 3 -and -not $signed; $attempt++) {
        & $signtool sign /fd sha256 /tr $url /td sha256 @certArgs $Path | Out-Host
        if ($LASTEXITCODE -eq 0) {
            $signed = $true
            Write-Host "Signed and timestamped by $url"
        } else {
            Write-Warning "Signing with timestamp server $url failed (attempt $attempt of 3)"
            Start-Sleep -Seconds (2 * $attempt)
        }
    }
    if ($signed) { break }
}
$global:LASTEXITCODE = 0
if (-not $signed) { throw 'Signing failed on every timestamp server; not publishing an untimestamped file.' }

& (Join-Path $PSScriptRoot 'verify.ps1') -Path $Path -ExpectedThumbprint $pinned
