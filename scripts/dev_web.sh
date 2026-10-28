#!/bin/bash
# date: 2026.10
# desc: one-command edit/build/test loop for the Emscripten single-thread client (Linux/macOS).
#       C++ changes -> rebuild ParaCraftSingleThread.{js,wasm}
#       NPL/Lua changes in paracraft_script -> overlay them on the released script package
#       both are copied into ~/webparacraft/auto_generate_files and served locally.
#       see docs/web-dev-workflow.md
#
# usage: scripts/dev_web.sh [setup|cpp|script|all|serve|status|restore]   (default: all)
#        scripts/dev_web.sh script --all     overlay the whole script/ config/ Mod/ tree
#        HOST=127.0.0.1 scripts/dev_web.sh serve   this machine only (default: also LAN)
#        scripts/dev_web.sh serve --https    https with a generated certificate (or CERT=... KEY=...)

set -e

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"

## all paths can be overridden with environment variables
EMSDK_DIR=${EMSDK_DIR:-$HOME/emsdk/emsdk}
BOOST_DIR=${BOOST_DIR:-$HOME/boost_1_85_0}
BUILD_DIR=${BUILD_DIR:-$REPO_DIR/build/emscripten_single_thread}
## webparacraft checkout (html, js, auto_generate_files)
WEB_DIR=${WEB_DIR:-$HOME/webparacraft}
## paracraft_script checkout (NPL/Lua source)
SCRIPT_SRC=${SCRIPT_SRC:-$HOME/CODE/paracraft_script}
## paracraft asset root; only passed to cmake, the asset package is not rebuilt here
APP_ROOT=${APP_ROOT:-$HOME/paracraft}
PORT=${PORT:-8088}
## 0.0.0.0 also serves the LAN; 127.0.0.1 for this machine only
HOST=${HOST:-0.0.0.0}
JOBS=${JOBS:-$(getconf _NPROCESSORS_ONLN)}
## certificates made by 'serve --https'
CERT_DIR=${CERT_DIR:-$REPO_DIR/build/web_cert}

TARGET=ParaCraftSingleThread
OUT_DIR=$WEB_DIR/auto_generate_files
## original script package files (main150727.pkg, ...) extracted from webparacraft git HEAD
SCRIPT_BASE_DIR=$BUILD_DIR/web_script_base
## base + overlaid source files; packed into paracraft_asset_script.data
SCRIPT_STAGE_DIR=$BUILD_DIR/web_script_stage
## paracraft_script commit the overlay diff is taken against
SCRIPT_BASE_COMMIT_FILE=$BUILD_DIR/web_script_base.commit
## source folders that are overlaid on the package
OVERLAY_DIRS="script config Mod"

log() { echo "[dev_web] $*"; }
die() { echo "[dev_web] ERROR: $*" >&2; exit 1; }

## emsdk needs python >= 3.10; macOS system python is 3.9
find_emsdk_python() {
    if [ -n "$EMSDK_PYTHON" ]; then return; fi
    for py in python3.13 python3.12 python3.11 python3.10 /opt/homebrew/bin/python3.12 /usr/local/bin/python3.12 python3; do
        if command -v $py >/dev/null 2>&1 && $py -c 'import sys; sys.exit(sys.version_info < (3, 10))' 2>/dev/null; then
            export EMSDK_PYTHON=$(command -v $py)
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
}

check_web_dir() {
    [ -f "$WEB_DIR/index.html" ] || die "webparacraft not found in $WEB_DIR (git clone --depth 1 http://code.kp-para.cn/paracraft/webparacraft.git $WEB_DIR)"
}

update_md5() {
    ## the page and ServiceWorker.js cache .js/.wasm/.data by the hash in this file
    (cd "$WEB_DIR" && bash calculate_md5.sh >/dev/null)
    log "updated auto_generate_files/paracraft_sitefile_md5sums.js"
}

