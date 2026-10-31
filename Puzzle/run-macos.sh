#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
if [ "${1:-}" = puzzle ]; then shift; fi
sh ./build-macos.sh
exec ./build/macos-puzzle/last_portal "$@"
