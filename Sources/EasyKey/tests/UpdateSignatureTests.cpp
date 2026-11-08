/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#include <windows.h>
#include <wincrypt.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "../win32/EasyKey/EasyKey/UpdateSignature.h"

static int failures = 0, total = 0;
static void check(bool ok, const char* label) {
    ++total;
    if (!ok) {
        ++failures;
        std::printf("FAIL [%s]\n", label);
    }
}

// A locally installed, Microsoft-signed SDK executable is the positive
// fixture. Tests only: production pins the committed EasyKey DER resource.
static std::vector<BYTE> signerCertificate(const std::wstring& path) {
    HCERTSTORE store = nullptr;
    HCRYPTMSG message = nullptr;
    std::vector<BYTE> der;
    if (!CryptQueryObject(CERT_QUERY_OBJECT_FILE, path.c_str(),
        CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED, CERT_QUERY_FORMAT_FLAG_BINARY,
        0, nullptr, nullptr, nullptr, &store, &message, nullptr))
        return der;
    DWORD size = 0;
    if (CryptMsgGetParam(message, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &size)) {
        std::vector<BYTE> buffer(size);
        if (CryptMsgGetParam(message, CMSG_SIGNER_INFO_PARAM, 0, buffer.data(), &size)) {
            const auto signer = reinterpret_cast<PCMSG_SIGNER_INFO>(buffer.data());
            CERT_INFO info = {};
            info.Issuer = signer->Issuer;
            info.SerialNumber = signer->SerialNumber;
            PCCERT_CONTEXT cert = CertFindCertificateInStore(store, X509_ASN_ENCODING |
                PKCS_7_ASN_ENCODING, 0, CERT_FIND_SUBJECT_CERT, &info, nullptr);
            if (cert) {
                der.assign(cert->pbCertEncoded, cert->pbCertEncoded + cert->cbCertEncoded);
                CertFreeCertificateContext(cert);
            }
        }
    }
    CryptMsgClose(message);
    CertCloseStore(store, 0);
    return der;
}

static bool verify(const std::wstring& path, const std::vector<BYTE>& pin) {
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    bool ok = verifyUpdateSignature(path, file, pin.data(), static_cast<DWORD>(pin.size()));
    if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    return ok;
}