configure() {
    load_emsdk
    [ -d "$BOOST_DIR/stage/lib" ] || die "boost not built in $BOOST_DIR/stage/lib (see docs/web-dev-workflow.md)"
    ## find_package(ZLIB) needs the zlib port in the emscripten cache
    embuilder build zlib libjpeg libpng sdl2 >/dev/null
    emcmake cmake -S "$REPO_DIR/NPLRuntime" -B "$BUILD_DIR" \
        -DBUILD_TESTING=OFF \
        -DEMSCRIPTEN=ON \
        -DEMSCRIPTEN_SINGLE_THREAD=ON \
        -DCMAKE_BUILD_TYPE=RELEASE \
        -DBOOST_ROOT="$BOOST_DIR" \
        -DAPP_ROOT="$APP_ROOT" \
        -DHTML_ROOT="$OUT_DIR"
}

## extract the released script package from webparacraft git HEAD (not the working tree,
## which may already hold our repacked .data)
extract_script_base() {
    local tmp=$BUILD_DIR/web_script_base.tmp
    rm -rf "$SCRIPT_BASE_DIR" "$tmp"
    mkdir -p "$tmp"
    git -C "$WEB_DIR" show HEAD:auto_generate_files/paracraft_asset_script.data > "$tmp/pkg.data"
    git -C "$WEB_DIR" show HEAD:auto_generate_files/paracraft_asset_script.js > "$tmp/pkg.js"
    python3 -I - "$tmp/pkg.js" "$tmp/pkg.data" "$SCRIPT_BASE_DIR" <<'EOF'
import json, os, re, sys
js, data, out = sys.argv[1:4]
meta = json.loads(re.findall(r'loadPackage\((\{"files".*?\})\);', open(js).read())[-1])
blob = open(data, 'rb').read()
for f in meta["files"]:
    path = os.path.join(out, f["filename"].lstrip("/"))
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as fp:
        fp.write(blob[f["start"]:f["end"]])
    print("  " + f["filename"])
EOF
    rm -rf "$tmp"
}

cmd_setup() {
    check_web_dir
    [ -d "$SCRIPT_SRC/.git" ] || die "paracraft_script not found in $SCRIPT_SRC (git clone --depth 1 http://code.kp-para.cn/paracraft/paracraft_script.git $SCRIPT_SRC)"
    configure
    log "extracting released script package from webparacraft HEAD"
    extract_script_base
    git -C "$SCRIPT_SRC" rev-parse HEAD > "$SCRIPT_BASE_COMMIT_FILE"
    log "script overlay base: paracraft_script $(cut -c1-9 "$SCRIPT_BASE_COMMIT_FILE")"
    log "setup done. next: scripts/dev_web.sh all && scripts/dev_web.sh serve"
}

cmd_cpp() {
    check_web_dir
    load_emsdk
    [ -f "$BUILD_DIR/CMakeCache.txt" ] || configure
    log "building $TARGET (-j$JOBS)"
    (cd "$BUILD_DIR" && emmake make -j"$JOBS" $TARGET)
    ## the post-build step copies only when relinking, so copy explicitly
    cp "$BUILD_DIR/bin/$TARGET.js" "$BUILD_DIR/bin/$TARGET.wasm" "$OUT_DIR/"
    log "copied $TARGET.js/.wasm -> $OUT_DIR"
}

## print source files to overlay: everything under OVERLAY_DIRS that differs from the base
## commit (committed or not) plus untracked files; with --all, the whole tree
list_overlay_files() {
    local prefixes="" d
    for d in $OVERLAY_DIRS; do prefixes="$prefixes $d/"; done
    if [ "$1" = "--all" ]; then
        git -C "$SCRIPT_SRC" ls-files -- $prefixes
    else
        local base=$(cat "$SCRIPT_BASE_COMMIT_FILE")
        { git -C "$SCRIPT_SRC" diff --name-only "$base" -- $prefixes
          git -C "$SCRIPT_SRC" ls-files --others --exclude-standard -- $prefixes; } | sort -u
    fi
}

