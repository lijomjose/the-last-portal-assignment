#!/bin/sh
cd "$(dirname "$0")" || exit 1
sh ./play-macos.sh
result=$?
printf '\nPress Return to close this window.\n'
read -r response
exit "$result"
