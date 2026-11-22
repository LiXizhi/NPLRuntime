#!/usr/bin/env bash
# ============================================================================
# NPLRuntimeUnified — Unix (Linux / macOS / WSL) Unified Build & Run Script
#
# Replaces all legacy root shell scripts (build_mac_*.sh, build_ios_*.sh,
# build_sdl2.sh, build_emscripten.sh, npl_install.sh) with a single entry point.
#
# Usage:
#   ./build_linux.sh                              Build Headless Server (Release, all cores)
#   ./build_linux.sh server [Release|Debug]       Build Headless Server -> build-server/bin/ParaEngineServer
#   ./build_linux.sh client [Release|Debug]       Build Desktop Client  -> build-client/bin/ (SDL2 on Linux, Cocoa on macOS)
#   ./build_linux.sh web [build|configure|serve]  Build/serve WebAssembly via scripts/dev_web.sh
#   ./build_linux.sh android [gradle_task]        Build Android APK (NPLRuntime/Platform/AndroidStudio)
#   ./build_linux.sh picovr [gradle_task]         Build PICO VR OpenXR APK (NPLRuntime/Platform/PicoVR)
#   ./build_linux.sh ios [framework|device|sim]   Configure/build iOS targets on macOS (Xcode)
#   ./build_linux.sh run [renderer] [world_pid]   Launch desktop client (default: auto 530)
#   ./build_linux.sh run-server [script.lua]      Run ParaEngineServer (default: NPLRuntime/tests/helloworld.lua)
#   ./build_linux.sh test                         Run CTest verification suite
#   ./build_linux.sh <jobs> [Release|Debug]       Legacy positional syntax for server build
#   ./build_linux.sh --help                       Show this help message
# ============================================================================

set -e

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
HOST_OS="$(uname -s)"
DEFAULT_JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
JOBS="${JOBS:-$DEFAULT_JOBS}"

ensure_cmake_and_deps() {
    if ! command -v cmake >/dev/null 2>&1; then
        for dir in \
            "/opt/homebrew/bin" \
            "/usr/local/bin" \
            "/Applications/CMake.app/Contents/bin" \
            "$REPO_ROOT/../deps/cmake/CMake.app/Contents/bin" \
            "$HOME/lxzsrc/deps/cmake/CMake.app/Contents/bin" \
            "$REPO_ROOT"/../deps/cmake-*/CMake.app/Contents/bin \
            "$HOME"/lxzsrc/deps/cmake-*/CMake.app/Contents/bin; do
            if [ -x "$dir/cmake" ]; then
                export PATH="$dir:$PATH"
                break
            fi
        done
    fi

    if [ "$HOST_OS" = "Darwin" ] && [ -z "${BOOST_ROOT:-}" ]; then
        for boost_dir in \
            "$REPO_ROOT/../deps/boost_1_85_0" \
            "$HOME/lxzsrc/deps/boost_1_85_0" \
            "$HOME/boost_1_85_0" \
            "/opt/homebrew/opt/boost"; do
            if [ -d "$boost_dir" ]; then
                BOOST_ROOT="$(cd "$boost_dir" && pwd)"
                export BOOST_ROOT
                break
            fi
        done
    fi

    if [ "$HOST_OS" = "Darwin" ] && [ -n "${BOOST_ROOT:-}" ] && [ -z "${BOOST_LIBRARYDIR:-}" ]; then
        if [ -d "$BOOST_ROOT/macos-build/stage/lib" ]; then
            BOOST_LIBRARYDIR="$BOOST_ROOT/macos-build/stage/lib"
            export BOOST_LIBRARYDIR
        elif [ -d "$BOOST_ROOT/stage/lib" ]; then
            BOOST_LIBRARYDIR="$BOOST_ROOT/stage/lib"
            export BOOST_LIBRARYDIR
        fi
    fi
}

ensure_cmake_and_deps

