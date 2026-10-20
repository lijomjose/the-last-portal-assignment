#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
exec sh Puzzle/debug-linux.sh "$@"
