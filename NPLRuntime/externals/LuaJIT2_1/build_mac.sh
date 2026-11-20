#!/usr/bin/env bash
set -euo pipefail

# Rebuild the macOS SDK from an exact upstream revision. An optional local
# LuaJIT Git checkout also supports offline builds and may be on any branch.
revision=63c8f1a9b479ba5518cf6a170ff7729178bc3cfd
dest="$(cd "$(dirname "$0")" && pwd)/prebuilt/mac"
work=$(mktemp -d "${TMPDIR:-/tmp}/npl-luajit.XXXXXX")
trap 'rm -rf "$work"' EXIT

if [[ $# -gt 0 ]]; then
    repo=$1
else
    repo="$work/repo"
    git init -q "$repo"
    git -C "$repo" fetch --depth=1 https://github.com/LuaJIT/LuaJIT.git "$revision"
fi
# git archive expands .relver to the pinned commit timestamp.
git -C "$repo" archive "$revision" > "$work/source.tar"
for arch in arm64 x86_64; do
    src="$work/$arch"
    mkdir "$src"
    tar -xf "$work/source.tar" -C "$src"
    deployment=11.0
    if [[ $arch == x86_64 ]]; then deployment=10.13; fi
    MACOSX_DEPLOYMENT_TARGET=$deployment make -C "$src/src" -j"$(sysctl -n hw.ncpu)" \
        BUILDMODE=static HOST_CC=clang CC="clang -arch $arch" libluajit.a luajit.h
    lipo "$src/src/libluajit.a" -verify_arch "$arch"
done
lipo -create "$work/arm64/src/libluajit.a" "$work/x86_64/src/libluajit.a" -output "$work/libluajit.a"
mkdir -p "$dest/include"
for arch in arm64 x86_64; do
    cp "$work/$arch/src/libluajit.a" "$dest/libluajit_$arch.a"
done
cp "$work/arm64/src/"{lua.h,luaconf.h,lauxlib.h,lualib.h,luajit.h,lua.hpp} "$dest/include/"
cp "$work/arm64/COPYRIGHT" "$dest/COPYRIGHT"
cp "$work/libluajit.a" "$dest/libluajit.a"
