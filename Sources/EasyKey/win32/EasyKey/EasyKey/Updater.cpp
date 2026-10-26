/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11, based on OpenKey.
Copyright (C) 2026 Cino

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
-----------------------------------------------------------*/
#include <windows.h>
#include <winhttp.h>
#include <bcrypt.h>
#include "Updater.h"
#include "AppDelegate.h"

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "bcrypt.lib")

#define VERSION_URL L"https://raw.githubusercontent.com/iamcino/EasyKey/master/version.json"
#define RELEASE_URL L"https://github.com/iamcino/EasyKey/releases/download/"
#define RELEASES_PAGE L"https://github.com/iamcino/EasyKey/releases/latest"
#define MAX_DOWNLOAD (32 * 1024 * 1024)
#define UPDATED_ARGUMENT L"--updated"

//GET over HTTPS only, following redirects (release files live on another host)
static bool httpGet(const wstring& url, string& body) {
	URL_COMPONENTS parts = { sizeof(parts) };
	wchar_t host[256], path[2048];
	parts.lpszHostName = host;
	parts.dwHostNameLength = 256;
	parts.lpszUrlPath = path;
	parts.dwUrlPathLength = 2048;
	if (!WinHttpCrackUrl(url.c_str(), 0, 0, &parts) || parts.nScheme != INTERNET_SCHEME_HTTPS)
		return false;

	bool ok = false;
	HINTERNET session = WinHttpOpen(L"EasyKey", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	HINTERNET connection = session ? WinHttpConnect(session, host, parts.nPort, 0) : NULL;
	HINTERNET request = connection ? WinHttpOpenRequest(connection, L"GET", path, NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE) : NULL;
	if (request && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
		WinHttpReceiveResponse(request, NULL)) {
		DWORD status = 0, size = sizeof(status);
		WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
			&status, &size, WINHTTP_NO_HEADER_INDEX);
		if (status == 200) {
			body.clear();
			char buffer[16384];
			DWORD read = 0;
			ok = true;
			while (WinHttpReadData(request, buffer, sizeof(buffer), &read) && read > 0) {
				body.append(buffer, read);
				if (body.size() > MAX_DOWNLOAD) {
					ok = false;
					break;
				}
			}
		}
	}
	if (request) WinHttpCloseHandle(request);
	if (connection) WinHttpCloseHandle(connection);
	if (session) WinHttpCloseHandle(session);
	return ok;
}

static string sha256Hex(const string& data) {
	string hex;
	BCRYPT_ALG_HANDLE algorithm = NULL;
	BCRYPT_HASH_HANDLE hash = NULL;
	UCHAR digest[32];
	if (BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, NULL, 0))) {
		if (BCRYPT_SUCCESS(BCryptCreateHash(algorithm, &hash, NULL, 0, NULL, 0, 0)) &&
			BCRYPT_SUCCESS(BCryptHashData(hash, (PUCHAR)data.data(), (ULONG)data.size(), 0)) &&
			BCRYPT_SUCCESS(BCryptFinishHash(hash, digest, sizeof(digest), 0))) {
			static const char digits[] = "0123456789abcdef";
			for (UCHAR byte : digest) {
				hex += digits[byte >> 4];
				hex += digits[byte & 15];
			}
		}
		if (hash) BCryptDestroyHash(hash);
		BCryptCloseAlgorithmProvider(algorithm, 0);
	}
	return hex;
}

//"1.2.3" as 0x010203, so versions compare as numbers
static DWORD versionNumber(const string& version) {
	int major = 0, minor = 0, patch = 0;
	sscanf_s(version.c_str(), "%d.%d.%d", &major, &minor, &patch);
	return ((DWORD)major << 16) | ((DWORD)(minor & 0xFF) << 8) | (DWORD)(patch & 0xFF);
}

static bool isVersionName(const string& version) {
	if (version.empty() || version.size() > 16)
		return false;
	for (char c : version) {
		if (!(c == '.' || (c >= '0' && c <= '9')))
			return false;
	}
	return true;
}

bool Updater::check(string& newVersion) {
	string data;
	if (!httpGet(VERSION_URL, data))
		return false;
	//"latestWinVersion": { "versionName": "1.0.1", ... }
	size_t pos = data.find("\"latestWinVersion\"");
	pos = pos == string::npos ? pos : data.find("\"versionName\"", pos);
	pos = pos == string::npos ? pos : data.find(':', pos);
	pos = pos == string::npos ? pos : data.find('"', pos);
	size_t end = pos == string::npos ? pos : data.find('"', pos + 1);
	if (end == string::npos)
		return false;
	newVersion = data.substr(pos + 1, end - pos - 1);
	if (!isVersionName(newVersion))
		return false;

	DWORD current = EasyKeyHelper::getVersionNumber(); //major | minor << 8 | patch << 16
	DWORD currentNumber = ((current & 0xFF) << 16) | (current & 0xFF00) | ((current >> 16) & 0xFF);
	return versionNumber(newVersion) > currentNumber;
}