int wmain(int argc, wchar_t** argv) {
    if (argc != 2 && argc != 3) {
        std::printf("Usage: UpdateSignatureTests <timestamped signed SDK executable> [EasyKey.exe signed by the release]\n");
        return 1;
    }
    const std::wstring fixture = argv[1];
    const auto pin = signerCertificate(fixture);
    check(!pin.empty(), "signed fixture certificate available");
    if (pin.empty()) return 1;
    check(verify(fixture, pin), "valid timestamped SHA-256 signature and matching pin");
    check(!verify(fixture, {}), "missing pin rejected");
    check(!verify(fixture, {1, 2, 3}), "invalid DER pin rejected");
    check(!verify(L"missing-updater-signature-fixture.exe", pin), "missing file rejected");
    wchar_t self[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, self, MAX_PATH);
    check(!verify(self, pin), "unsigned executable rejected");

    // The committed publisher certificate must reject a valid Microsoft signer.
    HANDLE publicCert = CreateFileW(EASYKEY_PUBLIC_CERT, GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    check(publicCert != INVALID_HANDLE_VALUE, "EasyKey public pin available");
    if (publicCert != INVALID_HANDLE_VALUE) {
        std::vector<BYTE> easyKeyPin(GetFileSize(publicCert, nullptr));
        DWORD read = 0;
        bool loaded = ReadFile(publicCert, easyKeyPin.data(), static_cast<DWORD>(easyKeyPin.size()), &read, nullptr) != FALSE;
        CloseHandle(publicCert);
        check(loaded && read == easyKeyPin.size() && !verify(fixture, easyKeyPin),
            "valid signature from wrong publisher rejected");
        // Acceptance: the exe the release job just signed passes the exact check the
        // updater runs, on a runner that has never trusted the EasyKey certificate.
        if (argc == 3) {
            check(loaded && verify(argv[2], easyKeyPin), "release-signed EasyKey.exe accepted with the EasyKey pin");
            check(!verify(argv[2], pin), "release-signed EasyKey.exe rejected with another pin");
        }
    }

    wchar_t temp[MAX_PATH] = {}, copy[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, temp);
    check(GetTempFileNameW(temp, L"eks", 0, copy) != 0, "temporary fixture created");
    bool copied = CopyFileW(fixture.c_str(), copy, FALSE) != FALSE;
    check(copied, "signed fixture copied");
    if (copied) {
        HANDLE held = CreateFileW(copy, GENERIC_READ | GENERIC_WRITE | DELETE,
            FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        check(held != INVALID_HANDLE_VALUE, "staged file handle opened");
        if (held != INVALID_HANDLE_VALUE) {
            check(verifyUpdateSignature(copy, held, pin.data(), static_cast<DWORD>(pin.size())),
                "signature verification works with updater's held handle");
            HANDLE writer = CreateFileW(copy, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE |
                FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            check(writer == INVALID_HANDLE_VALUE, "staging lock denies concurrent writes");
            if (writer != INVALID_HANDLE_VALUE) CloseHandle(writer);
            check(!DeleteFileW(copy), "staging lock denies deletion");

            // Rename using the held handle, exactly the OS operation required to
            // install without reopening the already verified path.
            const std::wstring renamed = std::wstring(copy) + L".exe";
            DWORD bytes = static_cast<DWORD>(renamed.size() * sizeof(wchar_t));
            std::vector<BYTE> renameBuffer(offsetof(FILE_RENAME_INFO, FileName) + bytes);
            auto info = reinterpret_cast<FILE_RENAME_INFO*>(renameBuffer.data());
            info->FileNameLength = bytes;
            std::memcpy(info->FileName, renamed.data(), bytes);
            bool moved = SetFileInformationByHandle(held, FileRenameInfo, info,
                static_cast<DWORD>(renameBuffer.size())) != FALSE;
            check(moved && verifyUpdateSignature(renamed, held, pin.data(), static_cast<DWORD>(pin.size())),
                "verified locked file can be renamed and still verifies");
            // Offset 512 is in the PE's hashed content, outside its checksum and
            // certificate directory. Flip a byte while retaining the signature.
            BYTE byte = 0;
            DWORD read = 0, written = 0;
            SetFilePointer(held, 512, nullptr, FILE_BEGIN);
            bool readOk = ReadFile(held, &byte, 1, &read, nullptr) && read == 1;
            byte ^= 1;
            SetFilePointer(held, 512, nullptr, FILE_BEGIN);
            bool changed = readOk && WriteFile(held, &byte, 1, &written, nullptr) && written == 1;
            FlushFileBuffers(held);
            SetFilePointer(held, 0, nullptr, FILE_BEGIN);
            check(changed && !verifyUpdateSignature(moved ? renamed : copy, held, pin.data(), static_cast<DWORD>(pin.size())),
                "tampered executable rejected even with matching signer certificate");
            CloseHandle(held);
            if (moved) DeleteFileW(renamed.c_str());
        }
    }

    // Changing an unsigned attribute's OID leaves the primary code signature
    // intact but removes the recognized RFC 3161 timestamp. Search only inside
    // the PE certificate table, which Authenticode excludes from the file hash.
    if (CopyFileW(fixture.c_str(), copy, FALSE)) {
        HANDLE file = CreateFileW(copy, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        bool changedTimestamp = false;
        if (file != INVALID_HANDLE_VALUE) {
            DWORD size = GetFileSize(file, nullptr), read = 0;
            std::vector<BYTE> data(size);
            if (ReadFile(file, data.data(), size, &read, nullptr) && read == size &&
                size > sizeof(IMAGE_DOS_HEADER)) {
                IMAGE_DOS_HEADER dos = {};
                std::memcpy(&dos, data.data(), sizeof(dos));
                if (dos.e_lfanew > 0 && static_cast<size_t>(dos.e_lfanew) + sizeof(IMAGE_NT_HEADERS64) <= data.size()) {
                    IMAGE_NT_HEADERS64 nt = {};
                    std::memcpy(&nt, data.data() + dos.e_lfanew, sizeof(nt));
                    const auto table = nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY];
                    const BYTE oid[] = {0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x03, 0x03, 0x01};
                    const size_t end = static_cast<size_t>(table.VirtualAddress) + table.Size;
                    if (end <= data.size()) {
                        for (size_t pos = table.VirtualAddress; pos + sizeof(oid) <= end; ++pos) {
                            if (std::memcmp(data.data() + pos, oid, sizeof(oid)) == 0) {
                                data[pos + sizeof(oid) - 1] = 0x7f;
                                changedTimestamp = true;
                            }
                        }
                    }
                }
                if (changedTimestamp) {
                    DWORD written = 0;
                    SetFilePointer(file, 0, nullptr, FILE_BEGIN);
                    changedTimestamp = WriteFile(file, data.data(), size, &written, nullptr) && written == size;
                }
            }
            CloseHandle(file);
        }
        check(changedTimestamp && !verify(copy, pin), "missing recognized RFC 3161 timestamp rejected");
    } else {
        check(false, "timestamp fixture copied");
    }
    DeleteFileW(copy);
    std::printf("%d/%d updater signature tests passed\n", total - failures, total);
    return failures ? 1 : 0;
}
