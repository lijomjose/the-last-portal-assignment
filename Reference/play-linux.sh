#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
if [ "$(uname -s)" = Darwin ]; then
  if [ -f bin/last_portal-macos ] && [ -f play-macos.sh ]; then
    exec sh ./play-macos.sh "$@"
  fi
  printf 'This package contains a Linux executable, which cannot run on macOS.\nDownload the completed Mac Reference package from the assignment release page.\n' >&2
  exit 2
fi
if [ ! -f bin/last_portal ]; then
  printf 'Missing game executable. Extract the entire assignment pack first.
' >&2
  exit 1
fi
chmod +x bin/last_portal
exec ./bin/last_portal "$@"
