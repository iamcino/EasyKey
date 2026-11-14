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
#include <winhttp.h>
#include <bcrypt.h>
#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>
#include "Updater.h"
#include "HttpBody.h"
#include "UpdateCleanup.h"
#include "UpdateSignature.h"
#include "../../../app-core/VersionInfo.h"
#include "AppDelegate.h"
#include "EasyKeyManager.h"
#include "MessageWindow.h"

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "bcrypt.lib")

//every release is published on GitHub and on GitLab with the same signed files. Every check
//tries GitHub first, then GitLab; the download and the release-notes link start from the
//host that answered the check, so an outage on one does not stop updates; the SHA-256 and
//the pinned signature check make the host itself untrusted
struct ReleaseHost {
	const wchar_t* versionUrl;
	const wchar_t* downloadPrefix; //+ version + downloadMiddle + file name
	const wchar_t* downloadMiddle;
	const wchar_t* notesPrefix;    //+ version
	const wchar_t* page;
};
static const ReleaseHost hosts[] = {
	{ L"https://raw.githubusercontent.com/iamcino/EasyKey/master/version.json",
	  L"https://github.com/iamcino/EasyKey/releases/download/", L"/",
	  L"https://github.com/iamcino/EasyKey/releases/tag/",
	  L"https://github.com/iamcino/EasyKey/releases/latest" },
	{ L"https://gitlab.com/cino-group/easykey/-/raw/master/version.json",
	  L"https://gitlab.com/cino-group/easykey/-/releases/", L"/downloads/",
	  L"https://gitlab.com/cino-group/easykey/-/releases/",
	  L"https://gitlab.com/cino-group/easykey/-/releases" },
};
static const LONG hostCount = (LONG)(sizeof(hosts) / sizeof(hosts[0]));
//written by the worker thread, read by the main thread
static volatile LONG workingHost = 0;

