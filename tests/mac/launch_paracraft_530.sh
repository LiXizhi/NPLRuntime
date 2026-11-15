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
    # Prefer the most recently modified built Paracraft binary across build/mac and build-client
    best_exe=""
    best_mtime=0
    for candidate in \
        "$REPO_ROOT/build/mac/bin/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build-client/bin/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build-client/bin/Release/Paracraft.app/Contents/MacOS/Paracraft" \
        "$REPO_ROOT/build-client/bin/Debug/Paracraft.app/Contents/MacOS/Paracraft"; do
        if [ -x "$candidate" ]; then
            mtime="$(stat -f %m "$candidate" 2>/dev/null || echo 1)"
            if [ "$mtime" -ge "$best_mtime" ]; then
                best_mtime="$mtime"
                best_exe="$candidate"
            fi
        fi
    done
    PARAENGINE_EXE="$best_exe"
fi

if [ -z "${PARAENGINE_EXE:-}" ] || [ ! -x "$PARAENGINE_EXE" ]; then
    echo "[ERROR] Built Paracraft binary not found under build/mac/bin/ or build-client/bin/." >&2
    echo "        Run './build_linux.sh client' (or VSCode task 'Mac: Build Paracraft') first." >&2
    exit 1
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
    "pid=$WORLD_PID"
    "${EXTRA_ARGS[@]}"
    "paracraft://cmd/loadworld/$WORLD_PID"
)

if [ "$WAIT_MODE" = "1" ]; then
    exec "$PARAENGINE_EXE" "${CMD_ARGS[@]}"
else
    LOG_DIR="$HOME/Documents/Paracraft/files"
    mkdir -p "$LOG_DIR"
    nohup "$PARAENGINE_EXE" "${CMD_ARGS[@]}" >"$LOG_DIR/stdout.log" 2>&1 </dev/null &
    APP_PID=$!
    echo "  PID      : $APP_PID (stdout -> $LOG_DIR/stdout.log)"
fi