cmd_script() {
    check_web_dir
    load_emsdk
    [ -d "$SCRIPT_BASE_DIR" ] && [ -f "$SCRIPT_BASE_COMMIT_FILE" ] || die "run 'scripts/dev_web.sh setup' first"

    rm -rf "$SCRIPT_STAGE_DIR"
    cp -R "$SCRIPT_BASE_DIR" "$SCRIPT_STAGE_DIR"
    local count=0 file
    while IFS= read -r file; do
        ## deleted files show up in the diff; the released package still has them
        [ -f "$SCRIPT_SRC/$file" ] || continue
        mkdir -p "$SCRIPT_STAGE_DIR/$(dirname "$file")"
        cp "$SCRIPT_SRC/$file" "$SCRIPT_STAGE_DIR/$file"
        [ "$1" = "--all" ] || log "  overlay $file"
        count=$((count + 1))
    done < <(list_overlay_files "$1")
    log "overlaid $count file(s) on the released script package"

    (cd "$OUT_DIR" && "$EMSDK_PYTHON" "$EMSDK/upstream/emscripten/tools/file_packager.py" paracraft_asset_script.data \
        --preload "$SCRIPT_STAGE_DIR@/" --js-output=paracraft_asset_script.js >/dev/null)
    log "packed paracraft_asset_script.data/.js -> $OUT_DIR"
}

cmd_status() {
    check_web_dir
    log "engine/script files that differ from webparacraft HEAD (Jenkins build):"
    git -C "$WEB_DIR" status --short -- auto_generate_files | sed 's/^/    /'
    if [ -f "$SCRIPT_BASE_COMMIT_FILE" ]; then
        log "script files that would be overlaid (base $(cut -c1-9 "$SCRIPT_BASE_COMMIT_FILE")):"
        list_overlay_files | sed 's/^/    /'
    fi
}

cmd_restore() {
    check_web_dir
    git -C "$WEB_DIR" checkout -- auto_generate_files
    log "auto_generate_files restored to webparacraft HEAD"
}

## make a certificate for localhost, 127.0.0.1 and the LAN ip; regenerated when the LAN ip changes.
## mkcert certificates are trusted by this machine (and by other devices that import `mkcert -CAROOT`/rootCA.pem);
## the openssl fallback is self-signed: the browser shows a warning and refuses to register the service worker
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
        log "mkcert not found (brew install mkcert); making a self-signed certificate with openssl"
        local san=$(for n in $names; do case $n in (*[a-z]*) printf 'DNS:%s,' $n ;; (*) printf 'IP:%s,' $n ;; esac; done)
        openssl req -x509 -newkey rsa:2048 -nodes -days 825 -subj "/CN=webparacraft dev" \
            -addext "subjectAltName=${san%,}" -addext "extendedKeyUsage=serverAuth" \
            -keyout "$KEY" -out "$CERT" 2>/dev/null || die "openssl failed to make $CERT"
    fi
    log "certificate: $CERT ($names)"
}

## set CERT and KEY (PEM files) or pass --https to serve https
cmd_serve() {
    check_web_dir
    [ "$1" = "--https" ] && [ -z "$CERT" ] && make_cert
    local args="--root $WEB_DIR --port $PORT --host $HOST"
    [ -n "$CERT" ] && args="$args --cert $CERT --key $KEY"
    exec python3 -u "$REPO_DIR/scripts/serve_webparacraft.py" $args
}

case "${1:-all}" in
    setup)   cmd_setup ;;
    cpp)     cmd_cpp; update_md5 ;;
    script)  cmd_script "$2"; update_md5 ;;
    all)     cmd_cpp; cmd_script "$2"; update_md5 ;;
    serve)   cmd_serve "$2" ;;
    status)  cmd_status ;;
    restore) cmd_restore ;;
    *) sed -n '2,12p' "$0"; exit 1 ;;
esac
