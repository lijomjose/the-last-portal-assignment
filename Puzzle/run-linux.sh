#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
if [ "$(uname -s)" = Darwin ]; then exec sh ./run-macos.sh "$@"; fi
default_edition=puzzle
[ -d puzzle/student ] || default_edition=completed
edition=${1:-$default_edition}
if [ "$#" -gt 0 ]; then shift; fi
./build-linux.sh "$edition"
exec "./build/$edition/last_portal" "$@"
