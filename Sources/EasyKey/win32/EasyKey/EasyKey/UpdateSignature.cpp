#include "UpdateSignature.h"
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>
#include <cstring>

#pragma comment(lib, "wintrust.lib")
#pragma comment(lib, "crypt32.lib")

namespace {
struct PolicyContext {
    PCCERT_CONTEXT pin;
    bool accepted = false;
};

PCCERT_CONTEXT leaf(PCCERT_CHAIN_CONTEXT chain) {
    if (!chain || !chain->cChain || !chain->rgpChain[0]->cElement)
        return nullptr;
    return chain->rgpChain[0]->rgpElement[0]->pCertContext;
}

bool sameCertificate(PCCERT_CONTEXT a, PCCERT_CONTEXT b) {
    return a && b && a->cbCertEncoded == b->cbCertEncoded &&
        std::memcmp(a->pbCertEncoded, b->pbCertEncoded, a->cbCertEncoded) == 0;
}

HRESULT WINAPI pinnedPolicy(PCRYPT_PROVIDER_DATA, DWORD stepError, DWORD,
    DWORD signerCount, PWTD_GENERIC_CHAIN_POLICY_SIGNER_INFO* signers, void* pin) {
    auto& context = *static_cast<PolicyContext*>(pin);
    if (stepError != ERROR_SUCCESS)
        return static_cast<HRESULT>(stepError);
    // Release tooling produces one primary signature. Fail closed on ambiguous
    // multi-signatures rather than accidentally accepting an unrelated signer.
    if (signerCount != 1 || !signers || !signers[0])
        return TRUST_E_SUBJECT_NOT_TRUSTED;
    const auto signer = signers[0];
    if (signer->dwError != ERROR_SUCCESS ||
        !sameCertificate(leaf(signer->pChainContext), context.pin) ||
        !signer->pMsgSignerInfo || !signer->pMsgSignerInfo->HashAlgorithm.pszObjId ||
        std::strcmp(signer->pMsgSignerInfo->HashAlgorithm.pszObjId, szOID_NIST_sha256) != 0)
        return TRUST_E_SUBJECT_NOT_TRUSTED;

    AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_PARA extra = {};
    extra.cbSize = sizeof(extra);
    extra.pSignerInfo = signer->pMsgSignerInfo;
    CERT_CHAIN_POLICY_PARA policy = {};
    policy.cbSize = sizeof(policy);
    policy.pvExtraPolicyPara = &extra;
    CERT_CHAIN_POLICY_STATUS status = {};
    status.cbSize = sizeof(status);
    if (!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_AUTHENTICODE,
        signer->pChainContext, &policy, &status) || status.dwError != ERROR_SUCCESS)
        return TRUST_E_SUBJECT_NOT_TRUSTED;

    bool rfc3161 = false;
    const auto& attributes = signer->pMsgSignerInfo->UnauthAttrs;
    for (DWORD i = 0; i < attributes.cAttr; ++i) {
        const char* oid = attributes.rgAttr[i].pszObjId;
        if (oid && std::strcmp(oid, szOID_RFC3161_counterSign) == 0)
            rfc3161 = true;
    }
    if (!rfc3161 || !signer->rgpCounterSigner)
        return TRUST_E_TIME_STAMP;

    // WinTrust verifies the countersignature's cryptography and builds the
    // primary chain at the authenticated signing time. A timestamp is mandatory;
    // its chain must use normal Windows trust, never the publisher's private root.
    for (DWORD i = 0; i < signer->cCounterSigner; ++i) {
        const auto timestamp = signer->rgpCounterSigner[i];
        if (!timestamp || timestamp->dwError != ERROR_SUCCESS || !timestamp->pChainContext)
            continue;
        CERT_CHAIN_POLICY_PARA tsPolicy = {};
        tsPolicy.cbSize = sizeof(tsPolicy);
        CERT_CHAIN_POLICY_STATUS tsStatus = {};
        tsStatus.cbSize = sizeof(tsStatus);
        if (CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_AUTHENTICODE_TS,
            timestamp->pChainContext, &tsPolicy, &tsStatus) && tsStatus.dwError == ERROR_SUCCESS) {
            context.accepted = true;
            return S_OK;
        }
    }
    return TRUST_E_TIME_STAMP;
}
}