static wstring folderOf(const wstring& path) {
	return path.substr(0, path.find_last_of(L'\\') + 1);
}

static void showError(const HWND& owner, LPCTSTR message) {
	if (MessageBox(owner, message, _T("EasyKey"), MB_ICONWARNING | MB_YESNO) == IDYES)
		ShellExecute(NULL, L"open", RELEASES_PAGE, NULL, NULL, SW_SHOWNORMAL);
}

static bool install(const HWND& owner, const string& version) {
	wstring wideVersion = utf8ToWideString(version);
	string exe, checksum;
	if (!httpGet(RELEASE_URL + wideVersion + L"/EasyKey.exe", exe) ||
		!httpGet(RELEASE_URL + wideVersion + L"/EasyKey.exe.sha256", checksum)) {
		showError(owner, TR("Không tải được bản mới. Mở trang tải về để tải thủ công?",
			"Could not download the update. Open the download page to get it yourself?"));
		return false;
	}
	//the checksum file is "<64 hex digits>  EasyKey.exe"
	string expected = checksum.substr(0, min(checksum.size(), (size_t)64));
	for (char& c : expected)
		c = (char)tolower(c);
	if (expected.size() != 64 || sha256Hex(exe) != expected || exe.size() < 2 || exe[0] != 'M' || exe[1] != 'Z') {
		showError(owner, TR("File tải về không khớp mã kiểm tra nên chưa được cài. Mở trang tải về?",
			"The download did not match its checksum, so it was not installed. Open the download page?"));
		return false;
	}

	wstring path = EasyKeyHelper::getFullPath();
	wstring folder = folderOf(path);
	wstring newPath = folder + L"EasyKey.new.exe", oldPath = folder + L"EasyKey.old.exe";
	HANDLE file = CreateFile(newPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD written = 0;
	bool saved = file != INVALID_HANDLE_VALUE && WriteFile(file, exe.data(), (DWORD)exe.size(), &written, NULL) && written == exe.size();
	if (file != INVALID_HANDLE_VALUE)
		CloseHandle(file);
	//Windows lets a running exe be renamed but not overwritten, so the old one steps aside first
	DeleteFile(oldPath.c_str());
	bool swapped = saved && MoveFileEx(path.c_str(), oldPath.c_str(), MOVEFILE_REPLACE_EXISTING);
	if (swapped && !MoveFileEx(newPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
		MoveFileEx(oldPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING);
		swapped = false;
	}
	if (!swapped) {
		DeleteFile(newPath.c_str());
		showError(owner, TR("Không ghi được vào thư mục của EasyKey, có thể cần quyền Admin. Mở trang tải về để tải thủ công?",
			"Could not write to the EasyKey folder, it may need admin rights. Open the download page to get it yourself?"));
		return false;
	}

	ShellExecute(NULL, NULL, path.c_str(), UPDATED_ARGUMENT, folder.c_str(), SW_SHOWNORMAL);
	AppDelegate::getInstance()->onEasyKeyExit();
	return true;
}

void Updater::checkAndOffer(const HWND& owner, const bool& silent) {
	string version;
	if (!check(version)) {
		if (!silent)
			MessageBox(owner, TR("Bạn đang dùng phiên bản mới nhất.", "You have the latest version."), _T("EasyKey"), MB_OK);
		return;
	}
	TCHAR message[256];
	wsprintf(message, TR("Đã có EasyKey %s. Cập nhật ngay? EasyKey sẽ tự khởi động lại.",
		"EasyKey %s is available. Update now? EasyKey will restart."), utf8ToWideString(version).c_str());
	if (MessageBox(owner, message, _T("EasyKey"), MB_ICONINFORMATION | MB_YESNO) == IDYES)
		install(owner, version);
}

void Updater::finishPreviousUpdate() {
	if (wcsstr(GetCommandLineW(), UPDATED_ARGUMENT)) {
		//the old EasyKey exits right after starting this one
		for (int i = 0; i < 50 && FindWindow(APP_CLASS, NULL); i++)
			Sleep(100);
	}
	wstring oldPath = folderOf(EasyKeyHelper::getFullPath()) + L"EasyKey.old.exe";
	DeleteFile(oldPath.c_str());
}
