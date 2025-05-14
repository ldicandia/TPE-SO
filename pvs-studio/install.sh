#!/bin/bash
# 2006-2008 (c) Viva64.com Team
# 2008-2020 (c) OOO "Program Verification Systems"
# 2020-2025 (c) PVS-Studio LLC
#

set -Eeuo pipefail

BASEDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)
PREFIX=${1:-}

if [ -z "$PREFIX" ]; then
  case "$OSTYPE" in
    darwin*)  PREFIX="/usr/local" ;;
    *)        PREFIX="/usr" ;;
  esac
fi

function do_install
{
  dst=$1
  for dir in ${@:2}
  do
    cp -av "$dir" "$dst"
  done
}

mkdir -p "$PREFIX"
do_install "$PREFIX" "$BASEDIR/bin" "$BASEDIR/etc" "$BASEDIR/lib"