bool verifyUpdateSignature(const std::wstring& path, HANDLE file,
    const BYTE* pinnedDer, DWORD pinnedSize) {
    if (!pinnedDer || !pinnedSize || file == INVALID_HANDLE_VALUE || !file)
        return false;
    PCCERT_CONTEXT pin = CertCreateCertificateContext(X509_ASN_ENCODING, pinnedDer, pinnedSize);
    if (!pin)
        return false;

    // Trust the committed self-signed certificate in an isolated chain engine.
    // Nothing is added to CurrentUser/LocalMachine certificate stores, and there
    // is no broad "ignore untrusted root" or "ignore signature error" exception.
    HCERTSTORE roots = CertOpenStore(CERT_STORE_PROV_MEMORY, 0, 0, 0, nullptr);
    HCERTCHAINENGINE engine = nullptr;
    bool selfSigned = CertCompareCertificateName(X509_ASN_ENCODING,
        &pin->pCertInfo->Subject, &pin->pCertInfo->Issuer) != FALSE;
    bool ready = roots != nullptr;
    if (ready && selfSigned) {
        CERT_CHAIN_ENGINE_CONFIG config = {};
        config.cbSize = sizeof(config);
        config.hExclusiveRoot = roots;
        ready = CertAddCertificateContextToStore(roots, pin, CERT_STORE_ADD_NEW, nullptr) &&
            CertCreateCertificateChainEngine(&config, &engine);
    }

    bool valid = false;
    if (ready) {
        LPSTR codeUsage = const_cast<LPSTR>(szOID_PKIX_KP_CODE_SIGNING);
        CERT_CHAIN_PARA codeParameters = {};
        codeParameters.cbSize = sizeof(codeParameters);
        codeParameters.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
        codeParameters.RequestedUsage.Usage.cUsageIdentifier = 1;
        codeParameters.RequestedUsage.Usage.rgpszUsageIdentifier = &codeUsage;
        LPSTR timestampUsage = const_cast<LPSTR>(szOID_PKIX_KP_TIMESTAMP_SIGNING);
        CERT_CHAIN_PARA timestampParameters = {};
        timestampParameters.cbSize = sizeof(timestampParameters);
        timestampParameters.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
        timestampParameters.RequestedUsage.Usage.cUsageIdentifier = 1;
        timestampParameters.RequestedUsage.Usage.rgpszUsageIdentifier = &timestampUsage;
        WTD_GENERIC_CHAIN_POLICY_CREATE_INFO primary = {};
        primary.cbSize = sizeof(primary);
        primary.hChainEngine = engine;
        primary.pChainPara = &codeParameters;
        primary.dwFlags = CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL;
        WTD_GENERIC_CHAIN_POLICY_CREATE_INFO timestamp = {};
        timestamp.cbSize = sizeof(timestamp);
        timestamp.pChainPara = &timestampParameters;
        timestamp.dwFlags = CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL;
        WTD_GENERIC_CHAIN_POLICY_DATA chain = {};
        chain.cbSize = sizeof(chain);
        chain.pSignerChainInfo = &primary;
        chain.pCounterSignerChainInfo = &timestamp;
        chain.pfnPolicyCallback = pinnedPolicy;
        PolicyContext context = {pin};
        chain.pvPolicyArg = &context;
        WINTRUST_FILE_INFO info = {};
        info.cbStruct = sizeof(info);
        info.pcwszFilePath = path.c_str();
        info.hFile = file;
        WINTRUST_DATA trust = {};
        trust.cbStruct = sizeof(trust);
        trust.pPolicyCallbackData = &chain;
        trust.dwUIChoice = WTD_UI_NONE;
        trust.fdwRevocationChecks = WTD_REVOKE_NONE;
        trust.dwUnionChoice = WTD_CHOICE_FILE;
        trust.pFile = &info;
        trust.dwStateAction = WTD_STATEACTION_VERIFY;
        trust.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL | WTD_REVOCATION_CHECK_NONE;
        GUID action = WINTRUST_ACTION_GENERIC_CHAIN_VERIFY;
        valid = WinVerifyTrust(nullptr, &action, &trust) == ERROR_SUCCESS && context.accepted;
        trust.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust(nullptr, &action, &trust);
    }
    if (engine) CertFreeCertificateChainEngine(engine);
    if (roots) CertCloseStore(roots, 0);
    CertFreeCertificateContext(pin);
    return valid;
}
