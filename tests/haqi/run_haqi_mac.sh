#!/usr/bin/env bash
set -e
exec python3 "$(cd "$(dirname "$0")" && pwd)/launch_haqi_native.py" "$@"