static const ReleaseHost& hostByTry(LONG attempt) {
	return hosts[(InterlockedCompareExchange(&workingHost, 0, 0) + attempt) % hostCount];
}
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
	//bound every phase: the defaults leave name resolution unbounded and a dead
	//host would hold the one update worker (and the fallback to the other host)
	if (session && !WinHttpSetTimeouts(session, 15000, 20000, 30000, 30000)) {
		WinHttpCloseHandle(session);
		session = NULL;
	}
	HINTERNET connection = session ? WinHttpConnect(session, host, parts.nPort, 0) : NULL;
	HINTERNET request = connection ? WinHttpOpenRequest(connection, L"GET", path, NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE) : NULL;
	if (request && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
		WinHttpReceiveResponse(request, NULL)) {
		DWORD status = 0, size = sizeof(status);
		WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
			&status, &size, WINHTTP_NO_HEADER_INDEX);
		if (status == 200) {
			DWORD contentLength = 0, contentLengthSize = sizeof(contentLength);
			bool haveLength = WinHttpQueryHeaders(request, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
				&contentLength, &contentLengthSize, WINHTTP_NO_HEADER_INDEX) != FALSE;
			ok = readHttpBody(request, &WinHttpReadData, MAX_DOWNLOAD, haveLength, contentLength, body);
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

//version names, their packed numbers and the version.json parse live in
//app-core/VersionInfo.cpp, where the test suite can reach them
bool Updater::check(string& newVersion, bool& failed) {
	string data;
	failed = true;
	bool found = false;
	bool read = false;
	for (LONG index = 0; index < hostCount && !found; index++) {
		string candidate;
		if (httpGet(hosts[index].versionUrl, data) && parseLatestVersionName(data, candidate)) {
			read = true;
			//a host below the rollback floor does not end the search: the other host may be current
			if (meetsVersionFloor(candidate)) {
				newVersion = candidate;
				InterlockedExchange(&workingHost, index);
				found = true;
			}
		}
	}
	if (!found) {
		failed = !read;
		return false;
	}
	failed = false;

	DWORD current = EasyKeyHelper::getVersionNumber(); //major | minor << 8 | patch << 16
	DWORD currentNumber = ((current & 0xFF) << 16) | (current & 0xFF00) | ((current >> 16) & 0xFF);
	return versionNumber(newVersion) > currentNumber;
}

wstring Updater::releaseNotesUrl(const wstring& version) {
	return hostByTry(0).notesPrefix + version;
}

static wstring folderOf(const wstring& path) {
	return path.substr(0, path.find_last_of(L'\\') + 1);
}

static void showError(const HWND& owner, LPCTSTR message) {
	if (MessageWindow::ask(owner, TR("Chưa cập nhật được", "The update did not finish"), message,
			TR("Mở trang tải về", "Open download page"), TR("Đóng", "Close")))
		ShellExecute(NULL, L"open", hostByTry(0).page, NULL, NULL, SW_SHOWNORMAL);
}

//checking, downloading and swapping the exe run on a worker thread: the keyboard and mouse
//hooks live on the main thread, and Windows holds every key press and mouse move while it is busy
#define WM_UPDATER_DONE (WM_APP + 1)

enum { JOB_CHECK, JOB_DOWNLOAD };
enum { DOWNLOAD_OK, DOWNLOAD_FAILED, DOWNLOAD_MISMATCH, SIGNATURE_INVALID, INSTALL_FAILED };

struct Job {
	int kind;
	HWND owner;
	bool silent;
	bool quiet = false; //only note the new version, see checkQuietly
	bool newer = false;
	bool failed = false; //version.json could not be read
	string version, exe;
	wstring path; //where the new exe went
	int result = DOWNLOAD_OK;
};

static HWND messageWindow = NULL;
static bool busy = false;
static string available;

//the downloaded exe takes the place of the running one; antivirus scans a new exe as it is
//written and moved, which can take a while. Before that, its Authenticode signature must
//come from the EasyKey certificate compiled into this exe (docs/signing/EasyKey-codesign.cer):
//the .sha256 file sits next to the exe on GitHub, so it only catches a broken download
static int install(const string& exe, wstring& path) {
	path = EasyKeyHelper::getFullPath();
	wstring folder = folderOf(path);
	//a random name created fresh, never an existing file or link someone placed there
	BYTE random[16];
	if (!BCRYPT_SUCCESS(BCryptGenRandom(NULL, random, sizeof(random), BCRYPT_USE_SYSTEM_PREFERRED_RNG)))
		return INSTALL_FAILED;
	static const wchar_t digits[] = L"0123456789abcdef";
	wstring suffix;
	for (BYTE byte : random) {
		suffix += digits[byte >> 4];
		suffix += digits[byte & 15];
	}
	wstring newPath = folder + L"EasyKey.new." + suffix + L".exe", oldPath = folder + L"EasyKey.old.exe";
	//the handle stays open, denying other writers and deleters, from writing through
	//verification to the final rename, so the file checked is the file installed
	HANDLE file = CreateFile(newPath.c_str(), GENERIC_READ | GENERIC_WRITE | DELETE,
		FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return INSTALL_FAILED;
	DWORD written = 0;
	bool saved = WriteFile(file, exe.data(), (DWORD)exe.size(), &written, NULL) && written == exe.size() && FlushFileBuffers(file);
	LARGE_INTEGER beginning = {};
	saved = saved && SetFilePointerEx(file, beginning, NULL, FILE_BEGIN);
	HRSRC resource = FindResource(NULL, MAKEINTRESOURCE(IDR_UPDATER_CERT), RT_RCDATA);
	HGLOBAL loaded = resource ? LoadResource(NULL, resource) : NULL;
	const BYTE* pin = loaded ? static_cast<const BYTE*>(LockResource(loaded)) : NULL;
	DWORD pinSize = resource ? SizeofResource(NULL, resource) : 0;
	if (!saved || !verifyUpdateSignature(newPath, file, pin, pinSize)) {
		CloseHandle(file);
		DeleteFile(newPath.c_str());
		return saved ? SIGNATURE_INVALID : INSTALL_FAILED;
	}
	//Windows lets a running exe be renamed but not overwritten, so the old one steps aside first
	DeleteFile(oldPath.c_str());
	bool swapped = MoveFileEx(path.c_str(), oldPath.c_str(), MOVEFILE_REPLACE_EXISTING) != FALSE;
	const DWORD nameBytes = static_cast<DWORD>(path.size() * sizeof(wchar_t));
	std::vector<BYTE> renameBuffer(offsetof(FILE_RENAME_INFO, FileName) + nameBytes + sizeof(wchar_t));
	FILE_RENAME_INFO* rename = reinterpret_cast<FILE_RENAME_INFO*>(renameBuffer.data());
	rename->ReplaceIfExists = FALSE;
	rename->RootDirectory = NULL;
	rename->FileNameLength = nameBytes;
	std::memcpy(rename->FileName, path.data(), nameBytes);
	if (swapped && !SetFileInformationByHandle(file, FileRenameInfo, rename, static_cast<DWORD>(renameBuffer.size()))) {
		MoveFileEx(oldPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING);
		swapped = false;
	}
	CloseHandle(file);
	if (!swapped)
		DeleteFile(newPath.c_str());
	return swapped ? DOWNLOAD_OK : INSTALL_FAILED;
}

static void download(Job* job) {
	//defense in depth: check() applies the same floor before a version can reach this job
	if (!meetsVersionFloor(job->version)) {
		job->result = DOWNLOAD_FAILED;
		return;
	}
	wstring wideVersion = utf8ToWideString(job->version);
	string checksum;
	LONG first = InterlockedCompareExchange(&workingHost, 0, 0);
	bool fetched = false;
	for (LONG attempt = 0; attempt < hostCount && !fetched; attempt++) {
		LONG index = (first + attempt) % hostCount;
		wstring base = hosts[index].downloadPrefix + wideVersion + hosts[index].downloadMiddle;
		if (httpGet(base + L"EasyKey.exe", job->exe) && httpGet(base + L"EasyKey.exe.sha256", checksum)) {
			InterlockedExchange(&workingHost, index);
			fetched = true;
		}
	}
	if (!fetched) {
		job->result = DOWNLOAD_FAILED;
		return;
	}
	//the checksum file is "<64 hex digits>  EasyKey.exe"
	string expected = checksum.substr(0, min(checksum.size(), (size_t)64));
	for (char& c : expected)
		c = (char)tolower(c);
	const string& exe = job->exe;
	if (expected.size() != 64 || sha256Hex(exe) != expected || exe.size() < 2 || exe[0] != 'M' || exe[1] != 'Z')
		job->result = DOWNLOAD_MISMATCH;
	else
		job->result = install(exe, job->path);
}

static DWORD WINAPI runJob(LPVOID param) {
	Job* job = (Job*)param;
	if (job->kind == JOB_CHECK)
		job->newer = Updater::check(job->version, job->failed);
	else
		download(job);
	PostMessage(messageWindow, WM_UPDATER_DONE, 0, (LPARAM)job);
	return 0;
}

static void startJob(Job* job) {
	busy = true;
	HANDLE thread = CreateThread(NULL, 0, runJob, job, 0, NULL);
	if (thread) {
		CloseHandle(thread);
	} else {
		busy = false;
		delete job;
	}
}

void Updater::restartAs(const wstring& path) {
	//the hooks go first: starting a new exe makes antivirus scan it while this thread waits,
	//and input would wait with it. CreateProcess, unlike ShellExecute, does not run the shell
	EasyKeyManager::freeEngine();
	wstring commandLine = L"\"" + path + L"\" " UPDATED_ARGUMENT;
	STARTUPINFO startup = { sizeof(startup) };
	PROCESS_INFORMATION process = {};
	if (CreateProcess(path.c_str(), &commandLine[0], NULL, NULL, FALSE, 0, NULL, folderOf(path).c_str(), &startup, &process)) {
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
	}
	AppDelegate::getInstance()->onEasyKeyExit();
}

static void finishJob(Job* job) {
	//the window that asked may have closed meanwhile
	HWND owner = job->owner && IsWindow(job->owner) ? job->owner : NULL;
	if (job->kind == JOB_CHECK) {
		if (job->newer && job->version != available) {
			available = job->version;
			AppDelegate::getInstance()->onUpdateAvailable();
		}
		if (job->quiet) {
			busy = false;
			delete job;
			return;
		}
		if (!job->newer) {
			busy = false;
			if (!job->silent && job->failed)
				AppDelegate::getInstance()->showNotice(TR("Chưa kiểm tra được bản mới", "Could not check for updates"),
					TR("Kiểm tra mạng, hoặc cho phép EasyKey trong phần mềm diệt virus", "Check the network, or allow EasyKey in your antivirus"));
			else if (!job->silent)
				AppDelegate::getInstance()->showNotice(TR("EasyKey đã là bản mới nhất", "EasyKey is up to date"),
					TR("Phiên bản ", "Version ") + EasyKeyHelper::getVersionString());
			delete job;
			return;
		}
		wstring version = utf8ToWideString(job->version);
		MessageWindow::Options options;
		options.heading = TR("Đã có EasyKey ", "EasyKey ") + version + TR("", " is available");
		options.text = TR("Cập nhật ngay? EasyKey sẽ tự tải về, thay file và khởi động lại.",
			"Update now? EasyKey downloads it, replaces itself and restarts.");
		options.link = TR("Xem có gì mới", "See what's new");
		options.linkUrl = Updater::releaseNotesUrl(version);
		options.buttons.push_back(TR("Cập nhật", "Update"));
		options.buttons.push_back(TR("Để sau", "Later"));
		if (MessageWindow::show(owner, options) == 0) {
			job->kind = JOB_DOWNLOAD;
			job->owner = owner;
			startJob(job);
		} else {
			busy = false;
			delete job;
		}
		return;
	}

	busy = false;
	if (job->result == DOWNLOAD_FAILED)
		showError(owner, TR("Không tải được bản mới. Mở trang tải về để tải thủ công?",
			"Could not download the update. Open the download page to get it yourself?"));
	else if (job->result == DOWNLOAD_MISMATCH)
		showError(owner, TR("File tải về không khớp mã kiểm tra nên chưa được cài. Mở trang tải về?",
			"The download did not match its checksum, so it was not installed. Open the download page?"));
	else if (job->result == SIGNATURE_INVALID)
		showError(owner, TR("Không xác minh được chữ ký EasyKey hoặc dấu thời gian của bản cập nhật nên chưa được cài. Mở trang tải về?",
			"The update's EasyKey signature or timestamp could not be verified, so it was not installed. Open the download page?"));
	else if (job->result == INSTALL_FAILED)
		showError(owner, TR("Không ghi được vào thư mục của EasyKey, có thể cần quyền Admin. Mở trang tải về để tải thủ công?",
			"Could not write to the EasyKey folder, it may need admin rights. Open the download page to get it yourself?"));
	else
		Updater::restartAs(job->path);
	delete job;
}

static LRESULT CALLBACK messageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_UPDATER_DONE) {
		finishJob((Job*)lParam);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static bool createMessageWindow() {
	if (!messageWindow) {
		WNDCLASS wc = {};
		wc.lpfnWndProc = messageProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpszClassName = _T("EasyKeyUpdater");
		RegisterClass(&wc);
		messageWindow = CreateWindow(wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, wc.hInstance, NULL);
	}
	return messageWindow != NULL;
}

void Updater::checkAndOffer(const HWND& owner, const bool& silent) {
	if (busy || !createMessageWindow())
		return;
	Job* job = new Job();
	job->kind = JOB_CHECK;
	job->owner = owner;
	job->silent = silent;
	if (!silent) //the answer can take a few seconds
		AppDelegate::getInstance()->showNotice(TR("Đang kiểm tra bản mới…", "Checking for updates…"),
			TR("Phiên bản hiện tại ", "Current version ") + EasyKeyHelper::getVersionString());
	startJob(job);
}

void Updater::checkQuietly() {
	if (busy || !createMessageWindow())
		return;
	Job* job = new Job();
	job->kind = JOB_CHECK;
	job->owner = NULL;
	job->silent = true;
	job->quiet = true;
	startJob(job);
}

const string& Updater::availableVersion() {
	return available;
}

void Updater::finishPreviousUpdate() {
	if (wcsstr(GetCommandLineW(), UPDATED_ARGUMENT)) {
		//the old EasyKey exits right after starting this one. Wait for the process,
		//not just its window: the image can stay locked after the window is gone.
		//Hooks are not installed in this process yet.
		HWND previous = FindWindow(APP_CLASS, NULL);
		DWORD pid = 0;
		if (previous)
			GetWindowThreadProcessId(previous, &pid);
		HANDLE process = pid ? OpenProcess(SYNCHRONIZE, FALSE, pid) : NULL;
		if (process) {
			WaitForSingleObject(process, 30000);
			CloseHandle(process);
		} else {
			for (int i = 0; i < 50 && FindWindow(APP_CLASS, NULL); i++)
				Sleep(100);
		}
	}
}

static DWORD WINAPI cleanupJob(LPVOID param) {
	std::unique_ptr<wstring> path(static_cast<wstring*>(param));
	cleanupOldUpdateFile(*path);
	return 0;
}

void Updater::cleanupPreviousUpdate() {
	//on a worker so a locked backup (antivirus scan) never stalls startup; a file
	//still locked after the retries is kept and tried again on the next start
	wstring* path = new wstring(folderOf(EasyKeyHelper::getFullPath()) + L"EasyKey.old.exe");
	HANDLE thread = CreateThread(NULL, 0, cleanupJob, path, 0, NULL);
	if (thread)
		CloseHandle(thread);
	else
		delete path;
}
