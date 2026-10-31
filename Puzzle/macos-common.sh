#!/usr/bin/env sh
# Shared native macOS build helpers. This file is sourced by the Mac scripts.
portal_macos_compiler()
{
    if [ "$(uname -s)" != Darwin ]; then
        printf 'This launcher is for macOS. On Linux use the Linux launcher.\n' >&2
        return 2
    fi
    if ! command -v xcrun >/dev/null 2>&1 || ! xcrun --find clang >/dev/null 2>&1; then
        printf 'Install Apple Command Line Tools first: xcode-select --install\nFinish that installation, then run this script again.\n' >&2
        return 1
    fi
    portal_cc=${CC:-$(xcrun --find clang)}
    if ! command -v "$portal_cc" >/dev/null 2>&1; then
        printf 'Compiler not found: %s\n' "$portal_cc" >&2
        return 1
    fi
}

portal_macos_dependencies()
{
    portal_macos_compiler || return $?
    portal_macos_bundled=false
    if [ -f vendor/raylib/macos/include/raylib.h ] && [ -f vendor/raylib/macos/lib/libraylib.a ]; then
        if ! xcrun lipo -verify_arch "$(uname -m)" vendor/raylib/macos/lib/libraylib.a; then
            printf 'The bundled raylib library does not support this Mac architecture. Re-extract the complete Mac add-on.\n' >&2
            return 1
        fi
        portal_macos_bundled=true
        return 0
    fi
    # Discover standard native Homebrew installs without executing shellenv.
    if ! command -v brew >/dev/null 2>&1; then
        if [ -x /opt/homebrew/bin/brew ]; then PATH="/opt/homebrew/bin:$PATH"
        elif [ -x /usr/local/bin/brew ]; then PATH="/usr/local/bin:$PATH"
        fi
        export PATH
    fi
    if ! command -v pkg-config >/dev/null 2>&1; then
        printf 'The bundled Mac library is missing. Copy the complete Mac add-on into this assignment folder.\nAlternatively, with Homebrew installed, run: brew install pkgconf raylib\nSee MACOS_README.md in the outer pack folder.\n' >&2
        return 1
    fi
    if command -v brew >/dev/null 2>&1; then
        portal_raylib_prefix=$(brew --prefix raylib 2>/dev/null) || portal_raylib_prefix=
        if [ -n "$portal_raylib_prefix" ]; then
            PKG_CONFIG_PATH="$portal_raylib_prefix/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
            export PKG_CONFIG_PATH
        fi
    fi
    if ! pkg-config --exists raylib; then
        printf 'A native macOS raylib installation is missing. Run:\n  brew install pkgconf raylib\nThe bundled Linux library cannot be used on a Mac.\n' >&2
        return 1
    fi
    portal_raylib_libdir=$(pkg-config --variable=libdir raylib) || return $?
    if [ -z "$portal_raylib_libdir" ]; then
        printf 'raylib pkg-config metadata has no library directory. Reinstall raylib.\n' >&2
        return 1
    fi
}

portal_macos_build()
{
    portal_mode=$1
    portal_macos_dependencies || return $?
    portal_build_dir=build/macos-puzzle
    if [ "$portal_mode" != release ]; then portal_build_dir=build/macos-puzzle-debug; fi
    mkdir -p "$portal_build_dir" || return $?
    portal_build_log=$portal_build_dir/build.log
    portal_binary=$portal_build_dir/last_portal
    printf 'Building native macOS Puzzle (%s / %s)...\n' "$(uname -m)" "$portal_mode"
    {
        printf 'Compiler: %s\nArchitecture: %s\n' "$portal_cc" "$(uname -m)"
        if [ "$portal_macos_bundled" = true ]; then
            printf 'raylib: bundled 6.2 universal macOS static library\n'
        else
            printf 'raylib: %s (pkg-config)\n' "$(pkg-config --modversion raylib)"
        fi
        "$portal_cc" --version
    } >"$portal_build_log" 2>&1
    set -- -O2
    if [ "$portal_mode" != release ]; then set -- -g3 -O0 -fno-omit-frame-pointer; fi
    if [ "$portal_mode" = sanitize ]; then
        set -- "$@" -fsanitize=address,undefined -fno-sanitize-recover=all
    fi
    if [ "$portal_macos_bundled" = true ]; then
        set -- "$@" -Ivendor/raylib/macos/include
    else
        # pkg-config output intentionally expands into arguments for a native
        # Mac installation. Never use the bundled Linux header or library.
        set -- "$@" $(pkg-config --cflags raylib)
    fi
    set -- -std=c99 -Wall -Wextra -Wpedantic -Werror "$@" \
        -DPORTAL_PUZZLE=1 -Iinclude \
        src/main.c src/view.c src/game.c src/repair.c \
        puzzle/student/stack.c puzzle/student/queue.c \
        puzzle/student/attack_list.c puzzle/student/diagnostics.c
    if [ "$portal_macos_bundled" = true ]; then
        set -- "$@" vendor/raylib/macos/lib/libraylib.a
    else
        set -- "$@" $(pkg-config --libs raylib) -Wl,-rpath,"$portal_raylib_libdir"
    fi
    "$portal_cc" "$@" -framework Cocoa -framework IOKit -framework OpenGL \
        -framework CoreVideo -framework CoreAudio -framework AudioToolbox \
        -lm -o "$portal_binary" >>"$portal_build_log" 2>&1
    portal_result=$?
    if [ "$portal_result" -ne 0 ]; then
        cat "$portal_build_log"
        printf '\nBuild failed; no game was launched. Fix the first error and rebuild.\nLog: %s\n' "$portal_build_log" >&2
        return "$portal_result"
    fi
    printf 'Built: %s\nCompiler log: %s\n' "$portal_binary" "$portal_build_log"
}
