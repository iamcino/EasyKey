<#
.SYNOPSIS
  Creates the EasyKey self-signed code signing certificate (ECDSA P-256, SHA-256, 2 years). Run once.

.DESCRIPTION
  - Private key: CurrentUser\My (re-imported as non-exportable) and .signing\EasyKey-codesign.pfx
    (password protected, for CI). .signing\ is git-ignored.
  - PFX password: random, stored with DPAPI under %LOCALAPPDATA%\EasyKey-signing, never printed.
  - Public certificate: docs\signing\EasyKey-codesign.cer (commit this one).

  A self-signed certificate does not remove SmartScreen or "Unknown publisher" warnings for other
  users; Windows only trusts certificates from CAs in the Microsoft Trusted Root Program.
#>
# SPDX-License-Identifier: GPL-3.0-only OR MIT
param(
    [string]$Subject = 'CN=EasyKey, O=Cino, OU=EasyKey - Bo go tieng Viet, C=VN',
    [int]$Years = 2,
    [switch]$Force
)

. (Join-Path $PSScriptRoot 'common.ps1')

if (((Test-Path $script:LocalPfxPath) -or (Test-Path $script:PublicCertPath)) -and -not $Force) {
    throw "A signing certificate already exists. Use -Force only when rotating it on purpose."
}

New-Item -ItemType Directory -Force -Path $script:SigningDir, (Split-Path $script:PublicCertPath), (Split-Path $script:PasswordPath) | Out-Null

$cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject $Subject `
    -KeyAlgorithm ECDSA_nistP256 -HashAlgorithm SHA256 -KeyUsage DigitalSignature `
    -KeyExportPolicy Exportable -CertStoreLocation Cert:\CurrentUser\My `
    -NotAfter (Get-Date).AddYears($Years)

# random 32 byte password, never shown
$bytes = New-Object byte[] 32
[System.Security.Cryptography.RandomNumberGenerator]::Create().GetBytes($bytes)
$password = ConvertTo-SecureString ([Convert]::ToBase64String($bytes)) -AsPlainText -Force
$password | Export-Clixml -Path $script:PasswordPath

Export-PfxCertificate -Cert $cert -FilePath $script:LocalPfxPath -Password $password | Out-Null
Export-Certificate -Cert $cert -FilePath $script:PublicCertPath -Type CERT | Out-Null

# keep only a non-exportable copy of the key in the store
$thumbprint = $cert.Thumbprint
Remove-Item -Path "Cert:\CurrentUser\My\$thumbprint" -DeleteKey
Import-PfxCertificate -FilePath $script:LocalPfxPath -CertStoreLocation Cert:\CurrentUser\My -Password $password | Out-Null

Write-Host "Created code signing certificate"
Write-Host "  Subject    : $($cert.Subject)"
Write-Host "  Thumbprint : $thumbprint"
Write-Host "  Valid until: $($cert.NotAfter.ToString('yyyy-MM-dd'))"
Write-Host "  Public cert: $script:PublicCertPath (commit)"
Write-Host "  PFX        : $script:LocalPfxPath (never commit)"
Write-Host "Set the GitHub variable EASYKEY_EXPECTED_THUMBPRINT to $thumbprint"
Write-Host "Run tools\signing\ci-secrets.ps1 to copy the CI secrets to the clipboard."