usage() {
    cat <<'EOF'
NPLRuntimeUnified — Unix (Linux / macOS / WSL) Build & Run Script

Usage:
  ./build_linux.sh                              Build Headless Server (Release) -> build-server/bin/ParaEngineServer
  ./build_linux.sh server [Release|Debug]       Build Headless Server (-DNPLRUNTIME_SERVER=ON)
  ./build_linux.sh client [Release|Debug]       Build Desktop Client (ParaCraftSDL2 on Linux, Paracraft.app on macOS)
  ./build_linux.sh web [build|configure|serve]  Build/serve Emscripten WebAssembly (delegates to scripts/dev_web.sh)
  ./build_linux.sh android [gradle_task]        Build Android APK (NPLRuntime/Platform/AndroidStudio)
  ./build_linux.sh picovr [gradle_task]         Build PICO VR OpenXR APK (NPLRuntime/Platform/PicoVR)
  ./build_linux.sh ios [framework|device|sim]   Configure/build iOS targets (macOS + Xcode required)
  ./build_linux.sh run [renderer] [world_pid]   Launch Desktop Client (e.g. './build_linux.sh run auto 530')
  ./build_linux.sh run-server [script.lua]      Run ParaEngineServer (default: NPLRuntime/tests/helloworld.lua)
  ./build_linux.sh test                         Run CTest suite
  ./build_linux.sh <jobs> [Release|Debug]       Legacy positional syntax for server build

Options (for server/client builds):
  --configure, -c      Force CMake re-configuration
  --clean              Remove the target build directory before building
  --oce                Enable OpenCASCADE CAD plugin (-DNPLRUNTIME_OCE=ON)
  --robotics           Enable MuJoCo + ONNX Runtime robotics (default ON on macOS client)
  --no-robotics        Disable MuJoCo + ONNX Runtime robotics
  --xcode              Generate Xcode project on macOS (-G Xcode) instead of Makefiles
  --build-sdl2         Download and build SDL2 2.0.22 into build-sdl2-deps/ (Linux client)
  --install            Symlink ParaEngineServer to /usr/local/bin/npl
  --help, -h           Show this help message

Environment Variables:
  BOOST_ROOT           Path to Boost root (if not installed in system paths)
  JOBS                 Parallel build worker count (default: all CPU cores)
  SDL_ROOT             Optional custom SDL2 install prefix for Linux client builds
EOF
}

cmd_ios() {
    local ios_mode="${1:-framework}"
    local toolchain="$REPO_ROOT/NPLRuntime/cmake/ios.toolchain.cmake"
    case "$ios_mode" in
        device|os)
            mkdir -p "$REPO_ROOT/build-ios-device"
            cmake -S "$REPO_ROOT/NPLRuntime" -B "$REPO_ROOT/build-ios-device" \
                -G Xcode \
                -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
                -DIOS_PLATFORM=OS \
                -DENABLE_BITCODE=0 \
                -DIOS_DEPLOYMENT_TARGET=14.0
            echo "[build_linux] Configured Xcode iOS device project at $REPO_ROOT/build-ios-device"
            ;;
        sim|simulator)
            mkdir -p "$REPO_ROOT/build-ios-sim"
            cmake -S "$REPO_ROOT/NPLRuntime" -B "$REPO_ROOT/build-ios-sim" \
                -G Xcode \
                -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
                -DIOS_PLATFORM=SIMULATOR \
                -DENABLE_BITCODE=OFF \
                -DIOS_DEPLOYMENT_TARGET=14.0
            echo "[build_linux] Configured Xcode iOS simulator project at $REPO_ROOT/build-ios-sim"
            ;;
        framework)
            local fw_build="$REPO_ROOT/NPLRuntime/build_ios_framework"
            local fw_install="$REPO_ROOT/NPLRuntime/install_ios_framework"
            local boost_ios="${BOOST_ROOT:-$REPO_ROOT/NPLRuntime/externals/boost/prebuild/apple/build/boost/1.84.0/ios/prefix}"
            rm -rf "$fw_build" "$fw_install"
            mkdir -p "$fw_build/builds/mac"
            if [ -f "$REPO_ROOT/NPLRuntime/externals/freetype-2.8.1/builds/mac/freetype-Info.plist" ]; then
                cp "$REPO_ROOT/NPLRuntime/externals/freetype-2.8.1/builds/mac/freetype-Info.plist" "$fw_build/builds/mac/"
            fi
            BOOST_ROOT="$boost_ios" cmake -S "$REPO_ROOT/NPLRuntime" -B "$fw_build" \
                -G Xcode \
                -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
                -DPLATFORM=OS64 \
                -DDEPLOYMENT_TARGET=14.0 \
                -DCMAKE_INSTALL_PREFIX="$fw_install" \
                -DBUILD_FRAMEWORK=ON \
                -DIOS=ON \
                -DNPLRUNTIME_STATIC_LIB=ON \
                -DNPLRUNTIME_LUAJIT21=ON \
                -DNPLRUNTIME_SUPPORT_FBX=ON \
                -DNPLRUNTIME_PHYSICS=ON \
                -DNPLRUNTIME_AUDIO=ON \
                -DCMAKE_XCODE_ATTRIBUTE_SKIP_INSTALL=NO \
                -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED=NO \
                -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="" \
                -DCMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE=NO
            if [ -f "$fw_build/NPLRuntime.xcodeproj/project.pbxproj" ]; then
                cp "$fw_build/NPLRuntime.xcodeproj/project.pbxproj" "$fw_build/NPLRuntime.xcodeproj/project.pbxproj.backup"
                sed -i '' '/module\.modulemap/d' "$fw_build/NPLRuntime.xcodeproj/project.pbxproj"
            fi
            xcodebuild -project "$fw_build/NPLRuntime.xcodeproj" \
                -target ParacraftFramework \
                -configuration Release \
                -sdk iphoneos \
                CODE_SIGNING_REQUIRED=NO \
                CODE_SIGN_IDENTITY="" \
                ENABLE_BITCODE=NO \
                ONLY_ACTIVE_ARCH=NO \
                -allowProvisioningUpdates
            if [ -f "$fw_build/NPLRuntime.xcodeproj/project.pbxproj.backup" ]; then
                mv "$fw_build/NPLRuntime.xcodeproj/project.pbxproj.backup" "$fw_build/NPLRuntime.xcodeproj/project.pbxproj"
            fi
            echo "[build_linux] Built ParacraftFramework.framework in $fw_build"
            ;;
        *)
            echo "[build_linux] Unknown iOS mode: $ios_mode (expected: framework | device | sim)" >&2
            exit 1
            ;;
    esac
}

