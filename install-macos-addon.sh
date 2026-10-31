#!/usr/bin/env sh
# Merge only platform support files. Never replace a project or student C file.
set -eu
cd "$(dirname "$0")"
if [ "$#" -ne 1 ] || [ ! -d "$1/Puzzle/puzzle/student" ]; then
    printf 'Usage: sh install-macos-addon.sh "/path/to/existing assignment folder"\nChoose the outer folder that contains Puzzle/puzzle/student.\n' >&2
    exit 2
fi
destination=$(CDPATH= cd "$1" && pwd)
set -- MACOS_README.md DEBUG_PUZZLE_MACOS.command debug-puzzle-macos.sh \
    Puzzle/macos-common.sh Puzzle/build-macos.sh Puzzle/run-macos.sh \
    Puzzle/debug-macos.sh Puzzle/check-macos.sh \
    Puzzle/build-linux.sh Puzzle/run-linux.sh Puzzle/debug-linux.sh \
    Puzzle/vendor/PROVENANCE.md Puzzle/vendor/raylib/macos/LICENSE \
    Puzzle/vendor/raylib/macos/include/raylib.h Puzzle/vendor/raylib/macos/lib/libraylib.a
for relative in "$@"; do
    if [ ! -f "$relative" ]; then
        printf 'Missing add-on file: %s. Extract the entire add-on first.\n' "$relative" >&2
        exit 1
    fi
done
for relative in "$@"; do
    mkdir -p "$destination/$(dirname "$relative")"
    cp -p "$relative" "$destination/$relative"
done
printf 'Mac support added to:\n%s\nYour four student C files were not replaced.\n\n' "$destination"
printf 'After installing Apple Command Line Tools, open a terminal in that assignment folder and run:\n  sh debug-puzzle-macos.sh\n'
