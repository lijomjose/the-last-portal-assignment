#!/usr/bin/env sh
set -u
cd "$(dirname "$0")" || exit 1
if [ "${1:-}" = puzzle ]; then shift; fi
if [ "$#" -ne 0 ]; then
    printf 'Usage: sh build-macos.sh [puzzle]\n' >&2
    exit 2
fi
. ./macos-common.sh
portal_macos_build release
