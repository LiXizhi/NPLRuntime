#!/usr/bin/env bash
set -euo pipefail

dir=$(cd "$(dirname "$0")" && pwd)
bash "$dir/build_mac.sh" "$@"
mkdir -p "$dir/prebuilt/mac_m1"
cp "$dir/prebuilt/mac/libluajit_arm64.a" "$dir/prebuilt/mac_m1/libluajit.a"