build_local_sdl2() {
    local sdl_ver="2.0.22"
    local sdl_root_dir="$REPO_ROOT/build-sdl2-deps"
    local sdl_install="$sdl_root_dir/install"
    if [ ! -d "$sdl_install" ]; then
        mkdir -p "$sdl_root_dir"
        local tarball="$sdl_root_dir/SDL2-${sdl_ver}.tar.gz"
        if [ ! -f "$tarball" ]; then
            curl -fsSL "https://github.com/libsdl-org/SDL/releases/download/release-${sdl_ver}/SDL2-${sdl_ver}.tar.gz" -o "$tarball"
        fi
        tar -xzf "$tarball" -C "$sdl_root_dir"
        cmake -S "$sdl_root_dir/SDL2-${sdl_ver}" -B "$sdl_root_dir/build" -DCMAKE_INSTALL_PREFIX="$sdl_install" -DCMAKE_BUILD_TYPE=Release
        cmake --build "$sdl_root_dir/build" --parallel "$JOBS"
        cmake --install "$sdl_root_dir/build"
    fi
    SDL_ROOT="$sdl_install"
}

case "${1:-}" in
    --help|-h|help)
        usage
        exit 0
        ;;
    web)
        shift
        exec "$REPO_ROOT/scripts/dev_web.sh" "${@:-build}"
        ;;
    android)
        shift
        cd "$REPO_ROOT/NPLRuntime/Platform/AndroidStudio"
        exec ./gradlew "${@:-assembleDebug}"
        ;;
    picovr)
        shift
        cd "$REPO_ROOT/NPLRuntime/Platform/PicoVR"
        exec ./gradlew "${@:-:PicoVR:assembleDebug}"
        ;;
    ios)
        shift
        cmd_ios "${1:-framework}"
        exit 0
        ;;
    run)
        shift
        if [ "$HOST_OS" = "Darwin" ]; then
            exec "$REPO_ROOT/tests/mac/launch_paracraft_530.sh" "$@"
        else
            CLIENT_BIN="$REPO_ROOT/build-client/bin/ParaCraftSDL2"
            [ -x "$CLIENT_BIN" ] || CLIENT_BIN="$REPO_ROOT/ParaWorld/bin64/ParaCraftSDL2"
            if [ ! -x "$CLIENT_BIN" ]; then
                echo "[build_linux] ERROR: ParaCraftSDL2 binary not found. Run './build_linux.sh client' first." >&2
                exit 1
            fi
            REQ_RENDERER="${1:-auto}"
            WORLD_PID="${2:-530}"
            exec "$CLIENT_BIN" "--renderer=$REQ_RENDERER" mc="true" debug="main" isDevEnv="true" noupdate="true" "pid=$WORLD_PID" "paracraft://cmd/loadworld/$WORLD_PID"
        fi
        ;;
    run-server)
        shift
        SERVER_BIN="$REPO_ROOT/build-server/bin/ParaEngineServer"
        [ -x "$SERVER_BIN" ] || SERVER_BIN="$REPO_ROOT/ParaWorld/bin64/ParaEngineServer"
        if [ ! -x "$SERVER_BIN" ]; then
            echo "[build_linux] ERROR: ParaEngineServer binary not found. Run './build_linux.sh server' first." >&2
            exit 1
        fi
        SCRIPT_PATH="${1:-$REPO_ROOT/NPLRuntime/tests/helloworld.lua}"
        export LD_LIBRARY_PATH="$(dirname "$SERVER_BIN"):${LD_LIBRARY_PATH:-}"
        export DYLD_LIBRARY_PATH="$(dirname "$SERVER_BIN"):${DYLD_LIBRARY_PATH:-}"
        exec "$SERVER_BIN" bootstrapper="$SCRIPT_PATH"
        ;;
    test)
        shift
        if [ -d "$REPO_ROOT/build-server" ]; then
            ctest --test-dir "$REPO_ROOT/build-server" --output-on-failure || true
        fi
        for client_test_dir in \
            "$REPO_ROOT/build/mac" \
            "$REPO_ROOT/build/mac-current" \
            "$REPO_ROOT/build-client"; do
            if [ -f "$client_test_dir/CTestTestfile.cmake" ]; then
                ctest --test-dir "$client_test_dir" --output-on-failure || true
                break
            fi
        done
        exit 0
        ;;
