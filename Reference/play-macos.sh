#!/usr/bin/env sh
set -u
cd "$(dirname "$0")" || exit 1
if [ "$(uname -s)" != Darwin ]; then
    printf 'This launcher needs macOS. Use the Windows or Linux launcher for those systems.\n' >&2
    exit 2
fi
if [ ! -f bin/last_portal-macos ]; then
    printf 'The completed Mac executable is missing. Extract the entire Mac Reference package.\n' >&2
    exit 1
fi
chmod +x bin/last_portal-macos || exit 1
printf 'Opening the completed Reference game. No compiler or rebuild is needed.\n'
./bin/last_portal-macos "$@" >macos-reference.log 2>&1
result=$?
if [ "$result" -ne 0 ]; then
    cat macos-reference.log
    printf '\nThe game exited with status %s. Send macos-reference.log and the Terminal error to your instructor.\nSee README_MACOS.md if macOS blocked this downloaded application.\n' "$result" >&2
fi
exit "$result"
