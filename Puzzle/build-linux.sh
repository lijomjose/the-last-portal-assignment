#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
if [ "$(uname -s)" = Darwin ]; then exec sh ./build-macos.sh "$@"; fi
default_edition=puzzle
[ -d puzzle/student ] || default_edition=completed
edition=${1:-$default_edition}
case "$edition" in
  puzzle) student_dir=puzzle/student; puzzle=1 ;;
  completed) student_dir=completed; puzzle=0 ;;
  *) printf 'Choose puzzle or completed.\n' >&2; exit 2 ;;
esac
mkdir -p "build/$edition"
${CC:-cc} -std=c99 -Wall -Wextra -Wpedantic -Werror -O2 \
  -DPORTAL_PUZZLE="$puzzle" -Iinclude -Ivendor/raylib/linux/include \
  src/main.c src/view.c src/game.c src/repair.c \
  "$student_dir/stack.c" "$student_dir/queue.c" "$student_dir/attack_list.c" "$student_dir/diagnostics.c" \
  vendor/raylib/linux/lib/libraylib.a \
  -lm -ldl -lpthread -lX11 -l:libGL.so.1 -lrt \
  -o "build/$edition/last_portal"
printf 'Built: build/%s/last_portal\n' "$edition"
