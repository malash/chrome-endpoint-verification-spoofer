#!/usr/bin/env bash
# Fetch the latest public macOS version, print it, and pin it into spoof.conf.
set -euo pipefail

conf="$(cd "$(dirname "$0")/.." && pwd)/spoof.conf"

macos="$(curl -fsSL https://gdmf.apple.com/v2/pmv \
    | plutil -extract PublicAssetSets.macOS json -o - -)"

ver="$(printf '%s' "$macos" | grep -oE '"ProductVersion":"[0-9.]+"' \
    | sed -E 's/.*:"([^"]+)"/\1/' | sort -t. -k1,1n -k2,2n -k3,3n | tail -1)"

build="$(printf '%s' "$macos" | grep -oE '\{[^}]*\}' \
    | grep -F "\"ProductVersion\":\"$ver\"" \
    | grep -oE '"Build":"[^"]+"' | head -1 | sed -E 's/.*:"([^"]+)"/\1/')"

echo "$ver ($build)"

sed -i '' -E "s/^os_version=.*/os_version=$ver/" "$conf"
sed -i '' -E "s|^os_version_string=.*|os_version_string=Version $ver (Build $build)|" "$conf"
