#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
mkdir -p build/puzzle
${CC:-cc} -std=c99 -Wall -Wextra -Wpedantic -Werror -DPORTAL_PUZZLE=1 \
  -Iinclude tests/check_repair.c src/repair.c \
  puzzle/student/stack.c puzzle/student/queue.c puzzle/student/attack_list.c puzzle/student/diagnostics.c \
  -o build/puzzle/check_repairs
exec ./build/puzzle/check_repairs "${1:-2}"