esac

MODE="server"
BUILD_TYPE="Release"
FORCE_CONFIGURE=0
CLEAN_BUILD=0
DO_INSTALL=0
ENABLE_OCE=0
USE_XCODE=0
BUILD_SDL2_FROM_SRC=0
ROBOTICS_OPT="auto"

# Legacy compatibility: `./build_linux.sh <jobs> [Release|Debug]`
if [[ "${1:-}" =~ ^[0-9]+$ ]]; then
    JOBS="$1"
    shift
    if [ $# -gt 0 ] && [[ "${1:-}" != --* ]]; then
        BUILD_TYPE="$1"
        shift
    fi
fi

while [ $# -gt 0 ]; do
    case "$1" in
        server)          MODE="server" ;;
        client)          MODE="client" ;;
        Debug|debug)     BUILD_TYPE="Debug" ;;
        Release|release) BUILD_TYPE="Release" ;;
        RelWithDebInfo)  BUILD_TYPE="RelWithDebInfo" ;;
        --configure|-c)  FORCE_CONFIGURE=1 ;;
        --clean)         CLEAN_BUILD=1 ;;
        --oce)           ENABLE_OCE=1 ;;
        --robotics)      ROBOTICS_OPT="on" ;;
        --no-robotics)   ROBOTICS_OPT="off" ;;
        --xcode)         USE_XCODE=1 ;;
        --build-sdl2)    BUILD_SDL2_FROM_SRC=1 ;;
        --install)       DO_INSTALL=1 ;;
        *)
            echo "[build_linux] Unknown argument: $1 (run ./build_linux.sh --help)" >&2
            exit 1
            ;;
    esac
    shift
done

CMAKE_EXTRA_ARGS=()
if [ -n "${BOOST_ROOT:-}" ]; then
    CMAKE_EXTRA_ARGS+=("-DBOOST_ROOT=$BOOST_ROOT")
fi
if [ -n "${BOOST_LIBRARYDIR:-}" ]; then
    CMAKE_EXTRA_ARGS+=("-DBOOST_LIBRARYDIR=$BOOST_LIBRARYDIR")
