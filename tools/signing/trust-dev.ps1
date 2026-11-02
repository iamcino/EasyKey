<#
.SYNOPSIS
  Trusts (or with -Remove, untrusts) the EasyKey signing certificate for the current Windows user.

.DESCRIPTION
  Developer machines only. Imports docs\signing\EasyKey-codesign.cer into CurrentUser\Root and
  CurrentUser\TrustedPublisher. Windows asks for confirmation when adding a root.
  Never ask other people to do this: anyone holding the private key could then sign code their
  machine trusts.
#>
# SPDX-License-Identifier: GPL-3.0-only OR MIT
param([switch]$Remove)

. (Join-Path $PSScriptRoot 'common.ps1')

$thumbprint = Get-PinnedThumbprint
foreach ($store in 'Root', 'TrustedPublisher') {
    $location = "Cert:\CurrentUser\$store"
    if ($Remove) {
        if (Test-Path "$location\$thumbprint") {
            Remove-Item "$location\$thumbprint"
            Write-Host "Removed $thumbprint from CurrentUser\$store"
        }
    } else {
        Import-Certificate -FilePath $script:PublicCertPath -CertStoreLocation $location | Out-Null
        Write-Host "Trusted $thumbprint in CurrentUser\$store"
    }
}
