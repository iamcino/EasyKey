/*----------------------------------------------------------
EasyKey - Vietnamese input for Windows 11.
Copyright (C) 2026 Cino

SPDX-License-Identifier: GPL-3.0-only OR MIT

This file was written for EasyKey and holds no OpenKey code.
Inside EasyKey it is used under the GNU GPL version 3, like
the rest of the program. On its own you may also use it under
the MIT license, see LICENSES/MIT.txt.
-----------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <string>

/*
 * Version names and the version.json of the update check, kept free of
 * Windows code so the test suite can cover them (refactor plan step 3.1;
 * this logic used to live as static functions inside Updater.cpp).
 *
 * A version name is exactly three numeric components: "1.0.21".
 * The packed number is major << 16 | minor << 8 | patch, so minor and
 * patch must fit in a byte (0-255) and major in the remaining 16 bits;
 * anything outside that is not a valid version name. This is stricter
 * than the code it replaces, which accepted "1.0" and "1.0.21.4" and
 * silently wrapped minor/patch values above 255.
 */
bool isVersionName(const std::string& version);

//packed number of a valid version name, so versions compare as numbers; 0 when the name is not valid
uint32_t versionNumber(const std::string& version);

/*
 * The oldest release this build will accept as an update. The floor is part
 * of the binary, not data from an update host: a compromised or mistaken
 * host can offer an old signed release, but it cannot lower this value.
 * Raise it only, and only to a release the project is willing to make the
 * permanent minimum.
 */
const char* const MIN_ACCEPTED_VERSION = "1.1.4";

//true when version is a valid version name at or above MIN_ACCEPTED_VERSION
bool meetsVersionFloor(const std::string& version);

/*
 * Read the version of "latestWinVersion" out of the text of version.json.
 * The search is anchored at "latestWinVersion", like Updater::check has
 * always done: a "versionName" belonging to any other block is not it.
 * Returns false, leaving version untouched, when there is no valid one.
 */
bool parseLatestVersionName(const std::string& data, std::string& version);