fi
if [ "$ENABLE_OCE" -eq 1 ]; then
    CMAKE_EXTRA_ARGS+=("-DNPLRUNTIME_OCE=ON")
fi
if [ "$USE_XCODE" -eq 1 ]; then
    CMAKE_EXTRA_ARGS+=("-G" "Xcode")
fi

BUILD_TARGET_ARGS=()
if [ "$MODE" = "server" ]; then
    BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build-server}"
    CMAKE_EXTRA_ARGS+=("-DNPLRUNTIME_SERVER=ON" "-DNPLRUNTIME_RENDERER=NULL")
    if [ "$HOST_OS" = "Darwin" ]; then
        CMAKE_EXTRA_ARGS+=("-DMAC_SERVER=ON")
    fi
else
    if [ "$HOST_OS" = "Darwin" ]; then
        if [ -z "${BUILD_DIR:-}" ]; then
            if [ -f "$REPO_ROOT/build/mac-current/CMakeCache.txt" ] && [ ! -e "$REPO_ROOT/build/mac" ] && [ ! -L "$REPO_ROOT/build/mac" ]; then
                ln -s mac-current "$REPO_ROOT/build/mac" 2>/dev/null || true
            fi
            if [ -f "$REPO_ROOT/build/mac/CMakeCache.txt" ]; then
                BUILD_DIR="$REPO_ROOT/build/mac"
            elif [ -f "$REPO_ROOT/build/mac-current/CMakeCache.txt" ]; then
                BUILD_DIR="$REPO_ROOT/build/mac-current"
            elif [ -f "$REPO_ROOT/build-client/CMakeCache.txt" ]; then
                BUILD_DIR="$REPO_ROOT/build-client"
            else
                BUILD_DIR="$REPO_ROOT/build/mac"
            fi
        fi
        CMAKE_EXTRA_ARGS+=("-DNPLRUNTIME_SERVER=OFF" "-DNPLRUNTIME_RENDERER=OPENGL" "-DNPLRUNTIME_ENABLE_METAL=ON" "-DNPLRUNTIME_STATIC_LIB=ON" "-DMAC_SERVER=OFF")
        BUILD_TARGET_ARGS+=("--target" "Paracraft")
    else
        BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build-client}"
        if [ "$BUILD_SDL2_FROM_SRC" -eq 1 ]; then
            build_local_sdl2
            CMAKE_EXTRA_ARGS+=("-DALSOFT_BACKEND_SDL2=TRUE" "-DALSOFT_BACKEND_OSS=FALSE" "-DALSOFT_BACKEND_WAVE=FALSE" "-DALSOFT_BACKEND_ALSA=FALSE")
        fi
        CMAKE_EXTRA_ARGS+=("-DNPLRUNTIME_SERVER=OFF" "-DLINUX=ON" "-DNPLRUNTIME_RENDERER=OPENGL")
        if [ -n "${SDL_ROOT:-}" ]; then
            CMAKE_EXTRA_ARGS+=("-DSDL_ROOT=$SDL_ROOT")
        fi
    fi
fi

if [ "$CLEAN_BUILD" -eq 1 ] && [ -d "$BUILD_DIR" ]; then
    echo "[build_linux] Cleaning $BUILD_DIR..."
    rm -rf "$BUILD_DIR"
fi

