/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#include "HttpBody.h"

bool readHttpBody(void* request, HttpReadDataFn readData, size_t maxBytes, bool haveLength, DWORD contentLength, std::string& body) {
	body.clear();
	char buffer[16384];
	for (;;) {
		DWORD read = 0;
		if (!readData(request, buffer, (DWORD)sizeof(buffer), &read)) {
			body.clear();
			return false;
		}
		if (read == 0)
			break;
		if (read > (DWORD)sizeof(buffer)) {
			body.clear();
			return false;
		}
		body.append(buffer, read);
		if (body.size() > maxBytes) {
			body.clear();
			return false;
		}
	}
	if (haveLength && body.size() != (size_t)contentLength) {
		body.clear();
		return false;
	}
	return true;
}
