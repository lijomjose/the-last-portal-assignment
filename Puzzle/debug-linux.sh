#!/usr/bin/env sh
# Build only the student's Puzzle edition, with source-level debug information.
set -u
cd "$(dirname "$0")" || exit 1
mode=${1:---run}
case "$mode" in
  --run|--build-only|--gdb|--sanitize) ;;
  *) printf 'Usage: sh debug-linux.sh [--run|--build-only|--gdb|--sanitize]\n' >&2; exit 2 ;;
esac
if [ "$#" -gt 1 ]; then
  printf 'Pass only one debug mode.\n' >&2; exit 2
fi
if [ ! -d puzzle/student ]; then
  printf 'Missing Puzzle source. Extract the whole assignment project first.\n' >&2
  exit 1
fi
compiler=${CC:-cc}
if ! command -v "$compiler" >/dev/null 2>&1; then
  printf 'A C99 compiler is required. Install the lab compiler and try again.\n' >&2
  exit 1
fi
if [ "$mode" = --gdb ] && ! command -v gdb >/dev/null 2>&1; then
  printf 'GDB is not installed. Use the default mode, or install GDB for interactive debugging.\n' >&2
  exit 1
fi
debug_dir=build/puzzle-debug
mkdir -p "$debug_dir" || exit 1
build_log=$debug_dir/build.log
run_log=$debug_dir/run.log
printf 'Building Puzzle with debug symbols and no optimisation...\n'
printf 'Compiler: %s\n' "$compiler" >"$build_log"
"$compiler" --version >>"$build_log" 2>&1
set --
if [ "$mode" = --sanitize ]; then
  set -- -fsanitize=address,undefined -fno-sanitize-recover=all
fi
"$compiler" -std=c99 -Wall -Wextra -Wpedantic -Werror -g3 -O0 \
  -fno-omit-frame-pointer "$@" -DPORTAL_PUZZLE=1 \
  -Iinclude -Ivendor/raylib/linux/include \
  src/main.c src/view.c src/game.c src/repair.c \
  puzzle/student/stack.c puzzle/student/queue.c \
  puzzle/student/attack_list.c puzzle/student/diagnostics.c \
  vendor/raylib/linux/lib/libraylib.a \
  -lm -ldl -lpthread -lX11 -l:libGL.so.1 -lrt \
  -o "$debug_dir/last_portal" >>"$build_log" 2>&1
result=$?
if [ "$result" -ne 0 ]; then
  cat "$build_log"
  printf '\nBuild failed. Fix the first error, save and try again. No game was launched.\nLog: %s\n' "$build_log" >&2
  exit "$result"
fi
printf 'Build succeeded: %s/last_portal\nCompiler log: %s\n' "$debug_dir" "$build_log"
if [ "$mode" = --build-only ]; then exit 0; fi
if [ "$mode" = --gdb ]; then
  printf 'GDB: after a crash, type bt for the call stack; quit to leave.\n'
  exec gdb -q -ex 'set pagination off' \
    -ex 'set logging file build/puzzle-debug/gdb.log' \
    -ex 'set logging overwrite on' -ex 'set logging enabled on' \
    -ex run --args "./$debug_dir/last_portal"
fi
printf 'Opening your Puzzle. Close it to see the runtime log here.\nRuntime log: %s\n' "$run_log"
if [ "$mode" = --sanitize ]; then
  printf 'AddressSanitizer and UndefinedBehaviorSanitizer are enabled for this run.\n'
fi
"./$debug_dir/last_portal" >"$run_log" 2>&1
result=$?
cat "$run_log"
if [ "$result" -ne 0 ]; then
  printf '\nGame exited with status %s. Read %s.\n' "$result" "$run_log" >&2
fi
exit "$result"
