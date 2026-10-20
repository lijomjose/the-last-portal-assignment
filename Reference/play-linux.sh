#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
if [ ! -f bin/last_portal ]; then
  printf 'Missing game executable. Extract the entire assignment pack first.
' >&2
  exit 1
fi
chmod +x bin/last_portal
exec ./bin/last_portal "$@"
