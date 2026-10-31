#!/usr/bin/env sh
set -u
cd "$(dirname "$0")" || exit 1
mode=${1:---run}
case "$mode" in
    --run|--build-only|--lldb|--sanitize) ;;
    *) printf 'Usage: sh debug-macos.sh [--run|--build-only|--lldb|--sanitize]\nmacOS uses LLDB rather than the optional Linux GDB mode.\n' >&2; exit 2 ;;
esac
if [ "$#" -gt 1 ]; then printf 'Pass only one debug mode.\n' >&2; exit 2; fi
. ./macos-common.sh
build_mode=debug
if [ "$mode" = --sanitize ]; then build_mode=sanitize; fi
portal_macos_build "$build_mode" || exit $?
if [ "$mode" = --build-only ]; then exit 0; fi
if [ "$mode" = --lldb ]; then
    printf 'LLDB starts the game. After a crash type bt; use quit to leave.\n'
    exec xcrun lldb -o run -- "./$portal_binary"
fi
run_log=$portal_build_dir/run.log
printf 'Opening your Puzzle. Close it to see runtime messages.\nLog: %s\n' "$run_log"
"./$portal_binary" >"$run_log" 2>&1
result=$?
cat "$run_log"
if [ "$result" -ne 0 ]; then printf '\nGame exit status: %s. See %s.\n' "$result" "$run_log" >&2; fi
exit "$result"
