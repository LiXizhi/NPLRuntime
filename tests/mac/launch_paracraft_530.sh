#!/usr/bin/env bash
# ============================================================================
# Launch Paracraft (../ParaEngine/paraworld/) on macOS with World PID 530
# Usage:
#   launch_paracraft_530.sh [renderer] [world_pid] [--wait] [extra_args...]
#
# Supported renderers on macOS:
#   auto | metal | opengl | null
# ============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

if [ -z "${PARAWORLD_ROOT:-}" ]; then
    for candidate in \
        "$REPO_ROOT/../ParaEngine/paraworld" \
        "$REPO_ROOT/../ParaEngine/Paraworld" \
        "$REPO_ROOT/../ParaEngine/ParaWorld" \
        "$REPO_ROOT/ParaWorld"; do
        if [ -d "$candidate/script" ] || [ -f "$candidate/paraengine.sig" ]; then
            PARAWORLD_ROOT="$(cd "$candidate" && pwd)"
            break
        fi
    done
fi

if [ -z "${PARAWORLD_ROOT:-}" ] || { [ ! -d "$PARAWORLD_ROOT/script" ] && [ ! -f "$PARAWORLD_ROOT/paraengine.sig" ]; }; then
    echo "[ERROR] Paraworld directory not found (checked ../ParaEngine/paraworld)." >&2
    echo "        Set PARAWORLD_ROOT to your ParaEngine/paraworld checkout path." >&2
    exit 1
fi

if [ -z "${PARAENGINE_EXE:-}" ]; then
    # Prefer the most recently modified built Paracraft binary across build/mac-current, build/mac, and build-client
    best_exe=""
    best_mtime=0
    for candidate in \
        "$REPO_ROOT/build/mac/bin/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build/mac/bin/Release/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build/mac/bin/Debug/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build/mac-current/bin/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build/mac-current/bin/Release/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build/mac-current/bin/Debug/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build-client/bin/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build-client/bin/Release/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build-client/bin/Debug/Paracraft.app/Contents/MacOS/Paracraft"; do
        if [ -x "$candidate" ]; then
            mtime="$(stat -f %m "$candidate" 2>/dev/null || echo 1)"
            if [ "$mtime" -gt "$best_mtime" ]; then
                best_mtime="$mtime"
                best_exe="$candidate"
            fi
        fi
    done
    PARAENGINE_EXE="$best_exe"
fi

if [ -z "${PARAENGINE_EXE:-}" ] || [ ! -x "$PARAENGINE_EXE" ]; then
    echo "[ERROR] Built Paracraft binary not found under build/mac-current/bin/, build/mac/bin/, or build-client/bin/." >&2
    echo "        Run './build_linux.sh client' (or VSCode task 'Mac: Build Paracraft') first." >&2
    exit 1
fi

APP_BUNDLE="$(cd "$(dirname "$PARAENGINE_EXE")/../.." && pwd -P)"
if [[ "$APP_BUNDLE" == *.app ]]; then
    PARAENGINE_EXE="$APP_BUNDLE/Contents/MacOS/$(basename "$PARAENGINE_EXE")"
fi

# Ensure the bundle has a valid ad-hoc signature on Apple Silicon before launching
if [[ "$APP_BUNDLE" == *.app ]]; then
    if ! /usr/bin/codesign --verify "$APP_BUNDLE" >/dev/null 2>&1; then
        echo "[ParaRHI] Re-signing $APP_BUNDLE (ad-hoc)..."
        /usr/bin/codesign --force --deep --sign - "$APP_BUNDLE"
    fi
fi

REQ_RENDERER="auto"
if [ $# -gt 0 ]; then
    case "$1" in
        auto|metal|opengl|null)
            REQ_RENDERER="$1"
            shift
            ;;
    esac
fi

WORLD_PID="530"
WAIT_MODE="${FOREGROUND:-0}"
EXTRA_ARGS=()

while [ $# -gt 0 ]; do
    case "$1" in
        --wait|--foreground|-w)
            WAIT_MODE=1
            ;;
        *)
            if [[ "$1" =~ ^[0-9]+$ ]]; then
                WORLD_PID="$1"
            else
                EXTRA_ARGS+=("$1")
            fi
            ;;
    esac
    shift
done

RHI_FLAG="--renderer=$REQ_RENDERER"

cd "$PARAWORLD_ROOT"

echo "[ParaRHI] Launching Paracraft on macOS (World PID $WORLD_PID)"
echo "  App      : $APP_BUNDLE"
echo "  Exe      : $PARAENGINE_EXE"
echo "  DevDir   : $PARAWORLD_ROOT"
echo "  Renderer : $RHI_FLAG"

CMD_ARGS=(
    "$RHI_FLAG"
    "dev=$PARAWORLD_ROOT"
    "mc=true"
    "debug=main"
    "isDevEnv=true"
    "noupdate=true"
    "noclientupdate=true"
    "bootstrapper=script/apps/Aries/main_loop.lua"
    "pid=$WORLD_PID"
    "${EXTRA_ARGS[@]}"
    "paracraft://cmd/loadworld/$WORLD_PID"
)

if [ "$WAIT_MODE" = "1" ] || [[ "$APP_BUNDLE" != *.app ]]; then
    exec "$PARAENGINE_EXE" "${CMD_ARGS[@]}"
else
    EXISTING_PIDS="$(pgrep -f "$PARAENGINE_EXE" || true)"
    if [ -n "$EXISTING_PIDS" ]; then
        echo "  Stopping previous Paracraft instance ($EXISTING_PIDS)..."
        kill $EXISTING_PIDS 2>/dev/null || true
        for _ in 1 2 3 4 5 6 7 8 9 10; do
            if ! pgrep -f "$PARAENGINE_EXE" >/dev/null 2>&1; then
                break
            fi
            sleep 0.2
        done
    fi
    # Launch via macOS LaunchServices (`open -n -a`) so launchd owns the GUI process (PPID=1)
    # and VSCode closing/reusing the task terminal never terminates Paracraft.
    open -n -a "$APP_BUNDLE" --args "${CMD_ARGS[@]}"
    APP_PID=""
    for _ in 1 2 3 4 5 6; do
        sleep 0.5
        APP_PID="$(pgrep -n -f "$PARAENGINE_EXE" || true)"
        if [ -n "$APP_PID" ]; then
            break
        fi
    done
    if [ -z "$APP_PID" ]; then
        echo "[ERROR] Paracraft failed to stay running after launch. Check ~/Documents/Paracraft/files/log.txt" >&2
        exit 1
    fi
    echo "  PID      : $APP_PID (log -> $HOME/Documents/Paracraft/files/log.txt)"
fi
