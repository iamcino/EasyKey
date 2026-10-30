#!/bin/sh
# Cuts Be Vietnam Pro down to Latin and Vietnamese and packs it as WOFF2 for EasyKey.exe.
# Usage: sh tools/make_font.sh <folder with BeVietnamPro-Regular.ttf and BeVietnamPro-SemiBold.ttf>
# The fonts come from https://github.com/google/fonts/tree/main/ofl/bevietnampro (SIL OFL 1.1).
# Needs fonttools (pyftsubset) and woff2 (woff2_compress).
set -e
SRC="$1"
OUT="$(dirname "$0")/../Sources/EasyKey/win32/EasyKey/EasyKey/fonts"
# Latin, Vietnamese (also the combining marks of "Unicode tổ hợp"), and the punctuation EasyKey shows
UNICODES="U+0020-007E,U+00A0-00FF,U+0102-0103,U+0110-0111,U+0128-0129,U+0168-0169,U+01A0-01A1,U+01AF-01B0,U+0300-0301,U+0303,U+0309,U+0323,U+1EA0-1EF9,U+2013-2014,U+2018-201A,U+201C-201E,U+2022,U+2026,U+2039-203A,U+20AB,U+20AC,U+2212"
TMP="$(mktemp -d)"
for weight in Regular SemiBold; do
	# hinting is kept: it keeps stems sharp at 100% scale; all names are kept, they carry the copyright and the OFL
	pyftsubset "$SRC/BeVietnamPro-$weight.ttf" --unicodes="$UNICODES" \
		--layout-features='kern,liga,calt,ccmp,mark,mkmk,locl' --name-IDs='*' --output-file="$TMP/BeVietnamPro-$weight.ttf"
	woff2_compress "$TMP/BeVietnamPro-$weight.ttf"
	cp "$TMP/BeVietnamPro-$weight.woff2" "$OUT/"
done
rm -rf "$TMP"