if [ "$HOST_OS" = "Darwin" ]; then
    OSX_ARCHS="${NPL_OSX_ARCHS:-}"
    if [ -z "$OSX_ARCHS" ] && [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
        OSX_ARCHS="$(sed -n 's/^NPL_OSX_ARCHS:[^=]*=//p' "$BUILD_DIR/CMakeCache.txt" | head -n 1)"
    fi
    if [ -z "$OSX_ARCHS" ] && [ -n "${BOOST_LIBRARYDIR:-}" ] && command -v lipo >/dev/null 2>&1; then
        for boost_lib in "$BOOST_LIBRARYDIR"/libboost_*.a; do
            if [ -f "$boost_lib" ]; then
                boost_archs="$(lipo -archs "$boost_lib" 2>/dev/null || true)"
                if [ -n "$boost_archs" ] && [[ "$boost_archs" != *" "* ]]; then
                    OSX_ARCHS="$boost_archs"
                fi
                break
            fi
        done
    fi
    if [ -n "$OSX_ARCHS" ]; then
        CMAKE_EXTRA_ARGS+=("-DNPL_OSX_ARCHS=$OSX_ARCHS")
    fi
fi

if [ "$ROBOTICS_OPT" = "on" ]; then
    CMAKE_EXTRA_ARGS+=("-DNPLRUNTIME_ENABLE_ROBOTICS=ON")
    if [ "$MODE" = "client" ] && [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
        if ! grep -q "^NPLRUNTIME_ENABLE_ROBOTICS:BOOL=ON" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null; then
            echo "[build_linux] Enabling MuJoCo + ONNX Runtime in $BUILD_DIR..."
            FORCE_CONFIGURE=1
        fi
    fi
elif [ "$ROBOTICS_OPT" = "off" ]; then
    CMAKE_EXTRA_ARGS+=("-DNPLRUNTIME_ENABLE_ROBOTICS=OFF" "-DNPLRUNTIME_MUJOCO=OFF" "-DNPLRUNTIME_ONNXRUNTIME=OFF")
    if [ "$MODE" = "client" ] && [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
        if grep -q "^NPLRUNTIME_ENABLE_ROBOTICS:BOOL=ON" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null; then
            echo "[build_linux] Disabling MuJoCo + ONNX Runtime in $BUILD_DIR..."
            FORCE_CONFIGURE=1
        fi
    fi
elif [ "$MODE" = "client" ] && [ "$HOST_OS" = "Darwin" ]; then
    if [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
        if grep -q "^NPLRUNTIME_ENABLE_ROBOTICS:BOOL=OFF" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null; then
            CMAKE_EXTRA_ARGS+=("-DNPLRUNTIME_ENABLE_ROBOTICS=OFF" "-DNPLRUNTIME_MUJOCO=OFF" "-DNPLRUNTIME_ONNXRUNTIME=OFF")
        elif grep -q "^NPLRUNTIME_ENABLE_ROBOTICS:BOOL=ON" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null; then
            CMAKE_EXTRA_ARGS+=("-DNPLRUNTIME_ENABLE_ROBOTICS=ON")
        fi
    elif [ -n "${MUJOCO_ROOT:-}" ] && [ -n "${ONNXRUNTIME_ROOT:-}" ]; then
        CMAKE_EXTRA_ARGS+=("-DNPLRUNTIME_ENABLE_ROBOTICS=ON")
    else
        CMAKE_EXTRA_ARGS+=("-DNPLRUNTIME_ENABLE_ROBOTICS=OFF" "-DNPLRUNTIME_MUJOCO=OFF" "-DNPLRUNTIME_ONNXRUNTIME=OFF")
    fi
fi

if [ "$FORCE_CONFIGURE" -eq 1 ] || [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "[build_linux] Configuring $MODE ($BUILD_TYPE) in $BUILD_DIR..."
    cmake -S "$REPO_ROOT/NPLRuntime" -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        "${CMAKE_EXTRA_ARGS[@]}"
fi

echo "[build_linux] Building $MODE ($BUILD_TYPE) with $JOBS parallel jobs..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" "${BUILD_TARGET_ARGS[@]}" --parallel "$JOBS"

echo ""
echo "[build_linux] Build succeeded!"
echo "  Build bin  : $BUILD_DIR/bin/"
if [ "$MODE" = "client" ] && [ "$HOST_OS" = "Darwin" ]; then
    echo "  App bundle : $BUILD_DIR/bin/Paracraft.app"
else
    echo "  Copied to  : $REPO_ROOT/ParaWorld/bin64/"
fi

if [ "$MODE" = "server" ] && [ "$DO_INSTALL" -eq 1 ]; then
    SERVER_OUT="$REPO_ROOT/ParaWorld/bin64/ParaEngineServer"
    [ -f "$SERVER_OUT" ] || SERVER_OUT="$BUILD_DIR/bin/ParaEngineServer"
    if [ -w /usr/local/bin ]; then
        ln -sf "$SERVER_OUT" /usr/local/bin/npl
        echo "  Symlinked : /usr/local/bin/npl -> $SERVER_OUT"
    else
        echo "  [WARN] /usr/local/bin is not writable; run with sudo if you want /usr/local/bin/npl symlinked."
    fi
fi
