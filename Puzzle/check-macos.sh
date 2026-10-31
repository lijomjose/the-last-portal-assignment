#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
. ./macos-common.sh
portal_macos_compiler
mkdir -p build/macos-puzzle
"$portal_cc" -std=c99 -Wall -Wextra -Wpedantic -Werror -DPORTAL_PUZZLE=1 \
    -Iinclude tests/check_repair.c src/repair.c \
    puzzle/student/stack.c puzzle/student/queue.c \
    puzzle/student/attack_list.c puzzle/student/diagnostics.c \
    -o build/macos-puzzle/check_repairs
exec ./build/macos-puzzle/check_repairs "${1:-2}"
