<#
.SYNOPSIS
  Verifies the Authenticode signature of a file against the pinned EasyKey certificate.

.DESCRIPTION
  Fails when the file is unsigned, its hash does not match, the signer is not the pinned
  certificate, or there is no timestamp. A self-signed root that this machine does not trust
  is accepted (status UnknownError/NotTrusted) because the thumbprint pin is the real check.
  With -JsonPath, writes signature.json.
#>
# SPDX-License-Identifier: GPL-3.0-only OR MIT
param(
    [Parameter(Mandatory)][string]$Path,
    [string]$ExpectedThumbprint,
    [string]$JsonPath
)

. (Join-Path $PSScriptRoot 'common.ps1')

$Path = (Resolve-Path $Path).Path
$pinned = Get-PinnedThumbprint
if ($ExpectedThumbprint -and $ExpectedThumbprint.ToUpperInvariant() -ne $pinned) {
    throw "Expected thumbprint $ExpectedThumbprint does not match the committed public certificate ($pinned)."
}

$sig = Get-AuthenticodeSignature -FilePath $Path
if ($sig.Status -in @('NotSigned', 'HashMismatch', 'NotSupportedFileFormat', 'Incompatible')) {
    throw "Invalid signature on ${Path}: $($sig.Status) $($sig.StatusMessage)"
}
if (-not $sig.SignerCertificate) { throw "No signer certificate on $Path" }
$thumbprint = $sig.SignerCertificate.Thumbprint.ToUpperInvariant()
if ($thumbprint -ne $pinned) { throw "Signed by $thumbprint, expected $pinned" }
if ($sig.Status -notin @('Valid', 'UnknownError', 'NotTrusted')) {
    throw "Unexpected signature status on ${Path}: $($sig.Status) $($sig.StatusMessage)"
}
if (-not $sig.TimeStamperCertificate) { throw "$Path is signed but not timestamped" }

$cert = $sig.SignerCertificate
Write-Host "Signature OK: $Path"
Write-Host "  Signer     : $($cert.Subject)"
Write-Host "  Thumbprint : $thumbprint"
Write-Host "  Timestamp  : $($sig.TimeStamperCertificate.Subject)"
Write-Host "  Status     : $($sig.Status) (self-signed roots are only trusted where installed)"

if ($JsonPath) {
    $info = [ordered]@{
        file                = Split-Path $Path -Leaf
        sha256              = (Get-FileHash $Path -Algorithm SHA256).Hash.ToLowerInvariant()
        signer              = $cert.Subject
        thumbprintSha1      = $thumbprint
        keyAlgorithm        = $cert.PublicKey.Oid.FriendlyName
        signatureAlgorithm  = $cert.SignatureAlgorithm.FriendlyName
        certificateNotAfter = $cert.NotAfter.ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ')
        selfSigned          = ($cert.Subject -eq $cert.Issuer)
        timestampAuthority  = $sig.TimeStamperCertificate.Subject
        status              = "$($sig.Status)"
        verifiedAt          = (Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ')
    }
    [IO.File]::WriteAllText($JsonPath, ($info | ConvertTo-Json) + "`n")
    Write-Host "Wrote $JsonPath"
}
