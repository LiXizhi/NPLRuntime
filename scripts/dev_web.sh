#!/bin/bash
# date: 2026.11
# desc: Zero-download edit/build/test loop for the Emscripten single-thread client (Linux/WSL/macOS).
#       Builds ParaCraftSingleThread.{js,wasm} into <repo>/web/auto_generate_files/ and serves
#       the self-contained <repo>/web shell mounting any local Paracraft directory directly via
#       ?searchroot=...&searchpath=... (no webparacraft clone or .data packing required).
#
# usage: scripts/dev_web.sh [cpp|build|configure|serve|all]   (default: cpp)
#        SEARCHROOT=C:/lxzsrc/ParaEngine/ParaWorld scripts/dev_web.sh serve
#        WEB_ROBOT=OFF scripts/dev_web.sh cpp                 (optional faster non-robotics build)
#        scripts/dev_web.sh serve --https                     (HTTPS for WebXR / LAN secure context)

set -e

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"

## On WSL (/mnt/...), default BUILD_DIR to native Linux ext4 for 10x faster compilation
if [[ "$REPO_DIR" == /mnt/* ]]; then
    DEFAULT_BUILD_DIR="$HOME/build/npl_unified_wasm"
else
    DEFAULT_BUILD_DIR="$REPO_DIR/build/emscripten_single_thread"
fi

EMSDK_DIR=${EMSDK_DIR:-$HOME/emsdk/emsdk}
BOOST_DIR=${BOOST_DIR:-$HOME/boost_1_85_0}
BUILD_DIR=${BUILD_DIR:-$DEFAULT_BUILD_DIR}
WEB_DIR=${WEB_DIR:-$REPO_DIR/web}
SEARCHROOT=${SEARCHROOT:-${PARACRAFT_SEARCHROOT:-}}
WEB_ROBOT=${WEB_ROBOT:-ON}
PORT=${PORT:-8088}
HOST=${HOST:-127.0.0.1}
JOBS=${JOBS:-$(getconf _NPROCESSORS_ONLN)}
CERT_DIR=${CERT_DIR:-$BUILD_DIR/web_cert}

TARGET=ParaCraftSingleThread
OUT_DIR="$WEB_DIR/auto_generate_files"

log() { echo "[dev_web] $*"; }
die() { echo "[dev_web] ERROR: $*" >&2; exit 1; }

find_emsdk_python() {
    if [ -n "$EMSDK_PYTHON" ]; then return; fi
    for py in python3.13 python3.12 python3.11 python3.10 /opt/homebrew/bin/python3.12 /usr/local/bin/python3.12 python3; do
        if command -v "$py" >/dev/null 2>&1 && "$py" -c 'import sys; sys.exit(sys.version_info < (3, 10))' 2>/dev/null; then
            export EMSDK_PYTHON=$(command -v "$py")
            return
        fi
    done
    die "emsdk needs python >= 3.10 (macOS: brew install python@3.12)"
}

load_emsdk() {
    [ -f "$EMSDK_DIR/emsdk_env.sh" ] || die "emsdk not found in $EMSDK_DIR (see docs/web-dev-workflow.md)"
    find_emsdk_python
    source "$EMSDK_DIR/emsdk_env.sh" >/dev/null 2>&1
    command -v emcc >/dev/null || die "emcc not on PATH after sourcing emsdk_env.sh"
    log "using $(emcc --version | head -n 1)"
}

check_web_dir() {
    [ -f "$WEB_DIR/index.html" ] || die "Web shell index.html not found in $WEB_DIR"
    mkdir -p "$OUT_DIR"
}

update_md5_if_needed() {
    if [ -f "$WEB_DIR/calculate_md5.sh" ]; then
        (cd "$WEB_DIR" && bash calculate_md5.sh >/dev/null)
        log "updated $WEB_DIR/auto_generate_files/paracraft_sitefile_md5sums.js"
    fi
}

configure() {
    check_web_dir
    load_emsdk
    [ -d "$BOOST_DIR/stage/lib" ] || die "boost not built in $BOOST_DIR/stage/lib (see docs/web-dev-workflow.md)"
    embuilder build zlib libjpeg libpng sdl2 >/dev/null
    mkdir -p "$BUILD_DIR"
    emcmake cmake -S "$REPO_DIR/NPLRuntime" -B "$BUILD_DIR" \
        -DBUILD_TESTING=OFF \
        -DNPLRUNTIME_BUILD_TESTS=OFF \
        -DEMSCRIPTEN=ON \
        -DEMSCRIPTEN_SINGLE_THREAD=ON \
        -DNPLRUNTIME_WEB_ROBOT="$WEB_ROBOT" \
        -DNPLRUNTIME_WEB_ROBOT_BUILD_JOBS="$JOBS" \
        -DNPLRUNTIME_WEB_PACK_ASSETS=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DBOOST_ROOT="$BOOST_DIR" \
        -DHTML_ROOT="$OUT_DIR"
}

cmd_cpp() {
    check_web_dir
    load_emsdk
    [ -f "$BUILD_DIR/CMakeCache.txt" ] || configure
    log "building $TARGET (-j$JOBS) in $BUILD_DIR"
    (cd "$BUILD_DIR" && emmake make -j"$JOBS" "$TARGET")
    cp "$BUILD_DIR/bin/$TARGET.js" "$BUILD_DIR/bin/$TARGET.wasm" "$OUT_DIR/"
    update_md5_if_needed
    log "copied $TARGET.js/.wasm -> $OUT_DIR"
}

make_cert() {
    local ip=$(python3 -c 'import socket; s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.connect(("10.255.255.255", 1)); print(s.getsockname()[0])' 2>/dev/null)
    local names="localhost 127.0.0.1 ::1 $ip"
    local tag=$( (command -v mkcert >/dev/null && echo mkcert || echo selfsigned) )
    CERT=$CERT_DIR/$tag-${ip:-noip}.pem
    KEY=$CERT_DIR/$tag-${ip:-noip}-key.pem
    [ -f "$CERT" ] && [ -f "$KEY" ] && return
    mkdir -p "$CERT_DIR"
    if [ "$tag" = mkcert ]; then
        mkcert -install
        mkcert -cert-file "$CERT" -key-file "$KEY" $names
    else
        log "mkcert not found; making a self-signed certificate with openssl"
        local san=$(for n in $names; do case $n in (*[a-z]*) printf 'DNS:%s,' $n ;; (*) printf 'IP:%s,' $n ;; esac; done)
        openssl req -x509 -newkey rsa:2048 -nodes -days 825 -subj "/CN=webparacraft dev" \
            -addext "subjectAltName=${san%,}" -addext "extendedKeyUsage=serverAuth" \
            -keyout "$KEY" -out "$CERT" 2>/dev/null || die "openssl failed to make $CERT"
    fi
    log "certificate: $CERT ($names)"
}

cmd_serve() {
    check_web_dir
    [ "$1" = "--https" ] && [ -z "$CERT" ] && make_cert
    local args=(--root "$WEB_DIR" --engine-dir "$BUILD_DIR/bin" --port "$PORT" --host "$HOST")
    [ -n "$SEARCHROOT" ] && args+=(--searchroot "$SEARCHROOT")
    [ -n "$CERT" ] && args+=(--cert "$CERT" --key "$KEY")
    exec python3 -u "$REPO_DIR/scripts/serve_webparacraft.py" "${args[@]}"
}

case "${1:-cpp}" in
    setup|configure) configure ;;
    cpp|build|all)   cmd_cpp ;;
    serve)           cmd_serve "$2" ;;
    *) sed -n '2,11p' "$0"; exit 1 ;;
esac
