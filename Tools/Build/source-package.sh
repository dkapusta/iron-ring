#!/usr/bin/env bash
# Packs the source package, Dist/IronRing-<version>-source.zip: the committed tree at
# HEAD (git archive), in one folder. Uncommitted changes are not included.
#
#   Tools/Build/source-package.sh
set -euo pipefail
cd "$(dirname "$0")/../.."
source Tools/Build/common.sh

VERSION=$(release_version "$(source_version)")
NAME="IronRing-$VERSION-source"
if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
    echo "Note: uncommitted changes are not in the package (it is HEAD, $(git rev-parse --short HEAD))." >&2
fi
mkdir -p Dist
rm -f "Dist/$NAME.zip"
git archive --format=zip --prefix="$NAME/" -o "Dist/$NAME.zip" HEAD
echo "== Ready: Dist/$NAME.zip ($(git rev-parse --short HEAD)) =="
